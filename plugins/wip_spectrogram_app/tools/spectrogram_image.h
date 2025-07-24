#pragma once

// #include "../spectrogram.h"
#include "../tool.h"
#include "core/style.h"
#include "dsp/block.h"
#include "dsp/device/options_displayer_warper.h"
#include "dsp/fft/fft_gen.h"
#include "dsp/flowgraph/node_int.h"
#include "dsp/io/file_source.h"
#include "imgui/imgui.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace satdump
{
    class SpectrogramGenTool : public SpectrogramTool
    {
    private:
        bool should_process = false;
        int d_size = 8192;

    public:
        std::string getName() { return "Spectrogram Gen Tool"; }

        void renderMenu(std::shared_ptr<SpectrogramContainer> &container, bool is_busy)
        {
            if (is_busy)
                style::beginDisabled();

            if (ImGui::Button("Generate FFT floats"))
                should_process = true;

            if (is_busy)
                style::endDisabled();
        }

        bool needToProcess()
        {
            //
            return should_process;
        }

        void setProcessed()
        {
            //
            should_process = false;
        }

        // void applyParams()

        void process(std::shared_ptr<SpectrogramContainer> &container, float &process_progess)
        {
            uint8_t *ptr = container->get_ptr();
            size_t size = container->get_ptr_size();
            char name[1000];
            tmpnam(name);
            std::string tmpfile = name;
            std::ofstream file_out(tmpfile, std::ios::binary);

            size_t floatPos = 0;
            float tmp_buf[8192];

            size_t current_ptr = 0;
            while (current_ptr < size)
            {
                size_t csize = std::min<size_t>(d_size / 8, size - current_ptr);
                if (d_size / 8 != csize)
                    break;

                memcpy(tmp_buf, ptr + current_ptr, (csize / 8) * sizeof(float));
                current_ptr += csize;

                // tmp_buf, csize

                file_out.write((char *)tmp_buf, csize * sizeof(float));

                process_progess = double(current_ptr) / double(size);
            }

            // uint8_t *ptr = container->get_ptr();
            // size_t size = container->get_ptr_size();
            // char name[1000];
            // tmpnam(name);
            // std::string tmpfile = name;
            // std::ofstream file_out(tmpfile, std::ios::binary);
            //
            // uint32_t floatPos = 0;
            // float tmp_buf[8192];
            //
            // size_t current_ptr = 0;
            // while (current_ptr < size)
            // {
            //     size_t csize = std::min<size_t>(8192, size - current_ptr);
            //
            //     memcpy(tmp_buf, ptr + current_ptr, csize * sizeof(float));
            //     current_ptr += csize;
            //
            //     // tmp_buf, csize
            //
            //     file_out.write((char *)tmp_buf, csize * sizeof(float));
            //
            //     process_progess = double(current_ptr) / double(size);
            // }

            file_out.close();

            std::shared_ptr<satdump::SpectrogramContainer> newimgc = std::make_shared<satdump::SpectrogramContainer>(container->getName() + " Floats", tmpfile);
            newimgc->d_isTmp = true;

            if (container->spectrogram != nullptr)
                ((SpectrogramHandler *)container->spectrogram)->addSubHandler(std::make_shared<SpectrogramHandler>(newimgc));
            else
                logger->error("Can't add container!");
        }
    };
} // namespace satdump
