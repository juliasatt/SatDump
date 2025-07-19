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
        delete[] wip_texture_buffer;
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

    void BBContainer::doDrawPlotTextures(ImPlotRect c)
    {
        //

        // ImPlot::PlotImage("Spectrogram", (void *)(intptr_t)textureID, ImVec2(0, 0), ImVec2(6e6, numOfLines));
    }

} // namespace satdump
