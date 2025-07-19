#include "bb_container.h"

#include "common/utils.h"
#include "core/resources.h"
#include "imgui/imgui.h"
#include "imgui/imgui_image.h"
#include "imgui/implot/implot.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <random>
#include <sys/mman.h>
#include <unistd.h>

namespace satdump
{
    BBContainer::BBContainer(std::string name, std::string file_path) : d_name(name), d_filepath(file_path)
    {
        // Generate unique ID
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> check(65, 90);
        for (size_t i = 0; i < 15; i++)
            unique_id[i] = check(rng);

        // Buffer for creating textures
        // wip_texture_buffer = new uint32_t[d_chunk_size * d_chunk_size];

        // Init mmap pointers
        d_file_memory_size = getFilesize(file_path);
        if (d_file_memory_size == 0)
            throw satdump_exception("Empty File!");
        fd = open(file_path.c_str(), O_RDONLY);
        d_file_memory_ptr = (uint8_t *)mmap(0, d_file_memory_size, PROT_READ, MAP_SHARED, fd, 0);
        if (d_file_memory_ptr == MAP_FAILED)
        {
            close(fd);
            throw satdump_exception("mmap failed!");
        }
    }

    BBContainer::~BBContainer()
    {
        delete[] textureBuffer;
        munmap(d_file_memory_ptr, d_file_memory_size);
        close(fd);

        if (d_is_temp && std::filesystem::exists(d_filepath))
        {
            try
            {
                std::filesystem::remove(d_filepath);
            }
            catch (std::exception &e)
            {
                logger->warn("Failed to delete temporary file: %s", e.what());
            }
        }
    }

    bool BBContainer::bufferAlloc(size_t size)
    {
        uint32_t *newTextureBuffer = (uint32_t *)realloc(textureBuffer, size);
        if (newTextureBuffer == nullptr)
        {
            logger->error("Cannot allocate memory for the Spectrogram");
            if (textureBuffer != nullptr)
            {
                free(textureBuffer);
                textureBuffer = nullptr;
            }
            last_x_pos = 0;
            last_y_pos = 0;
            return false;
        }

        textureBuffer = newTextureBuffer;
        uint64_t old_size = last_x_pos * last_y_pos;
        if (size > old_size * sizeof(uint32_t))
            memset(&textureBuffer[old_size], 0, size - old_size * sizeof(uint32_t));
        last_x_pos = x_pos;
        last_y_pos = y_pos;
        return true;
    }

    void BBContainer::setNumOfLines(int number_of_lines)
    {
        //
        numOfLines = number_of_lines;
    }

    void BBContainer::setFFTSize(int fft_size)
    {
        //
        bbFFTSize = fft_size;
    }

    void BBContainer::setScaleLimits(float scale_min, float scale_max)
    {
        scaleMin = scale_min;
        scaleMax = scale_max;
    }

    void BBContainer::pushFloatsIn(float *floats)
    {
        if (textureID == 0 || textureBuffer == nullptr)
            return;

        work_mtx.lock();
        if ((fft_img_i++ % fft_img_i_mod) == 0)
        {
            if (fft_img_i * 5e6 == fft_img_i_mod)
                fft_img_i = 0;

            memmove(&textureBuffer[x_pos * 1], &textureBuffer[x_pos * 0], x_pos * (y_pos - 1) * sizeof(uint32_t));

            double fz = (double)bbFFTSize / (double)x_pos;
            for (int i = 0; i < x_pos; i++)
            {
                float fft_pos = i * fz;

                if (fft_pos >= bbFFTSize)
                    fft_pos = bbFFTSize - 1;

                float final = -INFINITY;
                for (float v = fft_pos; v < fft_pos + 1; v += 1)
                    if (final < floats[(int)floor(v)])
                        final = floats[(int)floor(v)];

                int v = ((final - scaleMin) / fabs(scaleMax - scaleMin)) * resolution;

                if (v < 0)
                    v = 0;
                if (v >= resolution)
                    v = resolution - 1;

                textureBuffer[i] = palette[v];
            }

            has_to_update = true;
        }
        work_mtx.unlock();
    }

    void BBContainer::setRate(int input_rate, int output_rate)
    {
        work_mtx.lock();
        if (output_rate <= 0)
            output_rate = 1;
        fft_img_i_mod = input_rate / output_rate;
        if (fft_img_i_mod <= 0)
            fft_img_i_mod = 1;
        fft_img_i = 0;
        work_mtx.unlock();
    }

    void BBContainer::setPallete(colormaps::Map selected_palette, bool mutex)
    {
        if (mutex)
            work_mtx.lock();
        palette = colormaps::generatePalette(selected_palette, resolution);
        if (mutex)
            work_mtx.unlock();
    }

    void BBContainer::doDrawPlotTextures(ImPlotRect c)
    {

        work_mtx.lock();
        if (textureID == 0 || is_processing)
        {
            x_pos = c.Max().x > bbFFTSize ? bbFFTSize : c.Max().x;
            y_pos = c.Max().y > numOfLines ? numOfLines : c.Max().y;
        }

        if (textureID == 0)
        {
            textureID = makeImageTexture();
            has_to_update = bufferAlloc(x_pos * y_pos * sizeof(uint32_t));
            if ((int)palette.size() != resolution)
                setPallete(colormaps::loadMap(resources::getResourcePath("waterfall/classic.json")), false);
        }
        if (is_processing && (last_x_pos != x_pos || last_y_pos != y_pos))
        {
            if (textureBuffer != nullptr && last_x_pos != x_pos)
            {
                free(textureBuffer);
                textureBuffer = nullptr;
                last_x_pos = 0;
                last_y_pos = 0;
            }
            has_to_update = bufferAlloc(x_pos * y_pos * sizeof(uint32_t));
        }
        if (has_to_update)
        {
            updateImageTexture(textureID, textureBuffer, x_pos, y_pos);
            has_to_update = false;
        }
        work_mtx.unlock();

        ImPlot::PlotImage("Spectrogram", (void *)(intptr_t)textureID, ImVec2(0, 0), ImVec2(6e6, numOfLines));
    }

} // namespace satdump
