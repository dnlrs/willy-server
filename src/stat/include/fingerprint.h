#ifndef FINGERPRINT_H_INCLUDED
#define FINGERPRINT_H_INCLUDED
#pragma once

#include "packet_ieee80211.h"

#include <vector>
#include <string>

#define FINGERPRINT_LEN (4+8+HT_CAPABILITIES_LEN+EXT_CAPABILITIES_LEN+1+2+VHT_CAPABILITIES_LEN)

typedef struct s_fingerprint {
    /* aggregated info, always present */
    uint32_t tag_presence;    // Bit flags indicating if element TAGs presence.
    uint64_t supported_rates; // Bit flags indicating supported rates.

    /* HT capabilities, optional */
    uint16_t    ht_capability_info;             // ($9.4.2.56.2)
    uint8_t     ht_ampduparam;
    uint8_t     ht_mcsset[HT_MCSSET_LEN];
    uint16_t    ht_extended_capabilities;
    uint32_t    ht_tbc;                         // Transmit Beamforming Capabilities
    uint8_t     ht_asel_capabilities;

    /* Extended capabilities */
    uint8_t     ext_extended_capabilities[EXT_CAPABILITIES_LEN];

    /* Interworking */
    uint8_t iw_interworking; // Access Network Options

    /* Multi Band */
    uint8_t multi_band_id;
    uint8_t multi_band_channel;

    /* VHT Capabilities */
    uint32_t vht_capabilities_info;
    uint8_t  vht_supported_mcs_nss_set[VHT_MCS_NSS_SET_LEN];

    /* ssid list */
    std::vector<std::string> ssid_list;

} fingerprint;


#endif // !FINGERPRINT_H_INCLUDED