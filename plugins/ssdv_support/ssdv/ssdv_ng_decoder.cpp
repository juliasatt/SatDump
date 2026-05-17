#include "ssdv_ng_decoder.h"
#include "common/codings/randomization.h"
#include "common/codings/reedsolomon/reedsolomon.h"
#include "common/utils.h"
#include "pipeline/modules/base/filestream_to_filestream.h"

#define BUFFER_SIZE 8192 * 2

uint64_t getFilesize(std::string filepath);

namespace ssdv
{
    SSDVDecoderModule::SSDVDecoderModule(std::string input_file, std::string output_file_hint, nlohmann::json parameters)
        : satdump::pipeline::base::FileStreamToFileStreamModule(input_file, output_file_hint, parameters), d_viterbi_outsync_after(parameters["viterbi_outsync_after"].get<int>()),
          d_viterbi_ber_thresold(parameters["viterbi_ber_thresold"].get<float>()), viterbi(d_viterbi_ber_thresold, d_viterbi_outsync_after, BUFFER_SIZE)
    {
        viterbi_out = new uint8_t[BUFFER_SIZE * 2];
        soft_buf = new int8_t[BUFFER_SIZE];
        deframer.STATE_SYNCED = 18;

        fsfsm_file_ext = ".cadu";
    }

    SSDVDecoderModule::~SSDVDecoderModule()
    {
        delete[] viterbi_out;
        delete[] soft_buf;
    }

    void SSDVDecoderModule::process()
    {
        reedsolomon::ReedSolomon rs(reedsolomon::RS223);

        uint8_t frame_buf[1024 * 10];

        int noSyncRuns = 0;

        while (should_run())
        {
            read_data((uint8_t *)soft_buf, BUFFER_SIZE);

            int num_samps = viterbi.work(soft_buf, BUFFER_SIZE, viterbi_out);

            if (num_samps > 0)
            {
                int frames = deframer.work(viterbi_out, num_samps, frame_buf);

                if (deframer.getState() == deframer.STATE_SYNCING)
                {
                    noSyncRuns++;

                    if (noSyncRuns >= 10)
                    {
                        viterbi.reset();
                        noSyncRuns = 0;
                    }
                }
                else
                {
                    noSyncRuns = 0;
                }

                for (int i = 0; i < frames; i++)
                {
                    uint8_t *cadu = &frame_buf[i * 1024];

                    derand_ccsds(&cadu[4], 1024 - 4);

                    rs.decode_interlaved(&cadu[4], true, 4, errors);

                    write_data(cadu, 1024);
                }
            }
        }

        cleanup();
    }

    nlohmann::json SSDVDecoderModule::getModuleStats()
    {
        auto v = satdump::pipeline::base::FileStreamToFileStreamModule::getModuleStats();
        v["deframer_lock"] = deframer.getState() == deframer.STATE_SYNCED;
        v["viterbi_ber"] = viterbi.ber();
        v["viterbi_lock"] = viterbi.getState();
        v["rs_avg"] = (errors[0] + errors[1] + errors[2] + errors[3]) / 4;
        std::string viterbi_state = viterbi.getState() == 0 ? "NOSYNC" : "SYNCED";
        std::string deframer_state = deframer.getState() == deframer.STATE_NOSYNC ? "NOSYNC" : (deframer.getState() == deframer.STATE_SYNCING ? "SYNCING" : "SYNCED");
        v["viterbi_state"] = viterbi_state;
        v["deframer_state"] = deframer_state;
        return v;
    }

} // namespace ssdv
