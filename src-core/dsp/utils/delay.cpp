#include "delay.h"
#include <cstring>

namespace satdump
{
    namespace ndsp
    {
        template <typename T>
        DelayBlock<T>::DelayBlock()
            : BlockSimple<T, T>(                                            //
                  "delay_" + getShortTypeName<T>() + getShortTypeName<T>(), //
                  {{"in", getTypeSampleType<T>()}},                         //
                  {{"out", getTypeSampleType<T>()}})
        {
            in_buf_size = dsp::STREAM_BUFFER_SIZE; // TODOREWORK
            in_buf = 0;
        }

        template <typename T>
        DelayBlock<T>::~DelayBlock()
        {
        }

        template <typename T>
        uint32_t DelayBlock<T>::process(T *input, uint32_t nsamples, T *output)
        {
            if (needs_reinit)
            {
                needs_reinit = false;
                init();
            }

            if (in_buf + nsamples < in_buf_size)
            {
                memset(output, 0, nsamples * sizeof(T));
                in_buf += nsamples;
            }

            if (in_buf > d_delayN)
            {
                int count = 0;
                while (in_buf - count > d_delayN)
                {
                    memcpy(output, &input[count], d_delayN * sizeof(T));
                    count += d_delayN;
                }
                if (count < in_buf)
                {
                    memmove(output, &output[count], (in_buf - count) * sizeof(T));
                    in_buf -= count;
                }
            }
            //
            // for (int i = 0; nsamples < i; i++)
            // {
            //     if (nsamples != d_delayN)
            //         memset(output, 0, nsamples * sizeof(T));
            //     else
            //         memcpy(output, input, nsamples * sizeof(T));
            // }
            return nsamples;
        }
        template class DelayBlock<float>;
        template class DelayBlock<complex_t>;
    } // namespace ndsp
} // namespace satdump
