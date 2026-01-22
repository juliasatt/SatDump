#pragma once

#include "dsp/block.h"

namespace satdump
{
    namespace ndsp
    {
        // template <typename T>
        class CosBlock : public Block
        {
        private:
            int d_buffer_size = 8192;

            float d_samprate = 48000;
            float d_freq = 10000;
            float d_amp = 0.5;
            float d_phase = 0.0;

            // void *buff_source;

            bool work();

        public:
            CosBlock();
            ~CosBlock();

            void init() {}

            nlohmann::ordered_json get_cfg_list()
            {
                nlohmann::ordered_json p;
                add_param_simple(p, "samprate", "float", "Samplerate");
                add_param_simple(p, "freq", "float", "Frequency");
                add_param_simple(p, "amp", "float", "Amplitude");
                add_param_simple(p, "phase", "float", "Phase");
                add_param_simple(p, "bufs", "int", "Buffer Size");
                return p;
            }

            nlohmann::json get_cfg(std::string key)
            {
                if (key == "samprate")
                    return d_samprate;
                else if (key == "freq")
                    return d_freq;
                else if (key == "amp")
                    return d_amp;
                else if (key == "phase")
                    return d_phase;
                else if (key == "bufs")
                    return d_buffer_size;
                else
                    throw satdump_exception(key);
            }

            cfg_res_t set_cfg(std::string key, nlohmann::json v)
            {
                if (key == "samprate")
                    d_samprate = v;
                else if (key == "freq")
                    d_freq = v;
                else if (key == "amp")
                    d_amp = v;
                else if (key == "phase")
                    d_phase = v;
                else if (key == "bufs")
                    d_buffer_size = v;
                else
                    throw satdump_exception(key);
                return RES_OK;
            }
        };
    } // namespace ndsp
} // namespace satdump
