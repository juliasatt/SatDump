#include "test.h"
#include "common/ccsds/ccsds.h"
#include "common/ccsds/ccsds_aos/demuxer.h"
#include "common/ccsds/ccsds_aos/vcdu.h"
#include "common/net/udp.h"
#include "libs/supernovas/eph_manager.h"
#include "logger.h"
#include "pipeline/module.h"
#include "products/dataset.h"
#include "utils/stats.h"
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace ssdv
{
    SSDVInstrumentsDecoderModule::SSDVInstrumentsDecoderModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters)
        : satdump::pipeline::base::FileStreamToFileStreamModule(input_file, output_file_hint, parameters)
    {
        ip_addr = parameters.contains("ip_addr") ? parameters["ip_addr"].get<std::string>() : "127.0.0.1";
        addr_port = parameters.contains("addr_port") ? parameters["addr_port"].get<int>() : 9000;
    }

    void SSDVInstrumentsDecoderModule::process()
    {
        uint8_t cadu[1024];

        client = new net::UDPClient((char *)ip_addr.c_str(), addr_port);

        logger->info("Meow :3");

        ccsds::ccsds_aos::Demuxer demuxer_vcid0(884, false);

        std::ofstream output("output.ccsds");

        std::vector<uint8_t> ssdv_scids;

        while (should_run())
        {
            read_data((uint8_t *)cadu, 1024);

            ccsds::ccsds_aos::VCDU vcdu = ccsds::ccsds_aos::parseVCDU(cadu);

            client->send(cadu, 1024);

            // if (vcdu.vcid == 0)
            // {
            //     std::vector<ccsds::CCSDSPacket> ccsdsFrames = demuxer_vcid0.work(cadu);
            //     for (ccsds::CCSDSPacket &pkt : ccsdsFrames)
            //         if (pkt.header.apid == 1)
            //         {
            //             // pkt.payload.resize(65536);
            //             output.write((char *)pkt.header.raw, 6);
            //             output.write((char *)pkt.payload.data(), 1024);
            //             // ssdv_reader.work(pkt);
            //         }
            // }
        }

        cleanup();

        // int scid = satdump::most_common(ssdv_scids.begin(), ssdv_scids.end(), 0);
        // ssdv_scids.clear();
        //
        // std::string sat_name = "Unkown SSDV";
        // if (scid == SSDV_SCID)
        //     norad = SSDV_NORAD;

        // satdump::products::DataSet dataset;
        // dataset.satellite_name = "SSDV";

        // dataset.timestamp = satdump::get_median(nn_reader.timestamps);
    }

    void SSDVInstrumentsDecoderModule::drawUI(bool window)
    {
        ImGui::Begin("SSDV Decoder", NULL, window ? 0 : NOWINDOW_FLAGS);

        if (ImGui::BeginTable("##ssdvinstrumentstable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Instrument");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Images / Frames");
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("Status");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("SSDV Images");
            ImGui::TableSetColumnIndex(1);
            // ImGui::TextColored(style::theme.green, "%d", ssdv_reader.img_cnt);
            ImGui::TableSetColumnIndex(2);
            drawStatus(ssdv_status);

            ImGui::EndTable();
        }

        drawProgressBar();

        ImGui::End();
    }

    std::string SSDVInstrumentsDecoderModule::getID() { return "ssdv_instruments"; }

    std::shared_ptr<satdump::pipeline::ProcessingModule> SSDVInstrumentsDecoderModule::getInstance(std::string input_file, std::string output_file_hint, nlohmann::json parameters)
    { return std::make_shared<SSDVInstrumentsDecoderModule>(input_file, output_file_hint, parameters); }

} // namespace ssdv
