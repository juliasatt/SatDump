#pragma once

#include "common/net/udp.h"
#include "nlohmann/json.hpp"
#include "pipeline/module.h"
#include "pipeline/modules/base/filestream_to_filestream.h"
#include "pipeline/modules/instrument_utils.h"
#include <fcntl.h>
#include <memory>
#include <string>

namespace ssdv
{
    // namespace instruments
    // {
    class SSDVInstrumentsDecoderModule : public satdump::pipeline::base::FileStreamToFileStreamModule
    {
    protected:
        std::string ip_addr = "127.0.0.1";
        int addr_port = 9000;
        instrument_status_t ssdv_status = DECODING;

    private:
        net::UDPClient *client;

    public:
        SSDVInstrumentsDecoderModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
        void process();
        void drawUI(bool window);

    public:
        static std::string getID();
        virtual std::string getIDM() { return getID(); };
        static nlohmann::json getParams() { return {}; };
        static std::shared_ptr<ProcessingModule> getInstance(std::string input_file, std::string output_file_hint, nlohmann::json parameters);
    };
    // } // namespace instruments
} // namespace ssdv
