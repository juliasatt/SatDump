#pragma once

#include "common/colormaps.h"
#include "core/exception.h"
#include "imgui/implot/implot.h"
#include "logger.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace satdump
{
    class BBContainer
    {
    private:
        const std::string d_name;
        const std::string d_filepath;
        size_t d_file_memory_size;
        uint8_t *d_file_memory_ptr;
        int fd;
        char unique_id[16] = {0};

        // private:
        //     struct PartImage
        //     {
        //         unsigned int image_id = 0;
        //         double pos1_x = 0;
        //         double pos1_y = 0;
        //         double pos2_x = 0;
        //         double pos2_y = 0;
        //         int i = -1;
        //     };
        //
        //     size_t img_parts_x = 0;
        //     size_t img_parts_y = 0;
        //     std::vector<PartImage> image_parts;

    private:
        uint32_t *wip_texture_buffer = nullptr;

        bool force_update_all = false;
        bool update = false;

    private:
        bool is_processing = false;

    private:
        float *floatsInBuf;

    public:
        uint8_t *get_ptr() { return d_file_memory_ptr; }
        size_t get_ptr_size() { return d_file_memory_size; }

        bool d_is_temp = false;

    public:
        BBContainer(std::string name, std::string file);
        ~BBContainer();

        std::string getName() { return d_name; }
        std::string getID() { return std::string(unique_id); }

        void doDrawPlotTextures(ImPlotRect c);

    public:
        void *bb_toolbox = nullptr;
    };
} // namespace satdump
