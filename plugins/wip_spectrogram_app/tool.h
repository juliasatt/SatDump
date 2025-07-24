#pragma once

#include "libs/ctpl/ctpl_stl.h"
#include "spectrogram.h"

namespace satdump
{
    class SpectrogramTool
    {
    public:
        virtual std::string getName() = 0;
        virtual void renderMenu(std::shared_ptr<SpectrogramContainer> &container, bool is_busy) = 0;
        virtual bool needToProcess() = 0;
        virtual void setProcessed() = 0;
        virtual void process(std::shared_ptr<SpectrogramContainer> &container, float &process_progress) = 0;
    };
} // namespace satdump
