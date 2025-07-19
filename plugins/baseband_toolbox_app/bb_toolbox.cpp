#include "bb_toolbox.h"
#include "bb_container.h"
#include "core/resources.h"
#include "core/style.h"
#include "dsp/fft/fft_gen.h"
#include "dsp/io/iq_types.h"
#include "imgui/imgui.h"
#include "imgui/implot/implot.h"
#include "utils/time.h"
#include <filesystem>
#include <memory>

namespace satdump
{
    BasebandToolboxHandler::BasebandToolboxHandler(std::shared_ptr<BBContainer> c) : current_baseband_container(c)
    {
        c->bb_toolbox = this;
        handler_tree_icon = u8"\u03c9";
    }
    BasebandToolboxHandler::BasebandToolboxHandler()
    {
        //
        handler_tree_icon = u8"\u03c9";

        {
            palettes_str = "";
            std::filesystem::recursive_directory_iterator paletteIterator(resources::getResourcePath("waterfall"));
            std::error_code iteratorError;
            while (paletteIterator != std::filesystem::recursive_directory_iterator())
            {
                if (!std::filesystem::is_directory(paletteIterator->path()))
                {
                    auto map = colormaps::loadMap(paletteIterator->path().string());
                    palettes.push_back(map);
                    palettes_str += map.name + " [" + map.author + "]" + '\0';
                }

                paletteIterator.increment(iteratorError);
                if (iteratorError)
                    logger->critical(iteratorError.message());
            }
            //
        }

        fft_gen = std::make_shared<ndsp::FFTGenBlock>();
        file_source = std::make_shared<ndsp::FileSourceBlock>();
        file_opts = std::make_shared<ndsp::OptDisplayerWarper>(file_source);
        iq_type = std::make_shared<ndsp::IQType>();

        fft_gen->link(file_source.get(), 0, 0, 4);
        fft_gen->on_fft_gen_floats = [this](float *p) { current_baseband_container->pushFloatsIn(p); };
    }

    BasebandToolboxHandler::~BasebandToolboxHandler()
    {
        //
    }

    void BasebandToolboxHandler::applyParams()
    {
        iq_type.get();

        bb_type = std::filesystem::path(select_baseband_dialog.getPath()).extension().string().erase(0, 1);

        samplerate = samprate.get();

        size_t filesize = current_baseband_container->get_ptr_size();

        fileDuration = filesize / (samplerate * sampleSize);

        seconds = fileDuration % 60;

        nLines = seconds / (double(fft_size) / samplerate);

        timestamp = timestamp_from_filename(current_baseband_container->getName());

        timestampPlusDuration = timestamp + seconds; // TODOREWORK check to make sure it's just as simple as adding the seconds to the timestamp

        // current_baseband_container->setNumOfLines(nLines);
        // current_baseband_container->setFFTSize(fft_size);
        // current_baseband_container->setScaleLimits(scale_min, scale_max);
        // current_baseband_container->setRate(fft_size, nLines);

        file_source->set_cfg("file", select_baseband_dialog.getPath());
        file_source->set_cfg("type", bb_type);

        fft_gen->set_fft_gen_settings(fft_size, fft_overlap, block_size);
    }

    void BasebandToolboxHandler::process()
    {
        file_source->start();
        fft_gen->start();

        // needs_to_proc = true;

        while (!file_source->d_eof)
            stopProcessing();
    }

