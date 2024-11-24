#pragma once

#include "uhrit_file.h"
#include "common/ccsds/ccsds_aos/demuxer.h"
#include <memory>
#include <functional>

namespace uhrit
{
    class UHRITDemux
    {
    private:
        const int d_mpdu_size;
        const bool d_check_crc;

    private:
        std::map<int, std::unique_ptr<ccsds::ccsds_aos::Demuxer>> demuxers;
        std::map<int, std::map<int, UHRITFile>> wip_files;

        std::vector<UHRITFile> files;

    private:
        void processUHRITHeader(UHRITFile &file, ccsds::CCSDSPacket &pkt);
        void parseHeader(UHRITFile &file);
        void processUHRITData(UHRITFile &file, ccsds::CCSDSPacket &pkt, bool bad_crc = false);
        void finalizeUHRITData(UHRITFile &file);

    public:
        std::function<void(UHRITFile &)> onParseHeader =
            [](UHRITFile &) -> void {};
        std::function<bool(UHRITFile &, ccsds::CCSDSPacket &, bool)> onProcessData =
            [](UHRITFile &, ccsds::CCSDSPacket &, bool) -> bool
        { return true; };
        std::function<void(UHRITFile&)> onFinalizeData =
            [](UHRITFile&) -> void {};

    public:
        UHRITDemux(int mpdu_size = 2034, bool check_crc = true);
        ~UHRITDemux();

        std::vector<UHRITFile> work(uint8_t *cadu);
    };
};
