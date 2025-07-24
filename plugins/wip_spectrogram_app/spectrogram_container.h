#pragma once

#include "common/dsp/io/baseband_interface.h"
#include "dsp/device/options_displayer_warper.h"
#include "dsp/fft/fft_gen.h"
#include "dsp/io/file_source.h"
#include "dsp/io/iq_types.h"
#include "imgui/implot/implot.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace satdump
{
    class SpectrogramContainer
    {
    private:
        const std::string d_name;
        const std::string d_filePath;
        size_t d_fileMemSize;
        uint8_t *d_fileMemPtr;
        int fd;
        char unique_id[16] = {0};

    private:
        unsigned int textureID = 0;
        uint32_t *textureBuffer = nullptr;

        std::vector<uint32_t> palette;

        bool force_update_all = false;
        bool update = false;

    private:
        uint32_t d_blockSize;
        size_t d_bufferSize;

        float *floatsInBuffer;

        int fft_img_i_mod = 0;
        int fft_img_i = 0;
        int last_x_pos = 0;
        int last_y_pos = 0;
        int x_pos;
        int y_pos;

    public:
        struct InDef
        {
            size_t ptr;
            size_t size;
        };

        std::vector<InDef> ins;

    public:
        // DSP
        // std::shared_ptr<ndsp::FFTGenBlock> fft_gen;
        // std::shared_ptr<ndsp::FileSourceBlock> file_source;
        // std::shared_ptr<ndsp::OptDisplayerWarper> file_opts;
        //
        // std::string baseBandType;
        //
        // widgets::NotatedNum<uint64_t> samprate = widgets::NotatedNum<uint64_t>("Samplerate", 6e6, "S/s");
        // uint64_t sampleRate = 6e6;
        // uint64_t nOfLines = 1;
        // int fftSize = 8192;
        // int fftOverlap = 1;
        // int sampleSize = 8;
        // int lengthInSecs;
        // double timeStamp;

    public:
        // size_t d_resolution = 65535;

        uint8_t *get_ptr() { return d_fileMemPtr; }
        size_t get_ptr_size() { return d_fileMemSize; }
        std::string getFilePath() { return d_filePath; }

        bool d_isTmp = false;

    public:
        SpectrogramContainer(std::string name, std::string file);
        ~SpectrogramContainer();

        std::string getName() { return d_name; }
        std::string getID() { return std::string(unique_id); }

        // void initFFT();
        // void applyParams(uint64_t number_of_lines, int fft_size, int fft_overlap, int seconds, double file_timestamp);
        // void forceUpdateAll() { force_update_all = true; }
        //
        // void pushFloats(float *floats);
        // void stopDSP();
        // void setRate(uint32_t block_size);
        //
        // void doUpdateTextures();
        void doDrawPlotTextures(ImPlotRect c);

    public:
        void *spectrogram = nullptr;
        // void *floats = nullptr;
    }; // namespace spectrogram
} // namespace satdump
