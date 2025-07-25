#include "spectrogram.h"
#include "common/geodetic/wgs84.h"
#include "image/image.h"
#include "imgui/imgui.h"
#include "spectrogram_container.h"
#include "tools/spectrogram_image.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>

/* TODOREWORK fix/make plugin actually not "freeze" SatDump
 * while it's processing, also so itn have a proper/working
 * progress bar.
 */

namespace satdump
{
    // SpectrogramHandler::SpectrogramHandler(std::shared_ptr<SpectrogramContainer> *floats) : current_spectrogram_container(*floats)
    // {
    //     *floats->spectrogram = this;
    //     handler_tree_icon = u8"\u03c9";
    //
    //     all_spec_tools.push_back(std::make_shared<SpectrogramGenTool>());
    // }
    SpectrogramHandler::SpectrogramHandler(std::shared_ptr<SpectrogramContainer> c) : current_spectrogram_container(c)
    {
        c->spectrogram = this;
        handler_tree_icon = u8"\u03c9";

        all_spec_tools.push_back(std::make_shared<SpectrogramGenTool>());
    }
    SpectrogramHandler::SpectrogramHandler()
    {
        handler_tree_icon = u8"\u03c9";

        all_spec_tools.push_back(std::make_shared<SpectrogramGenTool>());

        /* TODOREWORK fix file_save_menu maybe like the example below
         * that wasn't just ToTaLlY copied from lut_generator.cpp
         * :)
         */

        image_save_menu.getimg_callback = [this]()
        {
            auto img = std::make_shared<image::Image>();
            *img = genImage();
            return img;
        };

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
        }

        fft_gen = std::make_shared<ndsp::FFTGenBlock>();
        file_source = std::make_shared<ndsp::FileSourceBlock>();
        // file_opts = std::make_shared<ndsp::OptDisplayerWarper>(file_source);

