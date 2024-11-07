#pragma once

#include "common/dsp/io/baseband_type.h"
#include "common/dsp/io/file_sink.h"
#include "common/dsp/path/splitter.h"
#include "common/dsp/resamp/rational_resampler.h"
#include "modules/demod/module_demod_base.h"
#include <cstdint>
#include <memory>

namespace iq_re
{
    class IQREModule : public demod::BaseDemodModule
    {
    protected:
        std::shared_ptr<dsp::RationalResamplerBlock<complex_t>> res;
        std::shared_ptr<dsp::FileSinkBlock> file_sink;
        std::shared_ptr<dsp::SplitterBlock> splitter;
        std::shared_ptr<dsp::stream<complex_t>> work_out;

        bool multiplyConjugate = true;
        bool exponentiate = false;

        int exponent = 2;

        uint64_t base_samplerate = 0;
        uint64_t frequency_hz = 0;

        //dsp::BasebandType baseband_format;

    public:
        IQREModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
        ~IQREModule();
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
