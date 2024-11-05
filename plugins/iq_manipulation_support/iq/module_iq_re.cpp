#include "module_iq_re.h"
#include "common/dsp/resamp/rational_resampler.h"
#include <memory>
#include <string>


namespace iq_re
{
    IQREModule::IQREModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters) : BaseDemodModule(input_file, output_file_hint, parameters)
    {
    }


    void IQREModule::init()
    {
        // Resampler
        //res = std::make_shared<dsp::RationalResamplerBlock<complex_t>>(agc->output_stream, samplerate, final_samplerate);
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

        std::string output_format = "";
        if (d_parameters["output_format"].get<std::string>() == "cf32")
            output_format = "FC32";

        logger->info("Using input baseband " + d_input_file);
        logger->info("Converting to " + d_output_file_hint + output_format);
        logger->info("Buffer size : " + std::to_string(d_buffer_size));

        time_t lastTime = 0;

        // Start
        BaseDemodModule::start();
        //res->start();


        int dat_size = 0;
        while (demod_should_run())
        {
            dat_size = input_stream->read();

            if (dat_size <= 0)
            {
                input_stream->flush();
                continue;
            }

            input_stream->flush();

            if (input_data_type == DATA_FILE)
                progress = file_source->getPosition();

            if (time(NULL) % 10 == 0 && lastTime != time(NULL))
            {
                lastTime = time(NULL);
                logger->info("Progress " + std::to_string(round(((double)progress / (double)filesize) * 1000.0) / 10.0) + "%%");
            }
        }





    }












}
