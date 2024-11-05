#pragma once

#include "modules/demod/module_demod_base.h"
#include <cstdint>
#include <memory>

namespace iq_conversion
{
    class IQConversionModule : public demod::BaseDemodModule
    {
    protected:
        //std::shared_ptr<dsp::SmartResamplerBlock<complex_t>> smart_res;
        std::shared_ptr<dsp::RationalResamplerBlock<complex_t>> res;

        //int decimation = 1;
        //int interpolation = 1;

        uint64_t base_samplerate = 0;

    public:
        IQConversionModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
        ~IQConversionModule();
        void init();
        void stop();
        void process();
        void drawUI(bool window);

    public:
        static std::string getID();
        virtual std::string getIDM() { return getID(); };
        static std::vector<std::string> getParameters();
        static std::shared_ptr<ProcessingModule> getInstance(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
    };
}