    bool BasebandToolboxHandler::buffer_alloc(size_t size)
    {
        uint32_t *newTextureBuffer = (uint32_t *)realloc(textureBuffer, size);
        if (newTextureBuffer == nullptr)
        {
            logger->error("Cannot allocate memory for Spectrogram");
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
        uint64_t oldSize = last_x_pos * last_y_pos;
        if (size > oldSize * sizeof(uint32_t))
            memset(&textureBuffer[oldSize], 0, size - oldSize * sizeof(uint32_t));
        last_x_pos = x_pos;
        last_y_pos = y_pos;
        return true;
    }

    void BasebandToolboxHandler::push_tmp_floats(float *values)
    {

        uint8_t *ptr = current_baseband_container->get_ptr();
        size_t size = current_baseband_container->get_ptr_size();

        char name[1000];
        tmpnam(name);
        std::string tmpfile = name;
        std::ofstream file_out(tmpfile, std::ios::binary);

        float *tmp_floats = nullptr;
        size_t current_ptr_size = 0;
        // current_ptr_size++;

        // file_out.write((char *)&vf, current_ptr_size * sizeof(float));
        if (textureID == 0 || textureBuffer == nullptr)
            return;

        if ((fft_img_i++ % fft_img_i_mod) == 0)
        {
            if (fft_img_i * 5e6 == fft_img_i_mod)
                fft_img_i = 0;

            memmove(&textureBuffer[x_pos * 1], &textureBuffer[x_pos * 0], x_pos * (y_pos - 1) * sizeof(uint32_t));

            double fz = (double)fft_size / (double)x_pos;
            for (int i = 0; i < x_pos; i++)
            {
                float fft_pos = i * fz;

                if (fft_pos >= fft_size)
                    fft_pos = fft_size - 1;

                float final = -INFINITY;
                for (float v = fft_pos; v < fft_pos + 1; v += 1)
                    if (final < values[(int)floor(v)])
                        final = values[(int)floor(v)];

                int v = ((final - scale_min) / fabs(scale_max - scale_min)) * resolution;

                if (v < 0)
                    v = 0;
                if (v >= resolution)
                    v = resolution - 1;

                current_ptr_size++;

                tmp_floats[i] = palette[v];
                textureBuffer[i] = palette[v];
                file_out.write((char *)tmp_floats, current_ptr_size);
            }
            has_to_update = true;
        }
        file_out.close();

        std::shared_ptr<satdump::BBContainer> newfloatc = std::make_shared<satdump::BBContainer>(current_baseband_container->getName() + " Flaots", tmpfile);
        newfloatc->d_is_temp = true;
        // newfloatc->doUpdateTextures();
        if (current_baseband_container->bb_toolbox != nullptr)
            ((BasebandToolboxHandler *)current_baseband_container->bb_toolbox)->addSubHandler(std::make_shared<BasebandToolboxHandler>(newfloatc));
        else
            logger->error("Can't add container!");
    }

    void BasebandToolboxHandler::set_rate(int input_rate, int output_rate)
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

    void BasebandToolboxHandler::set_palette(colormaps::Map selectedPalette, bool mutex)
    {
        if (mutex)
            work_mtx.lock();
        palette = colormaps::generatePalette(selectedPalette, resolution);
        if (mutex)
            work_mtx.unlock();
    }

    void BasebandToolboxHandler::stopProcessing()
    {
        file_source->stop();
        fft_gen->stop();
        is_busy = false;
    }

    void BasebandToolboxHandler::drawMenu()
    {
        if (is_busy)
            style::beginDisabled();
        if (ImGui::CollapsingHeader("Files##basebandToolBox", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Load File :");
            if (select_baseband_dialog.draw() && select_baseband_dialog.isValid())
            {
                try
                {
                    current_baseband_container = std::make_shared<BBContainer>(std::filesystem::path(select_baseband_dialog.getPath()).stem().string() +
                                                                                   std::filesystem::path(select_baseband_dialog.getPath()).stem().extension().string(),
                                                                               select_baseband_dialog.getPath());
                    current_baseband_container->bb_toolbox = this;
                }
                catch (std::exception &e)
                {
                    logger->error("Could not load file: %s", e.what());
                }
            }

            ImGui::Separator();
        }
        iq_type->draw_combo();

        ImGui::Separator();

        if (ImGui::CollapsingHeader("FFT Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Combo("FFT Size", &selected_fft_size,
                             "2097152\0"
                             "1048576\0"
                             "524288\0"
                             "262144\0"
                             "131072\0"
                             "65536\0"
                             "32768\0"
                             "16384\0"
                             "8192\0"
                             "4096\0"
                             "2048\0"
                             "1024\0"))
            {
                fft_size = fft_sizes_lut[selected_fft_size];
                applyParams();
            }
            if (ImGui::InputInt("FFT Overlap", &fft_overlap))
            {
                fft_size = fft_sizes_lut[selected_fft_size];
                applyParams();
            }
            widgets::SteppedSliderFloat("FFT Max", &scale_max, -160, 150);
            widgets::SteppedSliderFloat("FFT Min", &scale_min, -160, 150);
            if (ImGui::Combo("Palette", &selected_palette, palettes_str.c_str()))
                set_palette(palettes[selected_palette], true);
            // current_baseband_container->setPallete(palettes[selected_palette], true);
            ImGui::Separator();
        }
        if (is_busy)
            style::endDisabled();
        if (ImGui::Button("Process"))
        {
            applyParams();
            is_busy = true;
            process();
        }
    }

    void BasebandToolboxHandler::drawMenuBar()
    {
        //
    }

    void BasebandToolboxHandler::drawContents(ImVec2 win_size)
    {
        //
        ImVec2 window_size = win_size;

        ImPlot::BeginPlot("MainPlot", window_size);
        ImPlot::SetupAxes(nullptr, nullptr, 0, ImPlotAxisFlags_Invert);

        ImPlotRect c = ImPlot::GetPlotLimits();
        if (current_baseband_container && needs_to_proc)
            current_baseband_container->doDrawPlotTextures(c);

        ImPlot::EndPlot();
    }

} // namespace satdump
