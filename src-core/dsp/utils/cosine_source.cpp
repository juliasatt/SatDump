#include "cosine_source.h"

namespace satdump
{
    namespace ndsp
    {
        CosBlock::CosBlock() : Block("cosine_source_f", {}, {{"out", DSP_SAMPLE_TYPE_F32}}) {}

        CosBlock::~CosBlock() {}

        bool CosBlock::work()
        {
            if (!work_should_exit)
            {
                DSPBuffer oblk = outputs[0].fifo->newBufferSamples(d_buffer_size, sizeof(float));
                float *obuf = oblk.getSamples<float>();

                oblk.size = d_buffer_size;

                for (int i = 0; i < oblk.size; i++)
                {
                    float t = i / (double)d_samprate;
                    float amp = cosf(t * d_freq *  M_PI);
                    obuf[i] = amp;
                }

                // float fracFreq = d_freq / d_samprate;
                // d_phase += 2.0 * M_PI * fracFreq;
                //
                // d_phase += ((d_phase >= 2.0 * M_PI) * -2.0 * M_PI) + ((d_phase < 0.0) * 2.0 * M_PI);
                //
                // for (int i = 0; i < oblk.size; i++)
                // {
                //     obuf[i] = cos(d_phase) * d_amp;
                // }

                outputs[0].fifo->wait_enqueue(oblk);
            }
            else
            {
                outputs[0].fifo->wait_enqueue(outputs[0].fifo->newBufferTerminator());
                return true;
            }

            return false;
        }
    } // namespace ndsp
} // namespace satdump
