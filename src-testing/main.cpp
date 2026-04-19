/**********************************************************************
 * This file is used for testing random stuff without running the
 * whole of SatDump, which comes in handy for debugging individual
 * elements before putting them all together in modules...
 *
 * If you are an user, ignore this file which will not be built by
 * default, and if you're a developper in need of doing stuff here...
 * Go ahead!
 *
 * Don't judge the code you might see in there! :)
 **********************************************************************/

#include "init.h"
#include "logger.h"
#include <cmath>
#include <fstream>

int main(int argc, char *argv[])
{
    initLogger();

    logger->set_level(slog::LOG_OFF);
    satdump::initSatDump();
    completeLoggerInit();
    logger->set_level(slog::LOG_TRACE);

    uint8_t frms[448];
    uint8_t frm[224];

    // codings::crc::GenericCRC crc(16, 0x8005, 0xFFFF, 0x0000, false, false);

    // std::ofstream out_0("out_arktika_0.bin");
    // // std::ofstream out_1("out_arktika_1.bin");
    // std::ofstream out_2("out_arktika_2.bin");
    // std::ofstream out_3("out_arktika_3.bin");
    // std::ofstream out_4("out_arktika_4.bin");
    // std::ofstream out_5("out_arktika_5.bin");
    // // std::ofstream out_6("out_arktika_6.bin");
    // std::ofstream out_7("out_arktika_7.bin");
    // std::ofstream out_7_sub_0("out_arktika_7_sub_0.bin");
    // std::ofstream out_7_sub_7("out_arktika_7_sub_7.bin");
    // // std::ofstream out_8("out_arktika_8.bin");
    // // std::ofstream out_9("out_arktika_9.bin");
    // std::ofstream out_10("out_arktika_10.bin");
    // std::ofstream out_11("out_arktika_11.bin");
    // std::ofstream out_12("out_arktika_12.bin");
    // std::ofstream out_13("out_arktika_13.bin");
    // std::ofstream out_14("out_arktika_14.bin");
    // std::ofstream out_15("out_arktika_15.bin");
    // // std::ofstream data_ou(argv[2]);

    // std::vector<uint8_t> full_payload;

    std::ifstream data_in(argv[1]);

    std::ofstream out_0_0("out_0_0.bin");
    std::ofstream out_1_0("out_1_0.bin");
    std::ofstream out_2_0("out_2_0.bin");
    std::ofstream out_3_0("out_3_0.bin");
    std::ofstream out_3_2("out_3_2.bin");
    std::ofstream out_4_0("out_4_0.bin");
    std::ofstream out_5_0("out_5_0.bin");
    std::ofstream out_5_12("out_5_12.bin");
    std::ofstream out_7_0("out_7_0.bin");
    std::ofstream out_7_7("out_7_7.bin");

    while (!data_in.eof())
    {
        data_in.read((char *)frms, 448);

        memset(frm, 0, 224);
        memcpy(frm, frms, 224);

        int marker = frm[4] >> 4;

        // Frame 5 (in binary) sub marker, 36 bit position, with 4 bits in length
        int sub_marker = frm[4] & 0b1111;

        // printf("Marker : %d Sub_Marker : %d\n", marker, sub_marker);

        if (marker == 0)
            out_0_0.write((char *)frm, 224);
        else if (marker == 1)
            out_1_0.write((char *)frm, 224);
        else if (marker == 2)
            out_2_0.write((char *)frm, 224);
        else if (marker == 3 && sub_marker == 0)
            out_3_0.write((char *)frm, 224);
        else if (marker == 3 && sub_marker == 2)
            out_3_2.write((char *)frm, 224);
        else if (marker == 4)
            out_4_0.write((char *)frm, 224);
        else if (marker == 5 && sub_marker == 0)
            out_5_0.write((char *)frm, 224);
        else if (marker == 5 && sub_marker == 12)
            out_5_12.write((char *)frm, 224);
        else if (marker == 7 && sub_marker == 0)
            out_7_0.write((char *)frm, 224);
        else if (marker == 7 && sub_marker == 7)
            out_7_7.write((char *)frm, 224);

        // for (int i = 0; i < 15; i++)
        // {
        //     if (i == 0)
        //         continue;
        //     {
        //         out_0_0.write((char *)frm, 224);
        //         out_7.write((char *)frm, 224);
        //         if (sub_marker == 0)
        //             out_7_sub_0.write((char *)frm, 224);
        //         else if (sub_marker == 7)
        //             out_7_sub_7.write((char *)frm, 224);
        //     }
        // }

        // uint16_t timestamp = frm[5] << 8 | frm[6];

        // if (marker == 5)
        // {
        //     uint16_t crc16 = frm[224 - 2] << 8 | frm[224 - 1];
        //     // logger->critical("CRC 16 :%d", crc16);
        //     printf("%d\n", crc16);
        // }

        // if (marker == 5)
        // {
        //     for (int i = 0; i < 224; i++)
        //         full_payload.push_back(frm[i]);
        //
        //     std::vector<uint8_t> pkt_crc;
        //     pkt_crc.insert(pkt_crc.end(), full_payload.data(), full_payload.data());
        //     pkt_crc.insert(pkt_crc.end(), full_payload.begin(), full_payload.end() - 2);
        //     uint64_t crc_frm = full_payload[full_payload.size() - 2] << 8 | full_payload[full_payload.size() - 1];
        //     uint64_t crc_com = crc.compute(pkt_crc.data(), pkt_crc.size());
        //     pkt_crc.clear();
        //
        //     logger->debug("Frame :%lld Comp :%lld", crc_frm, crc_com);
        // }

        // if (marker == 5)
        // {
        //
        //     // uint32_t data_time = frm[5] << 24 | frm[6] << 16 | frm[7] << 8 | frm[8];
        //     // double data_time = frm[5] * 3600.0 + frm[6] * 60.0 + frm[7] + frm[8];
        //
        //     // uint32_t time1 = frm[5] << 24 | frm[6] << 16 | frm[7] << 8 | frm[8];
        //     // uint32_t time2 = (frm[10] & 0b00000001) << 16 | frm[11] << 8 | frm[12];
        //     // uint32_t time3 = frm[14] << 8 | frm[15];
        //     // uint64_t timestamp = time1;
        //     // double timestamp = data_time;
        //     // // timestamp -= 1800 + 88;
        //     // double timestamp = time1 + time2 + time3 / 256.0;
        //     // timestamp += 1735204808.2837029;
        //     // timestamp -= 1800 + 88;
        //     // data_time = data_time / 256.0;
        //     // logger->trace("%f", data_time);
        // }

        // if (marker == 0)
        //     out_0.write((char *)frm, 224);
        // // else if (marker == 1)
        // //     out_1.write((char *)frm, 224);
        // else if (marker == 2)
        //     out_2.write((char *)frm, 224);
        // else if (marker == 3)
        //     out_3.write((char *)frm, 224);
        // else if (marker == 4)
        //     out_4.write((char *)frm, 224);
        // else if (marker == 5)
        //     out_5.write((char *)frm, 224);
        // // else if (marker == 6)
        // //     out_6.write((char *)frm, 224);
        // else if (marker == 7)
        // {
        //     out_7.write((char *)frm, 224);
        //     if (sub_marker == 0)
        //         out_7_sub_0.write((char *)frm, 224);
        //     else if (sub_marker == 7)
        //         out_7_sub_7.write((char *)frm, 224);
        // }
        // // else if (marker == 8)
        // //     out_8.write((char *)frm, 224);
        // // else if (marker == 9)
        // //     out_9.write((char *)frm, 224);
        // else if (marker == 10)
        //     out_10.write((char *)frm, 224);
        // else if (marker == 11)
        //     out_11.write((char *)frm, 224);
        // else if (marker == 12)
        //     out_12.write((char *)frm, 224);
        // else if (marker == 13)
        //     out_13.write((char *)frm, 224);
        // else if (marker == 14)
        //     out_14.write((char *)frm, 224);
        // else if (marker == 15)
        //     out_15.write((char *)frm, 224);

        // printf("%d\n", marker);
        // logger->trace("Timestamp : %d", timestamp);
        // data_ou.write((char *)frm, 224);
    }
}