        fft_gen->link(file_source.get(), 0, 0, 4);
        fft_gen->on_fft_gen_floats = [this](float *p) { push_fft_floats(p); };
    }

    SpectrogramHandler::~SpectrogramHandler()
    {
        //
        delete[] textureBuffer;
    }

    void SpectrogramHandler::applyParams()
    {
        bbType = std::filesystem::path(file_open_menu.getPath()).extension().string().erase(0, 1);

        samplerate = samprate.get();

        uint64_t filesize = getFilesize(file_open_menu.getPath());

        if (bbType == "cf32" || bbType == "cs32" || bbType == "s32" || bbType == "f32")
        {
            sampleSize = 4 * bbChannels;
        }
        else if (bbType == "cs16" || bbType == "s16" || bbType == "wav16" || bbType == "wav")
        {
            sampleSize = 2 * bbChannels;
        }
        else if (bbType == "cs8" || bbType == "cu8" || bbType == "s8" || bbType == "u8")
        {
            sampleSize = 1 * bbChannels;
        }

        int fileDurationTime = filesize / (samplerate * sampleSize);

        // uint64_t fr_hz = samplerate / 16384.0; // FFT Frequency resolution

        // int days = fileDurationTime / (24 * 3600);
        // int hours = (fileDurationTime % (24 * 3600)) / 3600;
        // int minutes = (fileDurationTime % 3600) / 60;
        seconds = fileDurationTime % 60;

        nLines = seconds / (double(fft_size) / samplerate);

        fileName = std::filesystem::path(file_open_menu.getPath()).stem().stem();

        timestamp = timestamp_from_filename(fileName);

        timestampAndDuration = timestamp + seconds;

        set_rate(fft_size, nLines);

        // current_spectrogram_container->applyParams(nLines, fft_size, fft_overlap, seconds, timestamp);

        fft_gen->set_fft_gen_settings(fft_size, fft_overlap, block_size);

        file_source->set_cfg("file", file_open_menu.getPath());
        file_source->set_cfg("type", bbType);

        // file_opts->update();
    }

    void SpectrogramHandler::process()
    {
        file_source->start();
        fft_gen->start();

        while (!file_source->d_eof)
            stop();
    }

    bool SpectrogramHandler::buffer_alloc(size_t size)
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

    // void SpectrogramHandler::push_tmp_floats(float *values)
    // {
    //     // float *vf = values;
    //
    //     uint8_t *ptr = current_spectrogram_container->get_ptr();
    //     size_t size = current_spectrogram_container->get_ptr_size();
    //
    //     char name[1000];
    //     tmpnam(name);
    //     std::string tmpfile = name;
    //     std::ofstream file_out(tmpfile, std::ios::binary);
    //
    //     float *tmp_floats = nullptr;
    //     size_t current_ptr_size = 0;
    //     current_ptr_size++;
    //
    //     // file_out.write((char *)&vf, current_ptr_size * sizeof(float));
    //     if (textureID == 0 || textureBuffer == nullptr)
    //         return;
    //
    //     if ((fft_img_i++ % fft_img_i_mod) == 0)
    //     {
    //         if (fft_img_i * 5e6 == fft_img_i_mod)
    //             fft_img_i = 0;
    //
    //         memmove(&textureBuffer[x_pos * 1], &textureBuffer[x_pos * 0], x_pos * (y_pos - 1) * sizeof(uint32_t));
    //
    //         double fz = (double)fft_size / (double)x_pos;
    //         for (int i = 0; i < x_pos; i++)
    //         {
    //             float fft_pos = i * fz;
    //
    //             if (fft_pos >= fft_size)
    //                 fft_pos = fft_size - 1;
    //
    //             float final = -INFINITY;
    //             for (float v = fft_pos; v < fft_pos + 1; v += 1)
    //                 if (final < values[(int)floor(v)])
    //                     final = values[(int)floor(v)];
    //
    //             int v = ((final - scale_min) / fabs(scale_max - scale_min)) * resolution;
    //
    //             if (v < 0)
    //                 v = 0;
    //             if (v >= resolution)
    //                 v = resolution - 1;
    //
    //             current_ptr_size++;
    //
    //             tmp_floats[i] = palette[v];
    //             textureBuffer[i] = palette[v];
    //             file_out.write((char *)tmp_floats, current_ptr_size * sizeof(float));
    //         }
    //         has_to_update = true;
    //     }
    //     file_out.close();
    //
    //     std::shared_ptr<satdump::SpectrogramContainer> newfloatc = std::make_shared<satdump::SpectrogramContainer>(current_spectrogram_container->getName() + " Flaots", tmpfile);
    //     newfloatc->d_isTmp = false;
    //     // newfloatc->doUpdateTextures();
    //     if (current_spectrogram_container->spectrogram != nullptr)
    //         ((SpectrogramHandler *)current_spectrogram_container->spectrogram)->addSubHandler(std::make_shared<SpectrogramHandler>(newfloatc));
    //     else
    //         logger->error("Can't add container!");
    // }

    void SpectrogramHandler::push_fft_floats(float *values)
    {
        if (textureID == 0 || textureBuffer == nullptr)
            return;

        work_mtx.lock();
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

                textureBuffer[i] = palette[v];
            }

            has_to_update = true;
        }

        work_mtx.unlock();
    }

    void SpectrogramHandler::set_rate(int input_rate, int output_rate)
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

    void SpectrogramHandler::set_palette(colormaps::Map selectedPalete, bool mutex)
    {
        if (mutex)
            work_mtx.lock();
        palette = colormaps::generatePalette(selectedPalete, resolution);
        if (mutex)
            work_mtx.unlock();
    }

    void SpectrogramHandler::stop()
    {
        file_source->stop();
        fft_gen->stop();
        is_busy = false;
    }

    void SpectrogramHandler::drawMenu()
    {
        if (is_busy)
            style::beginDisabled();
        if (ImGui::CollapsingHeader("Files##spectrogram", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Load File :");
            if (file_open_menu.update())
            {
                try
                {
                    current_spectrogram_container = std::make_shared<SpectrogramContainer>(
                        std::filesystem::path(file_open_menu.getPath()).stem().string() + std::filesystem::path(file_open_menu.getPath()).stem().extension().string(), file_open_menu.getPath());

                    current_spectrogram_container->spectrogram = this; // TODO: this is where the spectrogram_container is set to the file path and file size. What I need to do is change this so it
                                                                       // points to the floats generated by the FFTGenBlock.
                    // FIX: so what I have to do here is place "spectrogram_container->spectrogram = this;" where the FFT is generated, pointing to the filename and the appropriate ptr (generated
                    // floats) of the FFTGenBlock.

                    applyParams();
                }
                catch (std::exception &e)
                {
                    logger->error("Could not load file: %s", e.what());
                }
            }

            ImGui::Separator();
        }

        // file_opts->draw();

        ImGui::Separator();

        if (ImGui::Checkbox("Stereo", &stereo))
        {
            if (stereo)
            {
                bbChannels = 2;
            }
            else
            {
                bbChannels = 1;
            }
        }

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
                // fft_gen->set_fft_gen_settings(fft_size, fft_overlap, block_size);
            }
            if (ImGui::InputInt("FFT Overlap", &fft_overlap))
            {
                fft_size = fft_sizes_lut[selected_fft_size];
                applyParams();
                // fft_gen->set_fft_gen_settings(fft_size, fft_overlap, block_size);
            }
            widgets::SteppedSliderFloat("FFT Max", &scale_max, -160, 150);
            widgets::SteppedSliderFloat("FFT Min", &scale_min, -160, 150);
            if (ImGui::Combo("Palette", &selected_palette, palettes_str.c_str()))
                set_palette(palettes[selected_palette], true);
            ImGui::Separator();
        }
        //
        // if (ImGui::CollapsingHeader("Control"))
        // {
        //     if (spectrogram_container)
        //     {
        //         if (ImGui::Button("Generate FFT"))
        //         {
        //             spectrogram_container;
        //         }
        //     }
        // }
        //
        if (is_busy)
            style::endDisabled();
        if (ImGui::CollapsingHeader("Tools", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (current_spectrogram_container)
            {
                for (auto &tool : all_spec_tools)
                {
                    ImGui::Separator();
                    ImGui::Text("%s", tool->getName().c_str());
                    ImGui::Separator();

                    tool->renderMenu(current_spectrogram_container, is_busy);

                    if (tool->needToProcess())
                    {
                        tool->setProcessed();
                        auto func = [this, tool](int)
                        {
                            tool->process(current_spectrogram_container, process_progress);
                            is_busy = false;
                        };
                        is_busy = true;
                        process_threadp.push(func);
                    }
                }

                ImGui::Spacing();
                ImGui::ProgressBar(process_progress);
            }
            if (ImGui::Button("Generate"))
            {
                is_busy = true;
                process();
            }
            ImGui::Separator();
        }

        // ImGui::Separator();
        // ImGui::Text("Progress");
        //
        // float prog = 0;
        // if (is_busy)
        //     prog = (float)file_source->d_progress; // TODOREWORK make it so we actually use the progress of the WHOLE plugin, not just the file_source progress
        // ImGui::ProgressBar(prog);
    }

    image::Image SpectrogramHandler::genImage()
    {
        image::Image SpecImg(16, fft_size, nLines, 4);
        for (size_t x = 0; x < fft_size; x++)
        {
            for (size_t y = 0; y < nLines; y++)
            {
                SpecImg.set(0, x, y, spec_img[x].r);
                SpecImg.set(1, x, y, spec_img[x].g);
                SpecImg.set(2, x, y, spec_img[x].b);
                SpecImg.set(3, x, y, spec_img[x].a);
                // SpecImg.set(0, x, y, textureBuffer[x]);
                // SpecImg.set(1, x, y, textureBuffer[x]);
                // SpecImg.set(2, x, y, textureBuffer[x]);
                // SpecImg.set(3, x, y, textureBuffer[x]);
            }
        }
        return SpecImg;
    }

    void SpectrogramHandler::drawMenuBar()
    {
        file_open_menu.render("Load Baseband File", "Select Baseband File", ".", {{"All Files", "*"}});
        image_save_menu.render("Save Image to File", "baseband", ".", "Save Image",
                               true); // TODOREWORK change "baseband" to include the name of the original file, maybe make it the same timestamp/date/preset as the original?
    }

    void SpectrogramHandler::drawContents(ImVec2 win_size)
    {
        ImVec2 window_size = win_size;

        work_mtx.lock();

        if (textureID == 0 || is_busy)
        {
            // x_pos = fft_size;
            // y_pos = nLines;
            x_pos = window_size.x > fft_size ? fft_size : window_size.x;
            y_pos = window_size.y > nLines ? nLines : window_size.y;
        }
        if (textureID == 0)
        {
            textureID = makeImageTexture();
            has_to_update = buffer_alloc(x_pos * y_pos * sizeof(uint32_t));
            if ((int)palette.size() != resolution)
                set_palette(colormaps::loadMap(resources::getResourcePath("waterfall/classic.json")), false);
        }
        if (is_busy && (last_x_pos != x_pos || last_y_pos != y_pos))
        {
            if (textureBuffer != nullptr && last_x_pos != x_pos)
            {
                free(textureBuffer);
                textureBuffer = nullptr;
                last_x_pos = 0;
                last_y_pos = 0;
            }
            has_to_update = buffer_alloc(x_pos * y_pos * sizeof(uint32_t));
        }
        if (has_to_update)
        {
            updateImageTexture(textureID, textureBuffer, x_pos, y_pos);
            has_to_update = false;
        }
        work_mtx.unlock();

        ImGui::Image((void *)(intptr_t)textureID, window_size);
        // ImPlot::BeginPlot("##Baseband Spectrogram", window_size);
        // ImPlot::GetStyle().UseISO8601 = true;
        // ImPlot::GetStyle().Use24HourClock = true;
        // ImPlot::SetupAxes("Samples", "Time", ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_Invert | ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_AutoFit);
        // ImPlot::SetupAxisLimits(ImAxis_X1, 0, samplerate);
        // ImPlot::SetupAxisLimits(ImAxis_Y1, 0, seconds);
        // ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void *)"Hz");
        // ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Time);
        // ImPlot::SetupAxisTicks(ImAxis_Y1, timestamp, timestampAndDuration, seconds);
        // ImPlot::PlotImage("Spectrogram", (void *)(intptr_t)textureID, ImVec2(0, 0), ImVec2(samplerate, seconds));
        // ImPlot::EndPlot();
    }

} // namespace satdump
