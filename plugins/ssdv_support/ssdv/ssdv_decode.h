#pragma once

#include "instruments/ssdv-ng/ssdv.h"
#include "pipeline/module.h"
#include "pipeline/modules/base/filestream_to_filestream.h"
#include "pipeline/modules/instrument_utils.h"
#include <cstdint>
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
        ssdvng::SSDVNGReader ssdv_ng_reader;
        instrument_status_t ssdv_ng_status = DECODING;

    protected:
        // uint8_t *net_buf;
        // int pkt_size;
        std::string addr;
        int port;

    private:
        // net::UDPClient *client;

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
