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
        bool has_to_update = false;
        unsigned int textureID = 0;
        uint32_t *textureBuffer = nullptr;
        const int resolution = 65536;

        std::vector<uint32_t> palette;

        bool force_update_all = false;
        bool update = false;

    private:
        std::mutex work_mtx;
        bool is_processing = false;
        bool bufferAlloc(size_t size);

    private:
        float *floatsInBuf;

        int fft_img_i_mod = 0;
        int fft_img_i = 0;
        int x_pos = 0;
        int y_pos = 0;
        int last_x_pos = 0;
        int last_y_pos = 0;

    public:
        double secs;
        uint64_t samplerate;
        int numOfLines = 512;
        int bbFFTSize = 8192;
        // int selectedFFTSize = 8;
        // std::vector<int> fftSizesLut = {2097152, 1048576, 524288, 262144, 131072, 65536, 32768, 16384, 8192, 4096, 2048, 1024};
        float scaleMin = -160;
        float scaleMax = 160;

        std::vector<colormaps::Map> palettes;
        std::string palettesString;
        int selected_palette = 0;

    public:
        uint8_t *get_ptr() { return d_file_memory_ptr; }
        size_t get_ptr_size() { return d_file_memory_size; }

        bool d_is_temp = false;

    public:
        BBContainer(std::string name, std::string file);
        ~BBContainer();

        std::string getName() { return d_name; }
        std::string getID() { return std::string(unique_id); }

        void setNumOfLines(int number_of_lines);
        void setFFTSize(int fft_size);
        void setScaleLimits(float scale_min, float scale_max);
        void setRate(int input_rate, int output_rate);
        void setPallete(colormaps::Map selected_pallete, bool mutex = true);

        void pushFloatsIn(float *floats);

        void doDrawPlotTextures(ImPlotRect c);

    public:
        void *bb_toolbox = nullptr;
    };
} // namespace satdump
