#include "ssdv_decode.h"
#include "common/ccsds/ccsds.h"
#include "common/ccsds/ccsds_aos/demuxer.h"
#include "common/ccsds/ccsds_aos/vcdu.h"
#include "common/net/udp.h"
#include "core/exception.h"
#include "core/style.h"
#include "imgui/imgui.h"
#include "libs/supernovas/eph_manager.h"
#include "logger.h"
#include "pipeline/module.h"
#include "products/dataset.h"
#include "utils/stats.h"
#include <cstdint>
#include <fstream>
#include <memory>
#include <nng/nng.h>
#include <string>
#include <vector>

namespace ssdv
{
    SSDVInstrumentsDecoderModule::SSDVInstrumentsDecoderModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters)
        : satdump::pipeline::base::FileStreamToFileStreamModule(input_file, output_file_hint, parameters)
    {
        // if (parameters.contains("pkt_size") > 0)
        //     pkt_size = parameters["pkt_size"].get<int>();
        // else
        //     throw satdump_exception("pkt_size parameter must be present!");

        if (parameters.count("server_address") > 0)
            addr = parameters["server_address"].get<std::string>();
        else
            throw satdump_exception("server_address parameter must be present!");

        if (parameters.count("server_port") > 0)
            port = parameters["server_port"].get<int>();
        else
            throw satdump_exception("server_port parameter must be present!");
    }

    void SSDVInstrumentsDecoderModule::process()
    {
        uint8_t cadu[1024];

        // net_buf = new uint8_t[pkt_size];

        net::UDPClient udp_send((char *)addr.c_str(), port);

        logger->info("Meow :3");
        logger->info("Demultiplexing and deframing...");

        ccsds::ccsds_aos::Demuxer demuxer_vcid0(884, false);

        // std::ofstream output("output.ccsds");

        std::vector<uint8_t> ssdv_scids;

        while (should_run())
        {
            read_data((uint8_t *)cadu, 1024);

            ccsds::ccsds_aos::VCDU vcdu = ccsds::ccsds_aos::parseVCDU(cadu);

            // client->send(cadu, 1024);

            if (vcdu.vcid == 0)
            {
                std::vector<ccsds::CCSDSPacket> ccsdsFrames = demuxer_vcid0.work(cadu);
                for (ccsds::CCSDSPacket &pkt : ccsdsFrames)
                    if (pkt.header.apid == 10)
                    {
                        ssdv_ng_reader.work(pkt);
                        // pkt.payload.resize(65536);
                        // output.write((char *)pkt.header.raw, 6);
                        // output.write((char *)pkt.payload.data(), 1024);
                        // ssdv_reader.work(pkt);
                    }
                    else if (pkt.header.apid == 20 && pkt.payload.size() == 188)
                    {
                        // logger->debug("Length : %d Size : %d", pkt.header.packet_length, pkt.payload.size());
                        // output.write((char *)pkt.payload.data(), pkt.payload.size());
                        udp_send.send(pkt.payload.data(), 188);
                    }
            }
        }

        // delete[] net_buf;

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
            ImGui::TextColored(style::theme.green, "%d", ssdv_ng_reader.img_cnt);
            ImGui::TableSetColumnIndex(2);
            drawStatus(ssdv_ng_status);

            // TODOREWORK: table with packets sent?

            ImGui::EndTable();
        }

        // ImGui::BeginGroup();
        // {
        //     ImGui::Button("Network Server", {200 * ui_scale, 20 * 20 * ui_scale});
        //     {
        //
        //         ImGui::Text("Address  : ");
        //         ImGui::SameLine();
        //         ImGui::TextColored(style::theme.green, "%s", addr.c_str());
        //
        //         ImGui::Text("Port    : ");
        //         ImGui::SameLine();
        //         ImGui::TextColored(style::theme.green, UITO_C_STR(port));
        //     }
        // }
        //
        // ImGui::EndGroup();

        if (!d_is_streaming_input)
            drawProgressBar();

        ImGui::End();

        ImGui::Begin("Network Server", NULL, window ? 0 : NOWINDOW_FLAGS);

        ImGui::Text("Address  : ");
        ImGui::SameLine();
        ImGui::TextColored(style::theme.green, "%s", addr.c_str());

        ImGui::Text("Port    : ");
        ImGui::SameLine();
        ImGui::TextColored(style::theme.green, UITO_C_STR(port));

        ImGui::End();
    }

    std::string SSDVInstrumentsDecoderModule::getID() { return "ssdv_instruments"; }

    std::shared_ptr<satdump::pipeline::ProcessingModule> SSDVInstrumentsDecoderModule::getInstance(std::string input_file, std::string output_file_hint, nlohmann::json parameters)
    { return std::make_shared<SSDVInstrumentsDecoderModule>(input_file, output_file_hint, parameters); }

} // namespace ssdv
