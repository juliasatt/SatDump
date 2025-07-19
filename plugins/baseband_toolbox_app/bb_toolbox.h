#pragma once

#include "bb_container.h"
#include "common/colormaps.h"
#include "common/widgets/menuitem_fileopen.h"
#include "dsp/device/options_displayer_warper.h"
#include "dsp/fft/fft_gen.h"
#include "dsp/io/file_source.h"
#include "dsp/io/iq_types.h"
#include "handlers/handler.h"
#include "imgui/dialogs/widget.h"
#include "imgui/imgui.h"
#include "libs/ctpl/ctpl_stl.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

namespace satdump
{
    class BasebandToolboxHandler : public handlers::Handler
    {
    protected:
        bool is_busy = false;

        void drawMenu();
        void drawContents(ImVec2 win_size);
        void drawMenuBar();
        bool active = false;

        bool buffer_alloc(size_t size);

    private:
        std::mutex work_mtx;

    private:
        FileSelectWidget select_baseband_dialog = FileSelectWidget("Baseband File", "Select Baseband File", false, true);

        std::shared_ptr<BBContainer> current_baseband_container;

    private:
        float process_progress = 0;
        ctpl::thread_pool process_threadp = ctpl::thread_pool(4);

    private:
        size_t fileDuration;
        double seconds;

        double timestamp;
        double timestampPlusDuration;

        std::string file_name_holder;

    private:
        bool has_to_update = false;
        unsigned int textureID = 0;
        uint32_t *textureBuffer = nullptr;
        const size_t d_nLinesToUpdate = 10;
        const int resolution = 65536;

        std::vector<uint32_t> palette;

        int fft_img_i_mod = 0;
        int fft_img_i = 0;
        int last_x_pos = 0;
        int last_y_pos = 0;
        int x_pos;
        int y_pos;

    private:
        uint64_t samplerate = 6e6;
        bool stereo = true;
        int bbChannels = 2;
        int sampleSize = 8;

        int fft_size = 8192;
        int fft_overlap = 1;
        int selected_fft_size = 8;
        std::vector<int> fft_sizes_lut = {2097152, 1048576, 524288, 262144, 131072, 65536, 32768, 16384, 8192, 4096, 2048, 1024};
        float scale_min = -160;
        float scale_max = 160;

        std::vector<colormaps::Map> palettes;
        std::string palettes_str;
        int selected_palette = 0;

        uint64_t nLines = 100;
        int block_size = 131072;

        bool needs_to_proc = false;

    public:
        BasebandToolboxHandler();
        BasebandToolboxHandler(std::shared_ptr<BBContainer> c);
        ~BasebandToolboxHandler();

        void process();
        void stopProcessing();
        void applyParams();
        void push_tmp_floats(float *values);
        void set_palette(colormaps::Map selectedPalette, bool mutex = true);
        void set_rate(int input_rate, int output_rate);

    public:
        std::shared_ptr<ndsp::FileSourceBlock> file_source;
        std::shared_ptr<ndsp::OptDisplayerWarper> file_opts;
        std::shared_ptr<ndsp::FFTGenBlock> fft_gen;

        std::shared_ptr<ndsp::IQType> iq_type;

        widgets::NotatedNum<uint64_t> samprate = widgets::NotatedNum<uint64_t>("Samplerate", 6e6, "S/s");

        std::string bb_type;

    public:
        std::string getID() { return "baseband_toolbox_handler"; }
        std::string getName()
        {
            if (current_baseband_container)
                return current_baseband_container->getName();
            else
                return "Baseband Toolbox";
        }
    };
} // namespace satdump
