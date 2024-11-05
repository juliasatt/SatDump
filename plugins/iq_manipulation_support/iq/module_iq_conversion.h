#pragma once

#include "common/dsp/resamp/rational_resampler.h"
#include "modules/demod/module_demod_base.h"

namespace iq_conversion
{
    class IQConversionModule : public demod::BaseDemodModule
    {
    protected:
        std::shared_ptr<dsp::RationalResamplerBlock<complex_t>> res;

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
