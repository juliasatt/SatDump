#pragma once

/**
 * @file delay.h
 */

#include "common/dsp/buffer.h"
#include "common/dsp/complex.h"
#include "dsp/block.h"
#include "dsp/block_helpers.h"
#include "dsp/block_simple.h"
#include <cstring>

namespace satdump
{
    namespace ndsp
    {
        template <typename T>
        class DelayBlock : public BlockSimple<T, T>
        {
        public:
            int in_buf = 0;
            int in_buf_size = 0;
            int p_delayN = 0;

            bool needs_reinit = false;

        private:
            int d_delayN;

        public:
            DelayBlock();
            ~DelayBlock();

            uint32_t process(T *input, uint32_t nsamples, T *output);

            void init() { d_delayN = p_delayN; }

            nlohmann::ordered_json get_cfg_list()
            {
                nlohmann::ordered_json p;
                add_param_simple(p, "delay", "int", "Delay N Samples");
                // add_param_simple(p, "bufs", "int", "Buffer Size");
                return p;
            }

            nlohmann::json get_cfg(std::string key)
            {
                if (key == "delay")
                    return p_delayN;
                // else if (key == "bufs")
                //     return p_buffer_size;
                else
                    throw satdump_exception(key);
            }

            Block::cfg_res_t set_cfg(std::string key, nlohmann::json v)
            {
                if (key == "delay")
                {
                    p_delayN = v;
                    needs_reinit = true;
                }
                // else if (key == "bufs")
                // {
                //     p_buffer_size = v;
                // }
                else
                    throw satdump_exception(key);
                return Block::RES_OK;
            }
        };
    } // namespace ndsp
} // namespace satdump
