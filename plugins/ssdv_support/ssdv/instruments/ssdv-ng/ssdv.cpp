#include "ssdv.h"
#include <cstdint>

namespace ssdv
{
    namespace ssdvng
    {
        SSDVNGReader::SSDVNGReader() {}
        SSDVNGReader::~SSDVNGReader() {}

        void SSDVNGReader::work(ccsds::CCSDSPacket &packet)
        {
            // uint8_t *dat = &packet.payload[6];
            // if (packet.payload.size() )
        }
    } // namespace ssdvng
} // namespace ssdv
