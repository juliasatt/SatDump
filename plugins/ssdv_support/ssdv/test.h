#pragma once

#include "nlohmann/json.hpp"
#include "pipeline/module.h"
#include "pipeline/modules/base/filestream_to_filestream.h"
#include "pipeline/modules/instrument_utils.h"
#include <fcntl.h>
#include <memory>
#include <string>

namespace ssdv
{
    namespace instruments
    {
        class SSDVInstrumentsDecoderModule : public satdump::pipeline::base::FileStreamToFileStreamModule
        {
        protected:
            instrument_status_t ssdv_status = DECODING;

        public:
            SSDVInstrumentsDecoderModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
            void procress();
            void drawUI(bool window);

        public:
            static std::string getID();
            virtual std::string getIDM() { return getID(); };
            static nlohmann::json getParams() { return {}; };
            static std::shared_ptr<ProcessingModule> getInstance(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
        };
    } // namespace instruments
} // namespace ssdv
