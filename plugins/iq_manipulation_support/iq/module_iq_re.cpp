#include "module_iq_re.h"
#include "common/dsp/buffer.h"
#include "common/dsp/io/file_sink.h"
#include "common/dsp/io/wav_writer.h"
#include "common/dsp/path/splitter.h"
#include "common/dsp/resamp/rational_resampler.h"
#include "common/utils.h"
#include "common/ziq.h"
#include "common/ziq2.h"
#include "core/module.h"
#include <cstdint>
#include <memory>
#include <string>
#include <volk/volk.h>
#include <volk/volk_complex.h>


namespace iq_re
{
    IQREModule::IQREModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters) : BaseDemodModule(input_file, output_file_hint, parameters)
    {
        res_samplerate = d_samplerate * 2;
    }


    void IQREModule::init()
    {
        BaseDemodModule::initb();

        // Resampler
        res = std::make_shared<dsp::RationalResamplerBlock<complex_t>>(agc->output_stream, 2, 1);
    }


    IQREModule::~IQREModule()
    {
    }

    void IQREModule::process()
    {
        if (input_data_type == DATA_FILE)
            filesize = file_source->getFilesize();
        else
            filesize = 0;

        //if (output_data_type == DATA_FILE)
        //{
        //    data_out = std::ofstream(d_output_file_hint + ".wav", std::ios::binary);
        //    d_output_files.push_back(d_output_file_hint + ".wav");
        //}

        std::string output_format = "cf32";
        if (d_parameters["output_format"].get<std::string>() == "cf32")
            output_format = "cf32";
        if (d_parameters["output_format"].get<std::string>() == "cs16")
            output_format = "cs16";
        if (d_parameters["output_format"].get<std::string>() == "cs8")
            output_format = "cs8";
        if (d_parameters["output_format"].get<std::string>() == "cu8")
            output_format = "cu8";
        if (d_parameters["output_format"].get<std::string>() == "wav")
            output_format = "wav";
        //if (d_parameters["output_format"].get<std::string>() == "ziq")
        //    output_format = "ziq";
        //if (d_parameters["output_format"].get<std::string>() == "ziq2")
        //    output_format = "ziq2";

        if (output_data_type == DATA_FILE)
        {
            data_out = std::ofstream(d_output_file_hint + "." + output_format, std::ios::binary);
            d_output_files.push_back(d_output_file_hint + "." + output_format);
        }

        if (output_format == "wav")
        {
            dsp::WavWriter wave_writer(data_out);
            wave_writer.write_header(res_samplerate, 2);
        }

//#ifdef BUILD_ZIQ
//        if (output_format == "ziq")
//        {
//            ziqcfg.is_compressed = true;
//            ziqcfg.bits_per_sample = output_format.ziq_depth;
//            ziqcfg.samplerate = res_samplerate;
//            ziqcfg.annotation = "";
//
//            ziqWriter = std::make_shared<ziq::ziq_writer>(ziqcfg, data_out);
//        }
//#endif
//#ifdef BUILD_ZIQ2
//        if (output_format == "ziq2")
//        {
//            int sz = ziq2::ziq2_write_file_hdr((uint8_t *)buffer_s8, samplerate);
//            data_out.write((char *)buffer_s8, sz);
//
//            if (mag_buffer == nullptr)
//                mag_buffer == dsp::create_volk_buffer<float>(d_buffer_size);
//        }
//#endif

        logger->info("Using input baseband " + d_input_file);
        logger->info("Converting to " + d_output_file_hint + output_format);
        logger->info("Buffer size : " + std::to_string(d_buffer_size));

        time_t lastTime = 0;

        uint64_t final_data_size = 0;
        uint64_t final_data_size_raw = 0;
        complex_t last_samp;

        // Start
        BaseDemodModule::start();
        res->start();

        complex_t *work_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);
        complex_t *base_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);
        complex_t *delayed_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);
        complex_t *non_delayed_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);

        float *work_buffer_float = dsp::create_volk_buffer<float>(d_buffer_size);

        int8_t * buffer_s8 = dsp::create_volk_buffer<int8_t>(d_buffer_size * 2);
        int16_t * buffer_s16 = dsp::create_volk_buffer<int16_t>(d_buffer_size * 2);

        int dat_size = 0;
        while (demod_should_run())
        {
            dat_size = res->output_stream->read();

            if (dat_size <= 0)
            {
                res->output_stream->flush();
                continue;
            }

            out_mtx.lock();

            if (multiplyConjugate)
            {
                logger->info("Multiply conjugate selected!");

                for (int i = 0; i < dat_size; i++)
                {
                    delayed_buffer_complex[i] = i == 0 ? last_samp : res->output_stream->readBuf[i - 1];
                    non_delayed_buffer_complex[i] = res->output_stream->readBuf[i];
                }

                last_samp = res->output_stream->readBuf[dat_size - 1];

                volk_32fc_x2_multiply_conjugate_32fc((lv_32fc_t *)work_buffer_complex, (lv_32fc_t *)non_delayed_buffer_complex, (lv_32fc_t *)delayed_buffer_complex, dat_size);
            }

            if (exponentiate)
            {
                logger->info("Exponentiate selected!");

                volk_32fc_x2_multiply_32fc((lv_32fc_t *)work_buffer_complex, (lv_32fc_t *)res->output_stream->readBuf, (lv_32fc_t *)res->output_stream->readBuf, dat_size);
                for (int i = 2; i < exponent; i++)
                {
                    volk_32fc_x2_multiply_32fc((lv_32fc_t *)work_buffer_complex, (lv_32fc_t *)work_buffer_complex, (lv_32fc_t *)res->output_stream->readBuf, dat_size);
                }

            }

            if (output_data_type == DATA_FILE)
            {
                if (output_format == "cf32")
                {
                    data_out.write((char *)work_buffer_complex, dat_size * sizeof(complex_t));
                    final_data_size += dat_size * sizeof(complex_t);
                }
                else if (output_format == "cs16" || output_format == "wav")
                {
                    volk_32f_s32f_convert_16i(buffer_s16, (float *)work_buffer_float, 65535, dat_size * 2);
                    data_out.write((char *)buffer_s16, dat_size * sizeof(int16_t) * 2);
                    final_data_size += dat_size * sizeof(int16_t) * 2;
                }
                else if (output_format == "cs8")
                {
                    volk_32f_s32f_convert_8i(buffer_s8, (float *)work_buffer_float, 127, dat_size * 2);
                    data_out.write((char *)buffer_s8, dat_size * sizeof(int8_t) * 2);
                    final_data_size += dat_size * sizeof(int8_t) * 2;
                }
//#ifdef BUILD_ZIQ
//                else if (output_format == "ziq")
//                {
//                    final_data_size += ziqWriter->write(work_buffer_complex, dat_size);
//                    final_data_size_raw += (ziqcfg.bits_per_sample / 4) * dat_size;
//                }
//#endif
//#ifdef BUILD_ZIQ2
//                else if (output_format == "ziq2")
//                {
//                    int sz = ziq2::ziq2_write_iq_pkt((uint8_t *)buffer_s8, work_buffer_complex, mag_buffer, dat_size, output_format.ziq_depth);
//                    data_out.write((char *)buffer_s8, sz);
//                    final_data_size += sz;
//                }
//#endif
            }

            //if (output_data_type == DATA_FILE)
            //{
            //    data_out.write((char *)work_buffer_complex, dat_size * sizeof(complex_t));
            //    final_data_size += dat_size * sizeof(complex_t);
            //}

            out_mtx.unlock();

            res->output_stream->flush();

            if (input_data_type == DATA_FILE)
                progress = file_source->getPosition();

            if (time(NULL) % 10 == 0 && lastTime != time(NULL))
            {
                lastTime = time(NULL);
                logger->info("Progress " + std::to_string(round(((double)progress / (double)filesize) * 1000.0) / 10.0) + "%%");
            }
        }
    }

    void IQREModule::stop()
    {
        // Stop
        BaseDemodModule::stop();
        res->stop();
        res->output_stream->stopReader();
    }

    std::string IQREModule::getID()
    {
        return "iq_re";
    }


    std::vector<std::string> IQREModule::getParameters()
    {
        std::vector<std::string> params;
        return params;
    }

    std::shared_ptr<ProcessingModule> IQREModule::getInstance(std::string input_file, std::string output_file_hint, nlohmann::json parameters)
    {
        return std::make_shared<IQREModule>(input_file, output_file_hint, parameters);
    }
}
