#include "fingerprint.h"

#include <cassert>
#include <string>
#include <sstream>
#include <ios>
#include <iomanip>

using std::string;
using std::stringstream;

fingerprint::fingerprint()
{
    /* aggregated info, always present */
    tag_presence    = 0;
    supported_rates = 0;

    /* HT capabilities, optional */
    ht_capability_info = 0;
    ht_ampduparam      = 0;
    memset(ht_mcsset, 0, HT_MCSSET_LEN);
    ht_extended_capabilities = 0;
    ht_tbc = 0;                         // Transmit Beamforming Capabilities
    ht_asel_capabilities = 0;

    /* Extended capabilities */
    memset(ext_extended_capabilities, 0, EXT_CAPABILITIES_LEN);

    /* Interworking */
    iw_interworking = 0;

    /* Multi Band */
    multi_band_id = 0;
    multi_band_channel = 0;

    /* VHT Capabilities */
    vht_capabilities_info = 0;
    memset(vht_supported_mcs_nss_set, 0, VHT_MCS_NSS_SET_LEN);

    /* ssid list */
    ssid_list.clear();
}

uint32_t fingerprint::serialize(uint8_t* buffer)
{
    uint8_t* dst = buffer + 4; // first 4 bytes are the buffer length
    uint8_t* src = (uint8_t*) &(this->tag_presence);

    for (int i = 0; i < FINGERPRINT_LEN; i++) {
        *dst++ = *src++;
    }

    uint8_t ssid_list_len = (uint8_t)ssid_list.size();

    *dst++ = ssid_list_len;
    if (ssid_list_len > 0) {
        for (int i = 0; i < ssid_list_len; i++) {
            uint8_t ssid_len = (uint8_t)ssid_list[i].size();

            *dst++ = ssid_len;
            for (int j = 0; j < ssid_len; j++)
                *dst++ = ssid_list[i][j];
        }
    }

    *((uint32_t*)buffer) = (uint32_t)(dst - buffer);

    return (uint32_t)(dst - buffer);
}

fingerprint&
fingerprint::deserialize(uint8_t* buffer, uint32_t buffer_len)
{
    uint8_t* src = buffer;
    uint8_t* dst = (uint8_t*) &(this->tag_presence);

    uint32_t length = *((uint32_t*)src);
    assert(length == buffer_len);
    src += 4; // jump over initial buffer length

    for (int i = 0; i < FINGERPRINT_LEN; i++) {
        *dst++ = *src++;
    }

    uint8_t ssid_list_len = *src++;

    if (ssid_list_len > 0) {
        for (int i = 0; i < ssid_list_len; i++) {
            uint8_t ssid_len = *src++;

            string ssid((char*)src, ssid_len);
            ssid_list.push_back(ssid);

            src += ssid_len;
        }
    }

    return *this;
}

string
fingerprint::str() const
{
    stringstream rval;
    uint8_t* src = (uint8_t*) &(tag_presence);

    // Tag Presence
    for (int i = 0; i < 4; i++)
        rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++;

    rval << '|';

    // Supported Rates
    for (int i = 0; i < 8; i++)
        rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++;

    rval << '|';

    // HT Capabilities
    for (int i = 0; i < HT_CAPABILITIES_LEN; i++)
        rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++;

    rval << '|';

    // Extended Capabilities
    for (int i = 0; i < EXT_CAPABILITIES_LEN; i++)
        rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++;

    rval << '|';

    // Interworking
    rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++;

    rval << '|';

    // Multi Band
    rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++; // id
    rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++; // channel

    rval << '|';

    // VHT Capabilities
    for (int i = 0; i < VHT_CAPABILITIES_LEN; i++)
        rval << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)*src++;

    rval << '|';

    rval << "\n\t" << "SSID List: ";
    rval << "\n\t\t" << "length: " << ssid_list.size();

    for (int i = 0; i < ssid_list.size(); i++) {
        rval << "\n\t\t" << "ssid[" + std::to_string(i) + "]: " << ssid_list[i];
    }

    return rval.str();
}
