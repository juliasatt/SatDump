#include "module_iq_re.h"
#include "common/dsp/buffer.h"
#include "common/dsp/io/file_sink.h"
#include "common/dsp/path/splitter.h"
#include "common/dsp/resamp/rational_resampler.h"
#include "common/utils.h"
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

        base_samplerate = d_samplerate * 2;
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

        //if (output_data_type == DATA_FILE)
        //{
        //    data_out = std::ofstream(d_output_file_hint + "." + output_format, std::ios::binary);
        //    d_output_files.push_back(d_output_file_hint + "." + output_format);
        //}

        if (output_data_type == DATA_FILE)
        {
            file_sink->start();
            file_sink->set_output_sample_type(output_format);
            std::string int_file = d_output_file_hint + "_" + std::to_string((uint64_t)base_samplerate);
        }


        logger->info("Using input baseband " + d_input_file);
        logger->info("Converting to " + d_output_file_hint + output_format);
        logger->info("Buffer size : " + std::to_string(d_buffer_size));

        time_t lastTime = 0;

        complex_t last_samp;

        // Start
        BaseDemodModule::start();
        res->start();
        file_sink->start();
        splitter->start();

        complex_t *work_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);
        complex_t *base_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);
        complex_t *delayed_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);
        complex_t *non_delayed_buffer_complex = dsp::create_volk_buffer<complex_t>(d_buffer_size);



        int dat_size = 0;
        while (demod_should_run())
        {
            dat_size = res->output_stream->read();

            if (dat_size <= 0)
            {
                res->output_stream->flush();
                continue;
            }

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

            //work_out->writeBuf = work_buffer_complex;
            //splitter = std::make_shared<dsp::SplitterBlock>(work_out->readBuf);

            std::shared_ptr<dsp::FileSinkBlock> file_sink = std::make_shared<dsp::FileSinkBlock>(work_buffer_complex);

            if (output_data_type == DATA_FILE)
            {
                splitter->add_output("output_baseband");
                splitter->set_enabled("output_baseband", true);
                file_sink = std::make_shared<dsp::FileSinkBlock>(splitter->get_output("output_baseband"));
            }


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
        file_sink->stop();
        splitter->stop();
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
