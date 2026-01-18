#pragma once

#include "dsp/block.h"
#include "dsp/block_simple.h"
#include <cstdint>

namespace satdump
{
    namespace ndsp
    {
        class CosBlock : public BlockSimple<float, float>
        {
        private:
            float d_samprate;
            float d_frequency;
            float d_amp;
            float d_phase;

        public:
            CosBlock();
            ~CosBlock();

            uint32_t process(float *input, uint32_t);

        };
    } // namespace ndsp
} // namespace satdump
