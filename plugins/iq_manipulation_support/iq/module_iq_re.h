#pragma once

#include "common/dsp/resamp/rational_resampler.h"
#include "common/ziq.h"
#include "modules/demod/module_demod_base.h"
#include <cstdint>
#include <memory>

namespace iq_re
{
    class IQREModule : public demod::BaseDemodModule
    {
    protected:
        std::shared_ptr<dsp::RationalResamplerBlock<complex_t>> res;

        bool multiplyConjugate = true;
        bool exponentiate = false;

        int exponent = 2;

        uint64_t res_samplerate = 0;
        uint64_t frequency_hz = 0;

        std::mutex out_mtx;

#ifdef BUILD_ZIQ
        ziq::ziq_cfg ziqcfg;
        std::shared_ptr<ziq::ziq_writer> ziqWriter;
#endif

        float *mag_buffer = nullptr;

        std::unique_ptr<dsp::WavWriter> wav_writer;

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
