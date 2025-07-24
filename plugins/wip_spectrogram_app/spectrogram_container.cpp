#include "spectrogram_container.h"
#include "common/dsp/buffer.h"
#include "common/dsp/io/baseband_interface.h"
#include "common/dsp/io/baseband_type.h"
#include "common/utils.h"
#include "core/exception.h"
#include "dsp/device/options_displayer_warper.h"
#include "dsp/io/iq_types.h"
#include "imgui/imgui_image.h"
#include "imgui/implot/implot.h"
#include "logger.h"
#include <algorithm>
#include <cstdint>
#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <random>
#include <sys/mman.h>
#include <unistd.h>

namespace satdump
{
    SpectrogramContainer::SpectrogramContainer(std::string name, std::string file_path) : d_name(name), d_filePath(file_path)
    {
        // Generate unique ID
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> check(65, 90);
        for (size_t i = 0; i < 15; i++)
            unique_id[i] = check(rng);

        d_fileMemSize = getFilesize(file_path);
        if (d_fileMemSize == 0)
            throw satdump_exception("Empty File!");
        fd = open(file_path.c_str(), O_RDONLY);
        d_fileMemPtr = (uint8_t *)mmap(0, d_fileMemSize, PROT_READ, MAP_SHARED, fd, 0);
        if (d_fileMemPtr == MAP_FAILED)
        {
            close(fd);
            throw satdump_exception("mmap failed!");
        }
    }

    SpectrogramContainer::~SpectrogramContainer()
    {
        delete[] textureBuffer;
        munmap(d_fileMemPtr, d_fileMemSize);
        close(fd);

        if (d_isTmp && std::filesystem::exists(d_filePath))
        {
            try
            {
                std::filesystem::remove(d_filePath);
            }
            catch (std::exception &e)
            {
                logger->warn("Failed to delete temporary file: %s", e.what());
            }
        }
    }

    // void SpectrogramContainer::doUpdateTextures()
    // {
    //     textureBuffer = new uint32_t[d_fileMemSize];
    //
    // }

    // void SpectrogramContainer::initFFT()
    // {
    //     // file_source->start();
    //     // fft_gen->start();
    //     //
    //     // while (!file_source->d_eof)
    //     //     stopDSP();
    //
    //     //
    // }

    // void SpectrogramContainer::applyParams(uint64_t number_of_lines, int fft_size, int fft_overlap, int seconds, double file_timestamp)
    // {
    //     timeStamp = file_timestamp;
    //     nOfLines = number_of_lines;
    //     fftSize = fft_size;
    //     fftOverlap = fft_overlap;
    //     lengthInSecs = seconds;
    //
    //     sampleRate = samprate.get();
    //
    //     fft_gen->set_fft_gen_settings(fftSize, fftOverlap, 131072); // FIX: give the user an option to change the d_blockSize
    //
    //     file_source->set_cfg("file", d_filePath);
    //     file_source->set_cfg("type", baseBandType);
    //
    //     file_opts->update();
    //
    //     // iqType = std::filesystem::path(d_filePath.getPath()).extension().string().erase(0, 1);
    //
    //     //
    // }

    // void SpectrogramContainer::stopDSP()
    // {
    //     file_source->stop();
    //     fft_gen->stop();
    // }

    // void SpectrogramContainer::initSpectrogram()
    // {
    //
    //     //
    // }

    // void SpectrogramContainer::pushFloats(float *floats)
    // {
    //     floatsInBuffer = floats;
    //     //
    // }
    // void SpectrogramContainer::doDrawPlotTextures(ImPlotRect c)
    // {
    //     for (auto &part : image_parts)
    //     {
    //         // if (c.Min().x > part.pos1_x || c.Max().y < part.pos1_y)
    //         //     continue;
    //         // if (c.Max().x < part.pos2_x || c.Min().y > part.pos2_y)
    //         //     continue;
    //
    //         if (part.i == -1)
    //             continue;
    //
    //         bool status_before = part.visible;
    //         part.visible = false;
    //         if (c.Min().x > part.pos2_x || c.Max().y < part.pos2_y) {}
    //         else if (c.Max().x < part.pos1_x || c.Min().y > part.pos1_y) {}
    //         else
    //         {
    //             // printf("%f %f - %f %f  ----  %f %f - %f %f ---- %d\n",
    //             //        c.Min().x, c.Min().y, c.Max().x, c.Max().y,
    //             //        part.pos1_x, part.pos1_y, part.pos2_x, part.pos2_y,
    //             //        part.i);
    //             ImPlot::PlotImage("Test", (void *)(intptr_t)part.image_id, {part.pos1_x, part.pos1_y}, {part.pos2_x, part.pos2_y});
    //
    //             part.visible = true;
    //         }
    //
    //         if (part.visible != status_before)
    //             update = true;
    //     }
    // }

    // void SpectrogramContainer::doDrawPlotTextures(ImPlotRect c)
    // {
    //     file_opts->draw();
    //     //
    // }

} // namespace satdump
