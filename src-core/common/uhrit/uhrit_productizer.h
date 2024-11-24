#pragma once

#include <string>
#include "common/image/image.h"
#include "uhrit_file.h"
// #include "products/image_products.h"
#include <thread>
#include <mutex>

namespace uhrit
{
    class UHRITProductizer
    {
    private:
        bool should_sweep_x;
        std::string instrument_id = "ukn";

        std::string compo_cache_path;

    private:
        bool can_make_composites = false;

        std::thread compositeGeneratorThread;
        bool composite_th_should_run = true;
        void compositeThreadFunc();

        std::mutex compo_queue_mtx;
        std::vector<std::pair<void *, std::string>> compo_queue;

    public:
        UHRITProductizer(std::string instrument_id, bool sweep_x, std::string cache_path);
        ~UHRITProductizer();

        void saveImage(image::Image img,
                       int bit_depth,
                       std::string directory,
                       std::string satellite,
                       std::string satshort,
                       std::string channel,
                       time_t timestamp,
                       std::string region = "",
                       ImageNavigationRecord *navigationHdr = nullptr,
                       ImageDataFunctionRecord *calibHdr = nullptr);

        void setInstrumentID(std::string id) { instrument_id = id; }

        // Calibration WILL vary between sats
        // std::function<void(satdump::ImageProducts *, ImageDataFunctionRecord *)> appendCalibrationToProductFunc;
    };
}
