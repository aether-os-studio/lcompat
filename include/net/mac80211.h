#pragma once

#include <asm/byteorder.h>
#include <linux/device.h>
#include <linux/atomic.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/leds.h>
#include <linux/timer.h>
#include <linux/completion.h>
#include <linux/netdevice.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <net/cfg80211.h>

#define IEEE80211_NUM_ACS 4
#define IEEE80211_NUM_TIDS 16
#define IEEE80211_MAX_CHAINS 4
#define IEEE80211_LINK_UNSPECIFIED 255
#define IEEE80211_MLD_MAX_NUM_LINKS 16
#define IEEE80211_HT_MCS_MASK_LEN 10
#define IEEE80211_MAX_MPDU_LEN_VHT_11454 11454
#define IEEE80211_MAX_DATA_LEN 2304
#define FCS_LEN 4
#define IEEE80211_FCTL_FTYPE 0x000c
#define IEEE80211_FCTL_STYPE 0x00f0
#define IEEE80211_FCTL_TODS 0x0100
#define IEEE80211_FCTL_FROMDS 0x0200
#define IEEE80211_FCTL_ORDER 0x8000
#define IEEE80211_FTYPE_MGMT 0x0000
#define IEEE80211_FTYPE_CTL 0x0004
#define IEEE80211_FTYPE_DATA 0x0008
#define IEEE80211_STYPE_ACTION 0x00d0
#define IEEE80211_STYPE_QOS_DATA 0x0080
#define IEEE80211_HT_CTL_LEN 4
#define IEEE80211_QOS_CTL_LEN 2
#define IEEE80211_MIN_ACTION_SIZE 24
#define IEEE80211_BAR_CTRL_TID_INFO_MASK GENMASK(15, 12)
#define WLAN_CATEGORY_BACK 3
#define WLAN_ACTION_ADDBA_REQ 0

enum {
    IEEE80211_AC_VO = 0,
    IEEE80211_AC_VI = 1,
    IEEE80211_AC_BE = 2,
    IEEE80211_AC_BK = 3,
};

enum ieee80211_ac_numbers {
    IEEE80211_AC_NUM_VO = IEEE80211_AC_VO,
    IEEE80211_AC_NUM_VI = IEEE80211_AC_VI,
    IEEE80211_AC_NUM_BE = IEEE80211_AC_BE,
    IEEE80211_AC_NUM_BK = IEEE80211_AC_BK,
};

#define IEEE80211_CHAN_NO_HT40MINUS BIT(0)
#define IEEE80211_CHAN_RADAR BIT(1)
#define IEEE80211_CHAN_NO_IR BIT(2)
#define IEEE80211_CHAN_NO_80MHZ BIT(3)
#define IEEE80211_CHAN_DISABLED BIT(4)
#define IEEE80211_IFACE_ITER_NORMAL 0
#define IEEE80211_IFACE_ITER_RESUME_ALL BIT(0)
#define NUM_NL80211_BANDS IEEE80211_NUM_BANDS

#define IEEE80211_CONF_IDLE BIT(0)
#define IEEE80211_CONF_CHANGE_IDLE BIT(1)
#define IEEE80211_CONF_CHANGE_CHANNEL BIT(2)
#define IEEE80211_CONF_OFFCHANNEL BIT(3)
#define IEEE80211_CONF_MONITOR BIT(4)
#define IEEE80211_CONF_CHANGE_POWER BIT(5)
#define IEEE80211_CONF_CHANGE_MONITOR BIT(6)

#define IEEE80211_VIF_BEACON_FILTER BIT(0)
#define IEEE80211_VIF_SUPPORTS_CQM_RSSI BIT(1)
#define IEEE80211_RATE_SHORT_PREAMBLE BIT(0)

#define FIF_ALLMULTI BIT(0)
#define FIF_OTHER_BSS BIT(1)
#define FIF_FCSFAIL BIT(2)
#define FIF_BCN_PRBRESP_PROMISC BIT(3)
#define FIF_PLCPFAIL BIT(4)
#define FIF_CONTROL BIT(5)
#define FIF_PSPOLL BIT(6)

#define BSS_CHANGED_ASSOC BIT(0)
#define BSS_CHANGED_BSSID BIT(1)
#define BSS_CHANGED_BEACON_INT BIT(2)
#define BSS_CHANGED_BEACON BIT(3)
#define BSS_CHANGED_BEACON_ENABLED BIT(4)
#define BSS_CHANGED_CQM BIT(5)
#define BSS_CHANGED_MU_GROUPS BIT(6)
#define BSS_CHANGED_ERP_SLOT BIT(7)
#define BSS_CHANGED_PS BIT(8)
#define BSS_CHANGED_HT BIT(9)
#define BSS_CHANGED_ERP_CTS_PROT BIT(10)
#define BSS_CHANGED_ERP_PREAMBLE BIT(11)
#define BSS_CHANGED_UNSOL_BCAST_PROBE_RESP BIT(12)
#define BSS_CHANGED_FILS_DISCOVERY BIT(13)
#define BSS_CHANGED_QOS BIT(14)
#define BSS_CHANGED_ARP_FILTER BIT(15)
#define BSS_CHANGED_MCAST_RATE BIT(16)
#define BSS_CHANGED_BASIC_RATES BIT(17)

#define WLAN_CIPHER_SUITE_WEP40 0x0001
#define WLAN_CIPHER_SUITE_WEP104 0x0002
#define WLAN_CIPHER_SUITE_TKIP 0x0003
#define WLAN_CIPHER_SUITE_CCMP 0x0004
#define WLAN_CIPHER_SUITE_AES_CMAC 0x0005
#define WLAN_CIPHER_SUITE_BIP_CMAC_256 0x0006
#define WLAN_CIPHER_SUITE_BIP_GMAC_128 0x0007
#define WLAN_CIPHER_SUITE_BIP_GMAC_256 0x0008
#define WLAN_CIPHER_SUITE_CCMP_256 0x0009
#define WLAN_CIPHER_SUITE_GCMP 0x000a
#define WLAN_CIPHER_SUITE_GCMP_256 0x000b
#define WLAN_CIPHER_SUITE_SMS4 0x000c

#define IEEE80211_KEY_FLAG_GENERATE_MMIC BIT(4)
#define IEEE80211_KEY_FLAG_SW_MGMT_TX BIT(5)
#define IEEE80211_KEY_FLAG_PAIRWISE BIT(6)
#define IEEE80211_KEY_FLAG_GENERATE_IV BIT(7)
#define IEEE80211_KEY_FLAG_RX_MGMT BIT(8)
#define IEEE80211_KEY_FLAG_GENERATE_MMIE BIT(9)

#define IEEE80211_HT_CAP_SGI_20 BIT(0)
#define IEEE80211_HT_CAP_MAX_AMSDU BIT(1)
#define IEEE80211_HT_CAP_RX_STBC_SHIFT 8
#define IEEE80211_HT_CAP_RX_STBC BIT(2)
#define IEEE80211_HT_CAP_LDPC_CODING BIT(3)
#define IEEE80211_HT_CAP_TX_STBC BIT(4)
#define IEEE80211_HT_CAP_SUP_WIDTH_20_40 BIT(5)
#define IEEE80211_HT_CAP_DSSSCCK40 BIT(6)
#define IEEE80211_HT_CAP_SGI_40 BIT(7)
#define IEEE80211_HT_CAP_GRN_FLD BIT(8)
#define IEEE80211_HT_MAX_AMPDU_64K 3
#define IEEE80211_HT_MCS_TX_DEFINED BIT(0)
#define IEEE80211_HT_OP_MODE_PROTECTION 0x0003
#define IEEE80211_HT_OP_MODE_PROTECTION_NONMEMBER 1
#define IEEE80211_HT_OP_MODE_PROTECTION_20MHZ 2
#define IEEE80211_HT_OP_MODE_PROTECTION_NONHT_MIXED 3
#define IEEE80211_HT_OP_MODE_NON_GF_STA_PRSNT BIT(2)
#define IEEE80211_HT_AMPDU_PARM_FACTOR GENMASK(1, 0)
#define IEEE80211_HT_AMPDU_PARM_DENSITY GENMASK(4, 2)
#define IEEE80211_HT_MPDU_DENSITY_NONE 0
#define IEEE80211_HT_MPDU_DENSITY_025 1
#define IEEE80211_HT_MPDU_DENSITY_05 2
#define IEEE80211_HT_MPDU_DENSITY_1 3
#define IEEE80211_HT_MPDU_DENSITY_2 4
#define IEEE80211_HT_MPDU_DENSITY_4 5
#define IEEE80211_HT_MPDU_DENSITY_8 6
#define IEEE80211_HT_MPDU_DENSITY_16 7

#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 BIT(0)
#define IEEE80211_VHT_CAP_MAX_MPDU_MASK GENMASK(1, 0)
#define IEEE80211_VHT_CAP_SHORT_GI_80 BIT(1)
#define IEEE80211_VHT_CAP_RXSTBC_1 BIT(2)
#define IEEE80211_VHT_CAP_RXSTBC_MASK IEEE80211_VHT_CAP_RXSTBC_1
#define IEEE80211_VHT_CAP_HTC_VHT BIT(3)
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK GENMASK(5, 4)
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT 4
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ BIT(5)
#define IEEE80211_VHT_CAP_TXSTBC BIT(6)
#define IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE BIT(7)
#define IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE BIT(8)
#define IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE BIT(9)
#define IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE BIT(10)
#define IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT 16
#define IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK GENMASK(18, 16)
#define IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT 9
#define IEEE80211_VHT_CAP_RXLDPC BIT(13)
#define IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN BIT(14)
#define IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN BIT(15)

#define IEEE80211_VHT_MCS_SUPPORT_0_9 0
#define IEEE80211_VHT_MCS_SUPPORT_0_8 1
#define IEEE80211_VHT_MCS_SUPPORT_0_7 2
#define IEEE80211_VHT_MCS_NOT_SUPPORTED 3
#define IEEE80211_VHT_MAX_AMPDU_8K 0
#define IEEE80211_VHT_MAX_AMPDU_16K 1
#define IEEE80211_VHT_MAX_AMPDU_32K 2
#define IEEE80211_VHT_MAX_AMPDU_64K 3
#define IEEE80211_VHT_MAX_AMPDU_128K 4
#define IEEE80211_VHT_MAX_AMPDU_256K 5
#define IEEE80211_VHT_MAX_AMPDU_512K 6
#define IEEE80211_VHT_MAX_AMPDU_1024K 7
#define IEEE80211_VHT_EXT_NSS_BW_CAPABLE BIT(0)
#define IEEE80211_OPMODE_NOTIF_RX_NSS_SHIFT 4

#define IEEE80211_STA_RX_BW_20 0
#define IEEE80211_STA_RX_BW_40 1
#define IEEE80211_STA_RX_BW_80 2
#define IEEE80211_STA_RX_BW_160 3
#define IEEE80211_RC_BW_CHANGED BIT(0)

#define IEEE80211_SMPS_STATIC 1
#define IEEE80211_SMPS_OFF 2
#define IEEE80211_SMPS_DYNAMIC 3

#define SIGNAL_DBM 0
#define RX_INCLUDES_FCS 1
#define AMPDU_AGGREGATION 2
#define MFP_CAPABLE 3
#define REPORTS_TX_ACK_STATUS 4
#define IEEE80211_HW_CONNECTION_MONITOR CONNECTION_MONITOR
#define SUPPORTS_PS 5
#define SUPPORTS_DYNAMIC_PS 6
#define SUPPORT_FAST_XMIT 7
#define SUPPORTS_AMSDU_IN_AMPDU 8
#define HAS_RATE_CONTROL 9
#define TX_AMSDU 10
#define SINGLE_SCAN_ON_ALL_BANDS 11
#define SUPPORTS_RC_TABLE 12
#define SUPPORTS_CLONED_SKBS 13
#define SUPPORTS_REORDERING_BUFFER 14
#define SPECTRUM_MGMT 15
#define TX_FRAG_LIST 16
#define AP_LINK_PS 17
#define SUPPORTS_VHT_EXT_NSS_BW 18
#define SUPPORTS_HT_CCK_RATES 19
#define HOST_BROADCAST_PS_BUFFERING 20
#define NEEDS_UNIQUE_STA_ADDR 21
#define SUPPORTS_MULTI_BSSID 22
#define SUPPORTS_TX_ENCAP_OFFLOAD 23
#define SUPPORTS_RX_DECAP_OFFLOAD 24
#define WANT_MONITOR_VIF 25
#define CONNECTION_MONITOR 26
#define SUPPORTS_ONLY_HE_MULTI_BSSID 27
#define CHANCTX_STA_CSA 28

#define IEEE80211_WMM_IE_STA_QOSINFO_SP_ALL 3
#define IEEE80211_WMM_IE_STA_QOSINFO_AC_VO BIT(0)
#define IEEE80211_WMM_IE_STA_QOSINFO_AC_VI BIT(1)
#define IEEE80211_WMM_IE_STA_QOSINFO_AC_BE BIT(2)
#define IEEE80211_WMM_IE_STA_QOSINFO_AC_BK BIT(3)

#define IEEE80211_MAX_MPDU_LEN_VHT_7991 7991
#define IEEE80211_VHT_CAP_SHORT_GI_160 BIT(16)

#define IEEE80211_HE_MAC_CAP0_HTC_HE BIT(0)
#define IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_MASK GENMASK(1, 0)
#define IEEE80211_HE_MAC_CAP2_BSR BIT(0)
#define IEEE80211_HE_MAC_CAP3_OMI_CONTROL BIT(0)
#define IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_MASK GENMASK(4, 3)
#define IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_EXT_3 BIT(5)
#define IEEE80211_HE_MAC_CAP4_AMSDU_IN_AMPDU BIT(0)
#define IEEE80211_HE_MAC_CAP4_BQR BIT(1)
#define IEEE80211_HE_MAC_CAP5_OM_CTRL_UL_MU_DATA_DIS_RX BIT(0)

#define IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G BIT(0)
#define IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G BIT(1)
#define IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G BIT(2)
#define IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_RU_MAPPING_IN_2G BIT(0)
#define IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_RU_MAPPING_IN_5G BIT(1)
#define IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G BIT(2)
#define IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_MASK GENMASK(3, 0)
#define IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_16US BIT(2)
#define IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD BIT(0)
#define IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US BIT(1)
#define IEEE80211_HE_PHY_CAP1_DEVICE_CLASS_A BIT(2)
#define IEEE80211_HE_PHY_CAP1_PREAMBLE_PUNC_RX_MASK GENMASK(4, 3)
#define IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US BIT(0)
#define IEEE80211_HE_PHY_CAP2_STBC_TX_UNDER_80MHZ BIT(1)
#define IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ BIT(2)
#define IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO BIT(3)
#define IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO BIT(4)
#define IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_MASK GENMASK(1, 0)
#define IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_QPSK 1
#define IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_2 BIT(2)
#define IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_MASK GENMASK(4, 3)
#define IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_QPSK BIT(3)
#define IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_2 BIT(5)
#define IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE BIT(0)
#define IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4 BIT(1)
#define IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_ABOVE_80MHZ_4 BIT(2)
#define IEEE80211_HE_PHY_CAP5_NG16_SU_FEEDBACK BIT(0)
#define IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK BIT(1)
#define IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU BIT(1)
#define IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU BIT(2)
#define IEEE80211_HE_PHY_CAP6_TRIG_CQI_FB BIT(3)
#define IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE BIT(0)
#define IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT BIT(4)
#define IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI BIT(0)
#define IEEE80211_HE_PHY_CAP7_STBC_TX_ABOVE_80MHZ BIT(1)
#define IEEE80211_HE_PHY_CAP7_STBC_RX_ABOVE_80MHZ BIT(2)
#define IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_SUPP BIT(3)
#define IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G BIT(4)
#define IEEE80211_HE_PHY_CAP8_20MHZ_IN_160MHZ_HE_PPDU BIT(5)
#define IEEE80211_HE_PHY_CAP8_80MHZ_IN_160MHZ_HE_PPDU BIT(6)
#define IEEE80211_HE_PHY_CAP8_HE_ER_SU_PPDU_4XLTF_AND_08_US_GI BIT(0)
#define IEEE80211_HE_PHY_CAP8_HE_ER_SU_1XLTF_AND_08_US_GI BIT(1)
#define IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_MASK GENMASK(3, 2)
#define IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_484 BIT(3)
#define IEEE80211_HE_PHY_CAP9_NON_TRIGGERED_CQI_FEEDBACK BIT(0)
#define IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU BIT(1)
#define IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU BIT(2)
#define IEEE80211_HE_PHY_CAP9_LONGER_THAN_16_SIGB_OFDM_SYM BIT(3)
#define IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB BIT(4)
#define IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB BIT(5)
#define IEEE80211_HE_PHY_CAP9_NOMINAL_PKT_PADDING_MASK GENMASK(7, 6)
#define IEEE80211_HE_PHY_CAP9_NOMINAL_PKT_PADDING_16US 2

#define IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP GENMASK(2, 0)
#define IEEE80211_HE_6GHZ_CAP_MIN_MPDU_START GENMASK(5, 3)
#define IEEE80211_HE_6GHZ_CAP_MAX_MPDU_LEN GENMASK(7, 6)
#define IEEE80211_HE_6GHZ_CAP_TX_ANTPAT_CONS BIT(8)
#define IEEE80211_HE_6GHZ_CAP_RX_ANTPAT_CONS BIT(9)
#define IEEE80211_MAX_AMPDU_BUF_HE 256
#define IEEE80211_MAX_AMPDU_BUF_EHT 256
#define IEEE80211_PPE_THRES_NSS_MASK GENMASK(2, 0)
#define IEEE80211_PPE_THRES_RU_INDEX_BITMASK_MASK GENMASK(6, 3)
#define IEEE80211_PPE_THRES_INFO_PPET_SIZE 6
#define IEEE80211_BSS_ARP_ADDR_LIST_LEN 4
#define IEEE80211_HE_MCS_SUPPORT_0_11 0
#define IEEE80211_HE_MCS_NOT_SUPPORTED 3

struct ieee80211_channel {
    int center_freq;
    int hw_value;
    enum nl80211_band band;
    u32 flags;
    int max_power;
    int max_reg_power;
    int orig_mpwr;
    enum nl80211_dfs_state dfs_state;
};

static inline bool
cfg80211_chandef_usable(struct wiphy *wiphy,
                        const struct cfg80211_chan_def *chandef,
                        u32 prohibited_flags) {
    (void)wiphy;
    return cfg80211_chandef_valid(chandef) &&
           !(chandef->chan->flags & prohibited_flags);
}

struct ieee80211_rate {
    int bitrate;
    int hw_value;
    int hw_value_short;
    u32 flags;
};

struct ieee80211_tpt_blink {
    int throughput;
    int blink_time;
};

struct ieee80211_sta_ht_cap {
    bool ht_supported;
    u16 cap;
    u8 ampdu_factor;
    u8 ampdu_density;
    struct {
        u8 rx_mask[10];
        __le16 rx_highest;
        u8 tx_params;
    } mcs;
};

struct ieee80211_sta_vht_cap {
    bool vht_supported;
    u32 cap;
    struct {
        __le16 rx_mcs_map;
        __le16 tx_mcs_map;
        __le16 rx_highest;
        __le16 tx_highest;
    } vht_mcs;
};

struct ieee80211_he_cap_elem {
    u8 mac_cap_info[6];
    u8 phy_cap_info[11];
};

struct ieee80211_he_mcs_nss_supp {
    __le16 rx_mcs_80;
    __le16 tx_mcs_80;
    __le16 rx_mcs_160;
    __le16 tx_mcs_160;
    __le16 rx_mcs_80p80;
    __le16 tx_mcs_80p80;
};

struct ieee80211_sta_he_cap {
    bool has_he;
    struct ieee80211_he_cap_elem he_cap_elem;
    struct ieee80211_he_mcs_nss_supp he_mcs_nss_supp;
    u8 ppe_thres[32];
};

struct ieee80211_sta_eht_cap {
    bool has_eht;
};

struct ieee80211_he_6ghz_capa {
    __le16 capa;
};

struct ieee80211_sta_agg {
    u16 max_amsdu_len;
    u16 max_rc_amsdu_len;
};

struct ieee80211_supported_band {
    enum nl80211_band band;
    struct ieee80211_channel *channels;
    int n_channels;
    struct ieee80211_rate *bitrates;
    int n_bitrates;
    struct ieee80211_sta_ht_cap ht_cap;
    struct ieee80211_sta_vht_cap vht_cap;
    struct ieee80211_sta_he_cap he_cap;
    struct ieee80211_sta_eht_cap eht_cap;
    const struct ieee80211_sband_iftype_data *iftype_data;
    int n_iftype_data;
};

struct ieee80211_iface_limit {
    u16 max;
    u16 types;
};

struct ieee80211_iface_combination {
    const struct ieee80211_iface_limit *limits;
    int n_limits;
    int max_interfaces;
    int num_different_channels;
    bool beacon_int_infra_match;
    u32 radar_detect_widths;
};

struct ieee80211_tx_queue_params {
    u16 txop;
    u16 cw_min;
    u16 cw_max;
    u8 aifs;
    bool mu_edca;
    struct ieee80211_he_mu_edca_param_ac_rec {
        u8 aifsn;
        u8 ecw_min_max;
        u8 mu_edca_timer;
    } mu_edca_param_rec;
};

struct ieee80211_vif;

struct ieee80211_conf {
    u32 flags;
    bool radar_enabled;
    int power_level;
    struct cfg80211_chan_def chandef;
};

struct ieee80211_bss_conf {
    bool assoc;
    u16 beacon_int;
    u16 aid;
    u8 link_id;
    u8 bssid[ETH_ALEN];
    u8 addr[ETH_ALEN];
    u8 transmitter_bssid[ETH_ALEN];
    u8 dtim_period;
    bool enable_beacon;
    bool use_short_slot;
    bool use_cts_prot;
    bool use_short_preamble;
    bool csa_active;
    bool nontransmitted;
    int power_type;
    u32 basic_rates;
    struct cfg80211_bitrate_mask beacon_tx_rate;
    int mcast_rate[IEEE80211_NUM_BANDS];
    bool qos;
    bool he_support;
    bool eht_support;
    u8 htc_trig_based_pkt_ext;
    u16 frame_time_rts_th;
    struct {
        bool enabled;
        u8 color;
    } he_bss_color;
    u8 bssid_indicator;
    u8 bssid_index;
    struct {
        u8 oppps_ctwindow;
    } p2p_noa_attr;
    s32 cqm_rssi_thold;
    u32 cqm_rssi_hyst;
    u16 ht_operation_mode;
    struct {
        struct cfg80211_chan_def oper;
    } chanreq;
    struct ieee80211_vif *vif;
    struct {
        u8 membership[8];
        u8 position[16];
    } mu_group;
};

struct ieee80211_vif_cfg {
    bool assoc;
    bool ps;
    u16 aid;
    u8 arp_addr_cnt;
    __be32 arp_addr_list[IEEE80211_BSS_ARP_ADDR_LIST_LEN];
};

struct ieee80211_vif {
    enum nl80211_iftype type;
    u32 driver_flags;
    struct ieee80211_bss_conf bss_conf;
    struct ieee80211_vif_cfg cfg;
    u8 addr[6];
    struct ieee80211_txq *txq;
    bool p2p;
    u16 valid_links;
    u16 active_links;
    struct ieee80211_bss_conf *link_conf[IEEE80211_MLD_MAX_NUM_LINKS];
    char drv_priv[];
};

struct ieee80211_hdr {
    __le16 frame_control;
    __le16 duration_id;
    u8 addr1[6];
    u8 addr2[6];
    u8 addr3[6];
    __le16 seq_ctrl;
    u8 addr4[6];
};

struct ieee80211_hdr_3addr {
    __le16 frame_control;
    __le16 duration_id;
    u8 addr1[6];
    u8 addr2[6];
    u8 addr3[6];
    __le16 seq_ctrl;
};

struct ieee80211_mgmt {
    __le16 frame_control;
    __le16 duration;
    u8 da[6];
    u8 sa[6];
    u8 bssid[6];
    __le16 seq_ctrl;
    union {
        struct {
            __le16 capab_info;
            __le16 listen_interval;
            u8 variable[];
        } assoc_req;
        struct {
            __le16 capab_info;
            __le16 status_code;
            __le16 aid;
            u8 variable[];
        } assoc_resp;
        struct {
            __le16 auth_alg;
            __le16 auth_transaction;
            __le16 status_code;
            u8 variable[];
        } auth;
        struct {
            __le16 reason_code;
            u8 variable[];
        } disassoc;
        struct {
            __le16 reason_code;
            u8 variable[];
        } deauth;
        struct {
            u8 category;
            union {
                struct {
                    u8 action_code;
                    __le16 capab;
                    __le16 timeout;
                    __le16 start_seq_num;
                } addba_req;
            } u;
        } action;
        struct {
            __le64 timestamp;
            __le16 beacon_int;
            __le16 capab_info;
            u8 variable[];
        } beacon;
        struct {
            __le64 timestamp;
            __le16 beacon_int;
            __le16 capab_info;
            u8 variable[];
        } probe_resp;
    } u;
} __packed;

struct ieee80211_bar {
    __le16 frame_control;
    __le16 duration;
    u8 ra[6];
    u8 ta[6];
    __le16 control;
    __le16 start_seq_num;
} __packed;

#define IEEE80211_SCTL_SEQ 0xfff0
#define IEEE80211_FCTL_PM 0x1000
#define IEEE80211_FCTL_MOREDATA 0x2000

struct ieee80211_key_conf {
    u32 flags;
    u8 keyidx;
    u8 hw_key_idx;
    u32 cipher;
    u8 keylen;
    u8 key[32];
    atomic64_t tx_pn;
};

struct ieee80211_key_seq {
    struct {
        u8 iv32[4];
        u16 iv16;
    } tkip;
    struct {
        u8 pn[6];
    } ccmp;
    struct {
        u8 pn[6];
    } aes_cmac;
};

struct ieee80211_txq {
    int ac;
    struct ieee80211_sta *sta;
    struct ieee80211_vif *vif;
    u8 tid;
    char drv_priv[];
};

struct ieee80211_link_sta {
    u8 addr[ETH_ALEN];
    struct ieee80211_sta_ht_cap ht_cap;
    struct ieee80211_sta_vht_cap vht_cap;
    struct ieee80211_sta_he_cap he_cap;
    struct ieee80211_sta_eht_cap eht_cap;
    struct ieee80211_he_6ghz_capa he_6ghz_capa;
    struct ieee80211_sta_agg agg;
    u32 supp_rates[IEEE80211_NUM_BANDS];
    u8 bandwidth;
    u8 rx_nss;
    u8 smps_mode;
    struct ieee80211_sta *sta;
};

#define IEEE80211_TX_MAX_RATES 4

struct ieee80211_sta {
    u8 addr[6];
    struct ieee80211_txq *txq[IEEE80211_NUM_TIDS];
    bool tdls;
    u16 aid;
    bool wme;
    u8 uapsd_queues;
    u8 max_sp;
    u16 valid_links;
    u32 supp_rates[IEEE80211_NUM_BANDS];
    u8 bandwidth;
    int max_rc_amsdu_len;
    struct ieee80211_sta_ht_cap ht_cap;
    struct ieee80211_sta_vht_cap vht_cap;
    struct ieee80211_link_sta deflink;
    struct ieee80211_link_sta *link[IEEE80211_MLD_MAX_NUM_LINKS];
    struct ieee80211_sta_rates *rates;
    char drv_priv[];
};

struct ieee80211_sta_rate {
    s8 idx;
    u16 flags;
};

struct ieee80211_sta_rates {
    struct rcu_head rcu_head;
    struct ieee80211_sta_rate rate[IEEE80211_TX_MAX_RATES];
};

struct ieee80211_sband_iftype_data {
    u32 types_mask;
    struct ieee80211_sta_he_cap he_cap;
    struct ieee80211_sta_eht_cap eht_cap;
    struct ieee80211_he_6ghz_capa he_6ghz_capa;
};

struct ieee80211_chanctx_conf {
    void *drv_priv;
    bool radar_enabled;
    struct cfg80211_chan_def def;
};

#define IEEE80211_CHANCTX_CHANGE_WIDTH BIT(0)
#define IEEE80211_CHANCTX_CHANGE_RADAR BIT(1)
#define IEEE80211_CHANCTX_CHANGE_PUNCTURING BIT(2)

struct rate_info {
    u32 flags;
    u16 legacy;
    u8 mcs;
    u8 nss;
    u8 bw;
    u8 he_gi;
    u8 he_dcm;
    u8 he_ru_alloc;
};

#define RATE_INFO_FLAGS_VHT_MCS BIT(0)
#define RATE_INFO_FLAGS_MCS BIT(1)
#define RATE_INFO_FLAGS_SHORT_GI BIT(2)
#define RATE_INFO_FLAGS_HE_MCS BIT(3)
#define RATE_INFO_BW_20 0
#define RATE_INFO_BW_40 1
#define RATE_INFO_BW_80 2
#define RATE_INFO_BW_160 3
#define RATE_INFO_BW_HE_RU 4
#define NL80211_RATE_INFO_HE_GI_3_2 2
#define NL80211_RATE_INFO_HE_RU_ALLOC_26 0
#define NL80211_RATE_INFO_HE_RU_ALLOC_52 1
#define NL80211_RATE_INFO_HE_RU_ALLOC_106 2
#define NL80211_RATE_INFO_HE_RU_ALLOC_242 3
#define NL80211_RATE_INFO_HE_RU_ALLOC_484 4
#define NL80211_RATE_INFO_HE_RU_ALLOC_996 5
#define NL80211_RATE_INFO_HE_RU_ALLOC_2x996 6

struct ieee80211_low_level_stats {
    u32 dot11RTSSuccessCount;
    u32 dot11RTSFailureCount;
    u32 dot11FCSErrorCount;
    u32 dot11ACKFailureCount;
};

enum mac80211_rx_encoding {
    RX_ENC_LEGACY = 0,
    RX_ENC_HT = 1,
    RX_ENC_VHT = 2,
    RX_ENC_HE = 3,
    RX_ENC_EHT = 4,
};

#define RX_FLAG_FAILED_FCS_CRC BIT(0)
#define RX_FLAG_DECRYPTED BIT(1)
#define RX_FLAG_MACTIME_START BIT(2)
#define RX_FLAG_NO_SIGNAL_VAL BIT(3)
#define RX_FLAG_NO_PSDU BIT(4)
#define RX_FLAG_RADIOTAP_HE BIT(5)
#define RX_FLAG_RADIOTAP_HE_MU BIT(6)
#define RX_FLAG_8023 BIT(7)
#define RX_FLAG_IV_STRIPPED BIT(8)
#define RX_FLAG_ONLY_MONITOR BIT(9)
#define RX_FLAG_PN_VALIDATED BIT(10)
#define RX_FLAG_AMPDU_DETAILS BIT(11)
#define RX_FLAG_DUP_VALIDATED BIT(12)
#define RX_FLAG_MMIC_STRIPPED BIT(13)
#define RX_FLAG_MIC_STRIPPED BIT(14)
#define RX_FLAG_RADIOTAP_TLV_AT_END BIT(15)
#define RX_FLAG_MMIC_ERROR BIT(16)

#define RX_ENC_FLAG_SHORTPRE BIT(0)
#define RX_ENC_FLAG_HT_GF BIT(1)
#define RX_ENC_FLAG_LDPC BIT(2)
#define RX_ENC_FLAG_SHORT_GI BIT(3)
#define RX_ENC_FLAG_STBC_SHIFT 4
#define RX_ENC_FLAG_STBC_MASK GENMASK(5, 4)

static inline int cfg80211_calculate_bitrate(struct rate_info *rate) {
    if (!rate)
        return 0;
    if (rate->legacy)
        return rate->legacy;
    return 0;
}

struct ieee80211_rx_status {
    u32 flag;
    int freq;
    int band;
    int signal;
    int encoding;
    u8 rate_idx;
    u8 nss;
    u64 mactime;
    u64 device_timestamp;
    u64 boottime_ns;
    u8 bw;
    u8 chains;
    u8 link_id;
    u8 qos_ctl;
    u8 enc_flags;
    u32 ampdu_reference;
    bool link_valid;
    struct {
        u8 ru;
        u8 gi;
    } eht;
    u8 he_ru;
    u8 he_gi;
    u8 he_dcm;
    u8 iv[6];
    s8 chain_signal[4];
};

struct station_info {
    struct rate_info txrate;
    u64 filled;
    u32 tx_failed;
    u32 tx_retries;
    s8 ack_signal;
    s8 avg_ack_signal;
};

enum set_key_cmd {
    SET_KEY = 0,
    DISABLE_KEY = 1,
};

enum ieee80211_ampdu_mlme_action {
    IEEE80211_AMPDU_TX_START = 0,
    IEEE80211_AMPDU_TX_STOP_CONT = 1,
    IEEE80211_AMPDU_TX_STOP_FLUSH = 2,
    IEEE80211_AMPDU_TX_STOP_FLUSH_CONT = 3,
    IEEE80211_AMPDU_TX_OPERATIONAL = 4,
    IEEE80211_AMPDU_RX_START = 5,
    IEEE80211_AMPDU_RX_STOP = 6,
};

#define IEEE80211_AMPDU_TX_START_IMMEDIATE 1

struct ieee80211_ampdu_params {
    struct ieee80211_sta *sta;
    u16 tid;
    u16 ssn;
    u16 buf_size;
    bool amsdu;
    enum ieee80211_ampdu_mlme_action action;
};

enum ieee80211_reconfig_type {
    IEEE80211_RECONFIG_TYPE_RESTART = 0,
};

enum ieee80211_sta_state {
    IEEE80211_STA_NOTEXIST = 0,
    IEEE80211_STA_NONE = 1,
    IEEE80211_STA_AUTH = 2,
    IEEE80211_STA_ASSOC = 3,
    IEEE80211_STA_AUTHORIZED = 4,
};

enum ieee80211_frame_release_type {
    IEEE80211_FRAME_RELEASE_PSPOLL = 0,
};

enum ieee80211_chanctx_switch_mode {
    CHANCTX_SWMODE_REASSIGN_VIF = 0,
    CHANCTX_SWMODE_SWAP_CONTEXTS = 1,
};

enum ieee80211_roc_type {
    IEEE80211_ROC_TYPE_NORMAL = 0,
};

enum ieee80211_ap_reg_power {
    IEEE80211_REG_UNSET_AP = 0,
    IEEE80211_REG_LPI_AP = 1,
    IEEE80211_REG_SP_AP = 2,
    IEEE80211_REG_VLP_AP = 3,
};

struct ieee80211_channel_switch {
    struct cfg80211_chan_def chandef;
    u8 count;
};

struct ieee80211_tx_rate {
    s8 idx;
    u8 count;
    u16 flags;
};

#define IEEE80211_TX_RC_USE_RTS_CTS BIT(0)
#define IEEE80211_TX_RC_USE_CTS_PROTECT BIT(1)
#define IEEE80211_TX_RC_USE_SHORT_PREAMBLE BIT(2)
#define IEEE80211_TX_RC_MCS BIT(3)
#define IEEE80211_TX_RC_GREEN_FIELD BIT(4)
#define IEEE80211_TX_RC_40_MHZ_WIDTH BIT(5)
#define IEEE80211_TX_RC_DUP_DATA BIT(6)
#define IEEE80211_TX_RC_SHORT_GI BIT(7)
#define IEEE80211_TX_RC_VHT_MCS BIT(8)
#define IEEE80211_TX_RC_80_MHZ_WIDTH BIT(9)
#define IEEE80211_TX_RC_160_MHZ_WIDTH BIT(10)

static inline int
ieee80211_rate_get_vht_mcs(const struct ieee80211_tx_rate *r) {
    return r ? (r->idx & 0xf) : 0;
}

static inline int
ieee80211_rate_get_vht_nss(const struct ieee80211_tx_rate *r) {
    return r ? ((r->idx >> 4) & 0xf) : 0;
}

static inline void ieee80211_rate_set_vht(struct ieee80211_tx_rate *r, int mcs,
                                          int nss) {
    if (!r)
        return;
    r->idx = (s8)(((nss & 0xf) << 4) | (mcs & 0xf));
    r->flags |= IEEE80211_TX_RC_VHT_MCS;
}

struct ieee80211_rate_status {
    struct rate_info rate_idx;
    u8 count;
    s8 idx;
    u16 flags;
};

struct ieee80211_vif_chanctx_switch {
    struct ieee80211_vif *vif;
    struct ieee80211_bss_conf *link_conf;
    struct ieee80211_chanctx_conf *old_ctx;
    struct ieee80211_chanctx_conf *new_ctx;
};

#define IEEE80211_TX_INFO_DRIVER_DATA_SIZE 40
#define IEEE80211_TX_INFO_RATE_DRIVER_DATA_SIZE 24

struct ieee80211_tx_info {
    u32 flags;
    u32 band : 3, status_data_idr : 1, status_data : 13, hw_queue : 4,
        tx_time_est : 10;
    union {
        struct {
            struct ieee80211_tx_rate rates[IEEE80211_TX_MAX_RATES];
            s8 rts_cts_rate_idx;
            u8 use_rts : 1;
            u8 use_cts_prot : 1;
            u8 short_preamble : 1;
            u8 skip_table : 1;
            u8 antennas : 2;
            struct ieee80211_vif *vif;
            struct ieee80211_key_conf *hw_key;
            u32 flags;
        } control;
        struct {
            struct ieee80211_tx_rate rates[IEEE80211_TX_MAX_RATES];
            s32 ack_signal;
            u8 ampdu_ack_len;
            u8 ampdu_len;
            u8 antenna;
            u8 pad;
            u16 tx_time;
            u8 flags;
            u8 pad2;
            u8 status_driver_data[16];
        } status;
        struct {
            struct ieee80211_tx_rate driver_rates[IEEE80211_TX_MAX_RATES];
            u8 pad[4];
            void *rate_driver_data[IEEE80211_TX_INFO_RATE_DRIVER_DATA_SIZE /
                                   sizeof(void *)];
        };
        void *driver_data[IEEE80211_TX_INFO_DRIVER_DATA_SIZE / sizeof(void *)];
    };
};

#define IEEE80211_TX_CTRL_MLO_LINK GENMASK(11, 8)

struct ieee80211_tx_control {
    struct ieee80211_sta *sta;
};

struct ieee80211_prep_tx_info {
    u16 duration;
};

struct ieee80211_scan_ies {
    const u8 *ies[IEEE80211_NUM_BANDS];
    size_t len[IEEE80211_NUM_BANDS];
    const u8 *common_ies;
    size_t common_ie_len;
};

struct ieee80211_scan_request {
    struct cfg80211_scan_request req;
    struct ieee80211_scan_ies ies;
};

struct ieee80211_mutable_offsets {
    u16 tim_offset;
    u16 tim_length;
    u16 cntdwn_counter_offs[2];
    u16 mbssid_off;
    u16 ema_offset[2];
};

struct ieee80211_ops;

struct ieee80211_hw {
    struct wiphy *wiphy;
    struct device *dev;
    const struct ieee80211_ops *ops;
    struct ieee80211_conf conf;
    int queues;
    u32 flags;
    int extra_tx_headroom;
    int txq_data_size;
    int sta_data_size;
    int vif_data_size;
    int uapsd_max_sp_len;
    int max_tx_fragments;
    int max_rates;
    int max_report_rates;
    int max_rate_tries;
    int max_rx_aggregation_subframes;
    int max_tx_aggregation_subframes;
    u64 netdev_features;
    struct {
        int units_pos;
    } radiotap_timestamp;
    int chanctx_data_size;
    u8 perm_addr[ETH_ALEN];
    void *lcompat_runtime;
    void *priv;
};

#define IEEE80211_SN_TO_SEQ(ssn) ((ssn) << 4)

struct ieee80211_ops {
    void (*tx)(struct ieee80211_hw *, struct ieee80211_tx_control *,
               struct sk_buff *);
    void (*wake_tx_queue)(struct ieee80211_hw *, struct ieee80211_txq *);
    int (*start)(struct ieee80211_hw *);
    void (*stop)(struct ieee80211_hw *, bool);
    int (*config)(struct ieee80211_hw *, int, u32);
    int (*add_interface)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*remove_interface)(struct ieee80211_hw *, struct ieee80211_vif *);
    int (*change_interface)(struct ieee80211_hw *, struct ieee80211_vif *,
                            enum nl80211_iftype, bool);
    int (*add_chanctx)(struct ieee80211_hw *, struct ieee80211_chanctx_conf *);
    void (*remove_chanctx)(struct ieee80211_hw *,
                           struct ieee80211_chanctx_conf *);
    void (*change_chanctx)(struct ieee80211_hw *,
                           struct ieee80211_chanctx_conf *, u32);
    int (*assign_vif_chanctx)(struct ieee80211_hw *, struct ieee80211_vif *,
                              struct ieee80211_bss_conf *,
                              struct ieee80211_chanctx_conf *);
    void (*unassign_vif_chanctx)(struct ieee80211_hw *, struct ieee80211_vif *,
                                 struct ieee80211_bss_conf *,
                                 struct ieee80211_chanctx_conf *);
    int (*switch_vif_chanctx)(struct ieee80211_hw *,
                              struct ieee80211_vif_chanctx_switch *, int,
                              enum ieee80211_chanctx_switch_mode);
    void (*configure_filter)(struct ieee80211_hw *, unsigned int,
                             unsigned int *, u64);
    void (*bss_info_changed)(struct ieee80211_hw *, struct ieee80211_vif *,
                             struct ieee80211_bss_conf *, u64);
    int (*start_ap)(struct ieee80211_hw *, struct ieee80211_vif *,
                    struct ieee80211_bss_conf *);
    void (*stop_ap)(struct ieee80211_hw *, struct ieee80211_vif *,
                    struct ieee80211_bss_conf *);
    int (*conf_tx)(struct ieee80211_hw *, struct ieee80211_vif *, unsigned int,
                   u16, const struct ieee80211_tx_queue_params *);
    int (*sta_add)(struct ieee80211_hw *, struct ieee80211_vif *,
                   struct ieee80211_sta *);
    int (*sta_remove)(struct ieee80211_hw *, struct ieee80211_vif *,
                      struct ieee80211_sta *);
    int (*sta_state)(struct ieee80211_hw *, struct ieee80211_vif *,
                     struct ieee80211_sta *, enum ieee80211_sta_state,
                     enum ieee80211_sta_state);
    void (*sta_pre_rcu_remove)(struct ieee80211_hw *, struct ieee80211_vif *,
                               struct ieee80211_sta *);
    int (*set_tim)(struct ieee80211_hw *, struct ieee80211_sta *, bool);
    int (*set_key)(struct ieee80211_hw *, enum set_key_cmd,
                   struct ieee80211_vif *, struct ieee80211_sta *,
                   struct ieee80211_key_conf *);
    int (*ampdu_action)(struct ieee80211_hw *, struct ieee80211_vif *,
                        struct ieee80211_ampdu_params *);
    void (*sta_set_decap_offload)(struct ieee80211_hw *, struct ieee80211_vif *,
                                  struct ieee80211_sta *, bool);
    bool (*can_aggregate_in_amsdu)(struct ieee80211_hw *, struct sk_buff *,
                                   struct sk_buff *);
    void (*sw_scan_start)(struct ieee80211_hw *, struct ieee80211_vif *,
                          const u8 *);
    void (*sw_scan_complete)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*mgd_prepare_tx)(struct ieee80211_hw *, struct ieee80211_vif *,
                           struct ieee80211_prep_tx_info *);
    void (*mgd_complete_tx)(struct ieee80211_hw *, struct ieee80211_vif *,
                            struct ieee80211_prep_tx_info *);
    int (*remain_on_channel)(struct ieee80211_hw *, struct ieee80211_vif *,
                             struct ieee80211_channel *, int,
                             enum ieee80211_roc_type);
    int (*cancel_remain_on_channel)(struct ieee80211_hw *,
                                    struct ieee80211_vif *);
    int (*set_rts_threshold)(struct ieee80211_hw *, int, u32);
    u64 (*get_tsf)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*set_tsf)(struct ieee80211_hw *, struct ieee80211_vif *, u64);
    int (*get_stats)(struct ieee80211_hw *, struct ieee80211_low_level_stats *);
    void (*sta_statistics)(struct ieee80211_hw *, struct ieee80211_vif *,
                           struct ieee80211_sta *, struct station_info *);
    int (*get_et_sset_count)(struct ieee80211_hw *, struct ieee80211_vif *,
                             int);
    void (*get_et_strings)(struct ieee80211_hw *, struct ieee80211_vif *, u32,
                           u8 *);
    void (*get_et_stats)(struct ieee80211_hw *, struct ieee80211_vif *,
                         struct ethtool_stats *, u64 *);
    void (*flush)(struct ieee80211_hw *, struct ieee80211_vif *, u32, bool);
    int (*set_bitrate_mask)(struct ieee80211_hw *, struct ieee80211_vif *,
                            const struct cfg80211_bitrate_mask *);
    int (*set_antenna)(struct ieee80211_hw *, int, u32, u32);
    int (*get_antenna)(struct ieee80211_hw *, int, u32 *, u32 *);
    int (*get_txpower)(struct ieee80211_hw *, struct ieee80211_vif *,
                       unsigned int, int *);
    int (*get_survey)(struct ieee80211_hw *, int, struct survey_info *);
    void (*set_coverage_class)(struct ieee80211_hw *, int, s16);
    void (*release_buffered_frames)(struct ieee80211_hw *,
                                    struct ieee80211_sta *, u16, int,
                                    enum ieee80211_frame_release_type, bool);
    void (*reconfig_complete)(struct ieee80211_hw *,
                              enum ieee80211_reconfig_type);
    int (*hw_scan)(struct ieee80211_hw *, struct ieee80211_vif *,
                   struct ieee80211_scan_request *);
    void (*cancel_hw_scan)(struct ieee80211_hw *, struct ieee80211_vif *);
    int (*sched_scan_start)(struct ieee80211_hw *, struct ieee80211_vif *,
                            struct cfg80211_sched_scan_request *,
                            struct ieee80211_scan_ies *);
    int (*sched_scan_stop)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*sta_rc_update)(struct ieee80211_hw *, struct ieee80211_vif *,
                          struct ieee80211_sta *, u32);
    void (*link_sta_rc_update)(struct ieee80211_hw *, struct ieee80211_vif *,
                               struct ieee80211_link_sta *, u32);
    void (*sta_rate_tbl_update)(struct ieee80211_hw *, struct ieee80211_vif *,
                                struct ieee80211_sta *);
    int (*set_sar_specs)(struct ieee80211_hw *,
                         const struct cfg80211_sar_specs *);
    void (*rfkill_poll)(struct ieee80211_hw *);
    void (*channel_switch_beacon)(struct ieee80211_hw *, struct ieee80211_vif *,
                                  struct cfg80211_chan_def *);
    int (*pre_channel_switch)(struct ieee80211_hw *, struct ieee80211_vif *,
                              struct ieee80211_channel_switch *);
    void (*channel_switch)(struct ieee80211_hw *, struct ieee80211_vif *,
                           struct ieee80211_channel_switch *);
    void (*abort_channel_switch)(struct ieee80211_hw *, struct ieee80211_vif *,
                                 struct ieee80211_bss_conf *);
    void (*channel_switch_rx_beacon)(struct ieee80211_hw *,
                                     struct ieee80211_vif *,
                                     struct ieee80211_channel_switch *);
    void (*vif_cfg_changed)(struct ieee80211_hw *, struct ieee80211_vif *, u64);
    void (*link_info_changed)(struct ieee80211_hw *, struct ieee80211_vif *,
                              struct ieee80211_bss_conf *, u64);
    int (*change_vif_links)(
        struct ieee80211_hw *, struct ieee80211_vif *, u16, u16,
        struct ieee80211_bss_conf *old[IEEE80211_MLD_MAX_NUM_LINKS]);
    int (*change_sta_links)(struct ieee80211_hw *, struct ieee80211_vif *,
                            struct ieee80211_sta *, u16, u16);
};

struct ieee80211_tx_control;
struct ieee80211_tx_status {
    struct ieee80211_sta *sta;
    struct sk_buff *skb;
    struct ieee80211_tx_info *info;
    struct ieee80211_rate_status *rates;
    int n_rates;
    struct list_head *free_list;
};

struct ieee80211_radiotap_he {
    __le16 data1;
    __le16 data2;
    __le16 data3;
    __le16 data4;
    __le16 data5;
    __le16 data6;
};

struct ieee80211_radiotap_he_mu {
    __le16 flags1;
    __le16 flags2;
    u8 ru_ch1[4];
    u8 ru_ch2[4];
};

struct ieee80211_radiotap_tlv {
    __le16 type;
    __le16 len;
    u8 data[];
} __packed;

struct ieee80211_radiotap_eht {
    __le32 known;
    __le32 data[8];
    __le32 user_info[1];
};

struct ieee80211_radiotap_eht_usig {
    __le32 common;
};

#define IEEE80211_RADIOTAP_EHT 35
#define IEEE80211_RADIOTAP_EHT_USIG 36
#define IEEE80211_RADIOTAP_TIMESTAMP_UNIT_US 1
#define IEEE80211_RADIOTAP_EHT_KNOWN_SPATIAL_REUSE BIT(0)
#define IEEE80211_RADIOTAP_EHT_KNOWN_GI BIT(1)
#define IEEE80211_RADIOTAP_EHT_KNOWN_EHT_LTF BIT(2)
#define IEEE80211_RADIOTAP_EHT_KNOWN_LDPC_EXTRA_SYM_OM BIT(3)
#define IEEE80211_RADIOTAP_EHT_KNOWN_PE_DISAMBIGUITY_OM BIT(4)
#define IEEE80211_RADIOTAP_EHT_KNOWN_NSS_S BIT(5)
#define IEEE80211_RADIOTAP_EHT_DATA0_SPATIAL_REUSE GENMASK(3, 0)
#define IEEE80211_RADIOTAP_EHT_DATA0_GI GENMASK(5, 4)
#define IEEE80211_RADIOTAP_EHT_DATA0_LTF GENMASK(7, 6)
#define IEEE80211_RADIOTAP_EHT_DATA0_PE_DISAMBIGUITY_OM BIT(8)
#define IEEE80211_RADIOTAP_EHT_DATA0_LDPC_EXTRA_SYM_OM BIT(9)
#define IEEE80211_RADIOTAP_EHT_DATA7_NSS_S GENMASK(3, 0)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_MCS_KNOWN BIT(0)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_CODING_KNOWN BIT(1)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_NSS_KNOWN_O BIT(2)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_BEAMFORMING_KNOWN_O BIT(3)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_DATA_FOR_USER BIT(4)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_MCS GENMASK(7, 5)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_NSS_O GENMASK(11, 8)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_BEAMFORMING_O BIT(12)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_CODING BIT(13)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_STA_ID_KNOWN BIT(14)
#define IEEE80211_RADIOTAP_EHT_USER_INFO_STA_ID GENMASK(26, 16)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_PHY_VER_KNOWN BIT(0)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_BW_KNOWN BIT(1)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_UL_DL_KNOWN BIT(2)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_BSS_COLOR_KNOWN BIT(3)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_TXOP_KNOWN BIT(4)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_PHY_VER GENMASK(7, 5)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_BW GENMASK(10, 8)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_UL_DL BIT(11)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_BSS_COLOR GENMASK(17, 12)
#define IEEE80211_RADIOTAP_EHT_USIG_COMMON_TXOP GENMASK(24, 18)

#define IEEE80211_RADIOTAP_HE_DATA1_BW_RU_ALLOC_KNOWN BIT(0)
#define IEEE80211_RADIOTAP_HE_DATA1_DATA_MCS_KNOWN BIT(1)
#define IEEE80211_RADIOTAP_HE_DATA1_DATA_DCM_KNOWN BIT(2)
#define IEEE80211_RADIOTAP_HE_DATA1_STBC_KNOWN BIT(3)
#define IEEE80211_RADIOTAP_HE_DATA1_CODING_KNOWN BIT(4)
#define IEEE80211_RADIOTAP_HE_DATA1_LDPC_XSYMSEG_KNOWN BIT(5)
#define IEEE80211_RADIOTAP_HE_DATA1_DOPPLER_KNOWN BIT(6)
#define IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE_KNOWN BIT(7)
#define IEEE80211_RADIOTAP_HE_DATA1_BSS_COLOR_KNOWN BIT(8)
#define IEEE80211_RADIOTAP_HE_DATA1_FORMAT_SU BIT(9)
#define IEEE80211_RADIOTAP_HE_DATA1_FORMAT_EXT_SU BIT(10)
#define IEEE80211_RADIOTAP_HE_DATA1_FORMAT_MU BIT(11)
#define IEEE80211_RADIOTAP_HE_DATA1_FORMAT_TRIG BIT(12)
#define IEEE80211_RADIOTAP_HE_DATA1_UL_DL_KNOWN BIT(13)
#define IEEE80211_RADIOTAP_HE_DATA1_BEAM_CHANGE_KNOWN BIT(14)
#define IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE2_KNOWN BIT(15)
#define IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE3_KNOWN BIT(0)
#define IEEE80211_RADIOTAP_HE_DATA1_SPTL_REUSE4_KNOWN BIT(1)
#define IEEE80211_RADIOTAP_HE_DATA2_RU_OFFSET_KNOWN BIT(0)
#define IEEE80211_RADIOTAP_HE_DATA2_RU_OFFSET GENMASK(15, 8)
#define IEEE80211_RADIOTAP_HE_DATA2_GI_KNOWN BIT(1)
#define IEEE80211_RADIOTAP_HE_DATA2_TXBF_KNOWN BIT(2)
#define IEEE80211_RADIOTAP_HE_DATA2_PE_DISAMBIG_KNOWN BIT(3)
#define IEEE80211_RADIOTAP_HE_DATA2_TXOP_KNOWN BIT(4)
#define IEEE80211_RADIOTAP_HE_DATA3_BSS_COLOR GENMASK(5, 0)
#define IEEE80211_RADIOTAP_HE_DATA3_LDPC_XSYMSEG BIT(6)
#define IEEE80211_RADIOTAP_HE_DATA3_BEAM_CHANGE BIT(7)
#define IEEE80211_RADIOTAP_HE_DATA3_UL_DL BIT(8)
#define IEEE80211_RADIOTAP_HE_DATA4_SU_MU_SPTL_REUSE GENMASK(3, 0)
#define IEEE80211_RADIOTAP_HE_DATA4_MU_STA_ID GENMASK(14, 4)
#define IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE1 GENMASK(3, 0)
#define IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE2 GENMASK(7, 4)
#define IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE3 GENMASK(11, 8)
#define IEEE80211_RADIOTAP_HE_DATA4_TB_SPTL_REUSE4 GENMASK(15, 12)
#define IEEE80211_RADIOTAP_HE_DATA5_PE_DISAMBIG BIT(0)
#define IEEE80211_RADIOTAP_HE_DATA5_LTF_SIZE GENMASK(2, 1)
#define IEEE80211_RADIOTAP_HE_DATA5_TXBF BIT(3)
#define IEEE80211_RADIOTAP_HE_DATA6_TXOP GENMASK(7, 0)
#define IEEE80211_RADIOTAP_HE_DATA6_DOPPLER BIT(8)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_MCS_KNOWN BIT(0)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_DCM_KNOWN BIT(1)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH1_RU_KNOWN BIT(2)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_SYMS_USERS_KNOWN BIT(3)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_COMP_KNOWN BIT(4)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_CH2_RU_KNOWN BIT(5)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW_KNOWN BIT(0)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS2_PUNC_FROM_SIG_A_BW_KNOWN BIT(1)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_MCS GENMASK(3, 0)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS1_SIG_B_DCM BIT(4)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS2_BW_FROM_SIG_A_BW GENMASK(2, 0)
#define IEEE80211_RADIOTAP_HE_MU_FLAGS2_SIG_B_SYMS_USERS GENMASK(15, 8)

static inline struct ieee80211_tx_info *IEEE80211_SKB_CB(struct sk_buff *skb) {
    return (struct ieee80211_tx_info *)skb->cb;
}

static inline struct ieee80211_rx_status *
IEEE80211_SKB_RXCB(struct sk_buff *skb) {
    return (struct ieee80211_rx_status *)skb->cb;
}

struct ieee80211_hw *ieee80211_alloc_hw(size_t priv_len,
                                        const struct ieee80211_ops *ops);
void ieee80211_free_hw(struct ieee80211_hw *hw);
int ieee80211_register_hw(struct ieee80211_hw *hw);
void ieee80211_unregister_hw(struct ieee80211_hw *hw);

static inline void ieee80211_hw_set(struct ieee80211_hw *hw, int flag) {
    if (hw)
        hw->flags |= BIT(flag);
}

static inline bool ieee80211_hw_check(struct ieee80211_hw *hw, int flag) {
    return hw ? !!(hw->flags & BIT(flag)) : false;
}

static inline void
ieee80211_tx_info_clear_status(struct ieee80211_tx_info *info) {
    struct ieee80211_tx_rate rates[IEEE80211_TX_MAX_RATES];

    if (!info)
        return;

    memcpy(rates, info->status.rates, sizeof(rates));
    memset(info, 0, sizeof(*info));
    memcpy(info->status.rates, rates, sizeof(rates));
    for (int i = 0; i < IEEE80211_TX_MAX_RATES; i++)
        info->status.rates[i].count = 0;
}

void ieee80211_tx_status_irqsafe(struct ieee80211_hw *hw, struct sk_buff *skb);

static inline void ieee80211_tx_status_ext(struct ieee80211_hw *hw,
                                           struct ieee80211_tx_status *status) {
    (void)status;
    if (status)
        ieee80211_tx_status_irqsafe(hw, status->skb);
}

void ieee80211_rx_irqsafe(struct ieee80211_hw *hw, struct sk_buff *skb);
void ieee80211_scan_completed(struct ieee80211_hw *hw,
                              struct cfg80211_scan_info *info);

static inline void SET_IEEE80211_DEV(struct ieee80211_hw *hw,
                                     struct device *dev) {
    if (hw) {
        hw->dev = dev;
        if (hw->wiphy && dev)
            hw->wiphy->dev = *dev;
    }
}

static inline struct ieee80211_hw *wiphy_to_ieee80211_hw(struct wiphy *wiphy) {
    return wiphy ? wiphy->hw : NULL;
}

static inline void SET_IEEE80211_PERM_ADDR(struct ieee80211_hw *hw,
                                           const u8 *addr) {
    if (hw && addr)
        ether_addr_copy(hw->perm_addr, addr);
}

static inline void ieee80211_free_txskb(struct ieee80211_hw *hw,
                                        struct sk_buff *skb) {
    (void)hw;
    dev_kfree_skb_any(skb);
}

static inline void ieee80211_queue_work(struct ieee80211_hw *hw,
                                        struct work_struct *work) {
    (void)hw;
    schedule_work(work);
}

static inline void ieee80211_queue_delayed_work(struct ieee80211_hw *hw,
                                                struct delayed_work *dwork,
                                                unsigned long delay) {
    (void)hw;
    queue_delayed_work(system_wq, dwork, delay);
}

static inline int ieee80211_restart_hw(struct ieee80211_hw *hw) {
    (void)hw;
    return 0;
}

static inline int ieee80211_start_tx_ba_session(struct ieee80211_sta *sta,
                                                u16 tid, u16 timeout) {
    (void)sta;
    (void)tid;
    (void)timeout;
    return 0;
}

static inline void ieee80211_stop_tx_ba_cb_irqsafe(struct ieee80211_vif *vif,
                                                   const u8 *ra, u16 tid) {
    (void)vif;
    (void)ra;
    (void)tid;
}

static inline int ieee80211_channel_to_frequency(int chan,
                                                 enum nl80211_band band) {
    if (band == NL80211_BAND_2GHZ)
        return chan == 14 ? 2484 : 2407 + chan * 5;
    if (band == NL80211_BAND_5GHZ)
        return 5000 + chan * 5;
    return 0;
}

static inline const char *
ieee80211_create_tpt_led_trigger(struct ieee80211_hw *hw, unsigned int flags,
                                 const struct ieee80211_tpt_blink *blink_table,
                                 int blink_table_len) {
    (void)hw;
    (void)flags;
    (void)blink_table;
    (void)blink_table_len;
    return "phy0tpt";
}

#define LCOMPAT_IEEE80211_FTYPE_CTL 0x0004
#define LCOMPAT_IEEE80211_FTYPE_DATA 0x0008
#define LCOMPAT_IEEE80211_STYPE_PROBE_REQ 0x0040
#define LCOMPAT_IEEE80211_STYPE_PROBE_RESP 0x0050
#define LCOMPAT_IEEE80211_STYPE_BEACON 0x0080
#define LCOMPAT_IEEE80211_STYPE_PS_POLL 0x00A0
#define LCOMPAT_IEEE80211_STYPE_NULLFUNC 0x0040
#define LCOMPAT_IEEE80211_STYPE_QOS_NULLFUNC 0x00C0
#define LCOMPAT_IEEE80211_FCTL_TODS BIT(8)

static inline bool lcompat_ieee80211_size_add(size_t a, size_t b, size_t *out) {
    if (a > SIZE_MAX - b)
        return true;
    *out = a + b;
    return false;
}

static inline size_t
lcompat_ieee80211_extra_tx_headroom(struct ieee80211_hw *hw) {
    if (!hw || hw->extra_tx_headroom <= 0)
        return 0;
    return (size_t)hw->extra_tx_headroom;
}

static inline struct sk_buff *
lcompat_ieee80211_alloc_helper_skb(struct ieee80211_hw *hw, size_t payload_len,
                                   size_t tailroom) {
    struct sk_buff *skb;
    size_t headroom;
    size_t skb_len;

    headroom = lcompat_ieee80211_extra_tx_headroom(hw);
    if (lcompat_ieee80211_size_add(headroom, payload_len, &skb_len) ||
        lcompat_ieee80211_size_add(skb_len, tailroom, &skb_len) ||
        skb_len > UINT_MAX || headroom > UINT_MAX)
        return NULL;

    skb = alloc_skb(skb_len, GFP_KERNEL);
    if (!skb)
        return NULL;

    if (headroom)
        skb_reserve(skb, (unsigned int)headroom);

    return skb;
}

static inline const u8 *lcompat_ieee80211_vif_bssid(struct ieee80211_vif *vif) {
    if (!vif)
        return NULL;
    if (!is_zero_ether_addr(vif->bss_conf.bssid))
        return vif->bss_conf.bssid;
    return vif->addr;
}

static inline void lcompat_ieee80211_copy_addr(u8 *dst, const u8 *src,
                                               bool broadcast_if_null) {
    if (src)
        ether_addr_copy(dst, src);
    else if (broadcast_if_null)
        eth_broadcast_addr(dst);
    else
        eth_zero_addr(dst);
}

static inline void lcompat_ieee80211_fill_3addr(struct ieee80211_hdr *hdr,
                                                u16 frame_control,
                                                const u8 *addr1,
                                                const u8 *addr2,
                                                const u8 *addr3) {
    hdr->frame_control = cpu_to_le16(frame_control);
    lcompat_ieee80211_copy_addr(hdr->addr1, addr1, true);
    lcompat_ieee80211_copy_addr(hdr->addr2, addr2, false);
    lcompat_ieee80211_copy_addr(hdr->addr3, addr3, true);
}

static inline struct sk_buff *
ieee80211_probereq_get(struct ieee80211_hw *hw, const u8 *src_addr,
                       const u8 *ssid, size_t ssid_len, size_t tailroom) {
    struct ieee80211_hdr *hdr;
    struct sk_buff *skb;
    size_t frame_len;
    u8 *ie;

    if (ssid_len > U8_MAX)
        return NULL;
    if (lcompat_ieee80211_size_add(sizeof(*hdr), 2 + ssid_len, &frame_len))
        return NULL;

    skb = lcompat_ieee80211_alloc_helper_skb(hw, frame_len, tailroom);
    if (!skb)
        return NULL;

    hdr = skb_put_zero(skb, sizeof(*hdr));
    if (!hdr)
        goto err;
    lcompat_ieee80211_fill_3addr(hdr, LCOMPAT_IEEE80211_STYPE_PROBE_REQ, NULL,
                                 src_addr, NULL);

    ie = skb_put(skb, (unsigned int)(2 + ssid_len));
    if (!ie)
        goto err;
    ie[0] = 0;
    ie[1] = (u8)ssid_len;
    if (ssid_len && ssid)
        memcpy(ie + 2, ssid, ssid_len);

    return skb;

err:
    dev_kfree_skb_any(skb);
    return NULL;
}

static inline struct sk_buff *
ieee80211_proberesp_get(struct ieee80211_hw *hw, struct ieee80211_vif *vif) {
    struct ieee80211_hdr *hdr;
    struct sk_buff *skb;
    const u8 *bssid = lcompat_ieee80211_vif_bssid(vif);

    skb = lcompat_ieee80211_alloc_helper_skb(hw, sizeof(*hdr), 0);
    if (!skb)
        return NULL;
    hdr = skb_put_zero(skb, sizeof(*hdr));
    if (!hdr) {
        dev_kfree_skb_any(skb);
        return NULL;
    }
    lcompat_ieee80211_fill_3addr(hdr, LCOMPAT_IEEE80211_STYPE_PROBE_RESP, NULL,
                                 vif ? vif->addr : NULL, bssid);
    IEEE80211_SKB_CB(skb)->control.vif = vif;
    return skb;
}

static inline struct sk_buff *
ieee80211_beacon_get_tim(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
                         u16 *tim_offset, void *tim_length,
                         unsigned int link_id) {
    struct ieee80211_hdr *hdr;
    struct sk_buff *skb;
    const u8 *bssid = lcompat_ieee80211_vif_bssid(vif);

    (void)tim_length;
    (void)link_id;
    if (tim_offset)
        *tim_offset = 0;

    skb = lcompat_ieee80211_alloc_helper_skb(hw, sizeof(*hdr), 0);
    if (!skb)
        return NULL;
    hdr = skb_put_zero(skb, sizeof(*hdr));
    if (!hdr) {
        dev_kfree_skb_any(skb);
        return NULL;
    }
    lcompat_ieee80211_fill_3addr(hdr, LCOMPAT_IEEE80211_STYPE_BEACON, NULL,
                                 vif ? vif->addr : NULL, bssid);
    IEEE80211_SKB_CB(skb)->control.vif = vif;
    return skb;
}

static inline struct sk_buff *ieee80211_beacon_get(struct ieee80211_hw *hw,
                                                   struct ieee80211_vif *vif,
                                                   unsigned int link_id) {
    return ieee80211_beacon_get_tim(hw, vif, NULL, NULL, link_id);
}

static inline struct sk_buff *ieee80211_beacon_get_template(
    struct ieee80211_hw *hw, struct ieee80211_vif *vif,
    struct ieee80211_mutable_offsets *offs, unsigned int link_id) {
    if (offs)
        memset(offs, 0, sizeof(*offs));
    return ieee80211_beacon_get(hw, vif, link_id);
}

static inline struct sk_buff *
ieee80211_get_buffered_bc(struct ieee80211_hw *hw, struct ieee80211_vif *vif) {
    (void)hw;
    (void)vif;
    return NULL;
}

static inline struct sk_buff *ieee80211_pspoll_get(struct ieee80211_hw *hw,
                                                   struct ieee80211_vif *vif) {
    struct ieee80211_hdr *hdr;
    struct sk_buff *skb;

    skb = lcompat_ieee80211_alloc_helper_skb(hw, sizeof(*hdr), 0);
    if (!skb)
        return NULL;
    hdr = skb_put_zero(skb, sizeof(*hdr));
    if (!hdr) {
        dev_kfree_skb_any(skb);
        return NULL;
    }
    lcompat_ieee80211_fill_3addr(
        hdr, LCOMPAT_IEEE80211_FTYPE_CTL | LCOMPAT_IEEE80211_STYPE_PS_POLL,
        lcompat_ieee80211_vif_bssid(vif), vif ? vif->addr : NULL,
        lcompat_ieee80211_vif_bssid(vif));
    IEEE80211_SKB_CB(skb)->control.vif = vif;
    return skb;
}

static inline struct sk_buff *
lcompat_ieee80211_nullfunc_get(struct ieee80211_hw *hw,
                               struct ieee80211_vif *vif, bool qos) {
    struct ieee80211_hdr *hdr;
    struct sk_buff *skb;
    u16 fc = LCOMPAT_IEEE80211_FTYPE_DATA | LCOMPAT_IEEE80211_FCTL_TODS;

    fc |= qos ? LCOMPAT_IEEE80211_STYPE_QOS_NULLFUNC
              : LCOMPAT_IEEE80211_STYPE_NULLFUNC;

    skb = lcompat_ieee80211_alloc_helper_skb(hw, sizeof(*hdr), 0);
    if (!skb)
        return NULL;
    hdr = skb_put_zero(skb, sizeof(*hdr));
    if (!hdr) {
        dev_kfree_skb_any(skb);
        return NULL;
    }
    lcompat_ieee80211_fill_3addr(hdr, fc, lcompat_ieee80211_vif_bssid(vif),
                                 vif ? vif->addr : NULL,
                                 lcompat_ieee80211_vif_bssid(vif));
    IEEE80211_SKB_CB(skb)->control.vif = vif;
    return skb;
}

#define __lcompat_ieee80211_nullfunc_get2(hw, vif)                             \
    lcompat_ieee80211_nullfunc_get((hw), (vif), false)
#define __lcompat_ieee80211_nullfunc_get3(hw, vif, qos)                        \
    lcompat_ieee80211_nullfunc_get((hw), (vif), (qos))
#define __lcompat_ieee80211_nullfunc_get4(hw, vif, link_id, qos)               \
    ((void)(link_id), lcompat_ieee80211_nullfunc_get((hw), (vif), (qos)))
#define __lcompat_ieee80211_nullfunc_pick(_1, _2, _3, _4, name, ...) name
#define ieee80211_nullfunc_get(...)                                            \
    __lcompat_ieee80211_nullfunc_pick(                                         \
        __VA_ARGS__, __lcompat_ieee80211_nullfunc_get4,                        \
        __lcompat_ieee80211_nullfunc_get3,                                     \
        __lcompat_ieee80211_nullfunc_get2)(__VA_ARGS__)

static inline int ieee80211_vif_type_p2p(struct ieee80211_vif *vif) {
    return vif ? vif->type : 0;
}

#define ieee80211_request_smps(...) 0

static inline struct ieee80211_sta *
ieee80211_find_sta(struct ieee80211_vif *vif, const u8 *addr) {
    extern struct ieee80211_sta *lcompat_ieee80211_find_sta(
        struct ieee80211_vif * vif, const u8 *addr);

    return lcompat_ieee80211_find_sta(vif, addr);
}

static inline struct ieee80211_sta *
ieee80211_find_sta_by_ifaddr(struct ieee80211_hw *hw, const u8 *addr,
                             const u8 *localaddr) {
    extern struct ieee80211_sta *lcompat_ieee80211_find_sta_by_ifaddr(
        struct ieee80211_hw * hw, const u8 *addr, const u8 *localaddr);

    return lcompat_ieee80211_find_sta_by_ifaddr(hw, addr, localaddr);
}

static inline bool ieee80211_vif_is_mld(struct ieee80211_vif *vif) {
    return vif ? !!vif->valid_links : false;
}

static inline struct ieee80211_bss_conf *
link_conf_dereference_protected(struct ieee80211_vif *vif, u8 link_id) {
    if (!vif)
        return NULL;
    if (link_id < IEEE80211_MLD_MAX_NUM_LINKS && vif->link_conf[link_id])
        return vif->link_conf[link_id];
    return &vif->bss_conf;
}

static inline struct ieee80211_link_sta *
link_sta_dereference_protected(struct ieee80211_sta *sta, u8 link_id) {
    (void)link_id;
    return sta ? (struct ieee80211_link_sta *)&sta->deflink : NULL;
}

static inline void ieee80211_stop_queues(struct ieee80211_hw *hw) { (void)hw; }
static inline void ieee80211_wake_queues(struct ieee80211_hw *hw) { (void)hw; }
static inline struct sk_buff *ieee80211_tx_dequeue(struct ieee80211_hw *hw,
                                                   struct ieee80211_txq *txq) {
    (void)hw;
    (void)txq;
    return NULL;
}

static inline void ieee80211_purge_tx_queue(struct ieee80211_hw *hw,
                                            struct sk_buff_head *queue) {
    (void)hw;
    skb_queue_purge(queue);
}

static inline struct ieee80211_txq *ieee80211_next_txq(struct ieee80211_hw *hw,
                                                       int ac) {
    (void)hw;
    (void)ac;
    return NULL;
}

static inline void ieee80211_return_txq(struct ieee80211_hw *hw,
                                        struct ieee80211_txq *txq, bool force) {
    (void)hw;
    (void)txq;
    (void)force;
}

static inline void ieee80211_schedule_txq(struct ieee80211_hw *hw,
                                          struct ieee80211_txq *txq) {
    (void)hw;
    (void)txq;
}

static inline void ieee80211_txq_schedule_start(struct ieee80211_hw *hw,
                                                int ac) {
    (void)hw;
    (void)ac;
}

static inline void ieee80211_txq_schedule_end(struct ieee80211_hw *hw, int ac) {
    (void)hw;
    (void)ac;
}

static inline void ieee80211_txq_get_depth(struct ieee80211_txq *txq,
                                           unsigned long *frame_cnt,
                                           unsigned long *byte_cnt) {
    (void)txq;
    if (frame_cnt)
        *frame_cnt = 0;
    if (byte_cnt)
        *byte_cnt = 0;
}

static inline void ieee80211_cqm_rssi_notify(struct ieee80211_vif *vif,
                                             int event, s32 rssi, gfp_t gfp) {
    (void)vif;
    (void)event;
    (void)rssi;
    (void)gfp;
}

static inline void ieee80211_connection_loss(struct ieee80211_vif *vif) {
    (void)vif;
}

void ieee80211_disconnect(struct ieee80211_vif *vif, bool reconnect);

void ieee80211_iterate_active_interfaces(
    struct ieee80211_hw *hw, int iterator_flags,
    void (*iterator)(void *data, u8 *mac, struct ieee80211_vif *vif),
    void *data);

void ieee80211_iterate_interfaces(struct ieee80211_hw *hw, int iterator_flags,
                                  void (*iterator)(void *data, u8 *mac,
                                                   struct ieee80211_vif *vif),
                                  void *data);

void ieee80211_iterate_active_interfaces_atomic(
    struct ieee80211_hw *hw, int iterator_flags,
    void (*iterator)(void *data, u8 *mac, struct ieee80211_vif *vif),
    void *data);

void ieee80211_iterate_stations_atomic(
    struct ieee80211_hw *hw,
    void (*iterator)(void *data, struct ieee80211_sta *sta), void *data);

static inline void ieee80211_iter_keys_rcu(
    struct ieee80211_hw *hw, struct ieee80211_vif *vif,
    void (*iterator)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
                     struct ieee80211_sta *sta, struct ieee80211_key_conf *key,
                     void *data),
    void *data) {
    (void)hw;
    (void)vif;
    (void)iterator;
    (void)data;
}

static inline bool ieee80211_has_tods(__le16 fc) {
    return !!(le16_to_cpu(fc) & BIT(8));
}

static inline bool ieee80211_has_fromds(__le16 fc) {
    return !!(le16_to_cpu(fc) & BIT(9));
}

static inline bool ieee80211_has_a4(__le16 fc) {
    return ieee80211_has_tods(fc) && ieee80211_has_fromds(fc);
}

static inline bool ieee80211_has_order(__le16 fc) {
    return !!(le16_to_cpu(fc) & IEEE80211_FCTL_ORDER);
}

static inline bool ieee80211_is_mgmt(__le16 fc) {
    return ((le16_to_cpu(fc) >> 2) & 0x3) == 0;
}

static inline bool ieee80211_is_ctl(__le16 fc) {
    return ((le16_to_cpu(fc) >> 2) & 0x3) == 1;
}

static inline bool ieee80211_is_data(__le16 fc) {
    return ((le16_to_cpu(fc) >> 2) & 0x3) == 2;
}

static inline bool ieee80211_is_any_nullfunc(__le16 fc) {
    u16 fc_cpu = le16_to_cpu(fc);

    return ieee80211_is_data(fc) &&
           ((fc_cpu & 0x00f0) == 0x0040 || (fc_cpu & 0x00f0) == 0x00c0);
}

static inline bool ieee80211_is_data_qos(__le16 fc) {
    return ieee80211_is_data(fc) && !!(le16_to_cpu(fc) & 0x0080);
}

static inline bool ieee80211_is_data_present(__le16 fc) {
    return ieee80211_is_data(fc) && !ieee80211_is_any_nullfunc(fc);
}

static inline bool ieee80211_is_qos_nullfunc(__le16 fc) {
    return ieee80211_is_data(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x00c0);
}

static inline bool ieee80211_is_pspoll(__le16 fc) {
    return ieee80211_is_ctl(fc);
}

static inline bool ieee80211_is_frag(const struct ieee80211_hdr *hdr) {
    return hdr && !!(le16_to_cpu(hdr->seq_ctrl) & 0x000f);
}

static inline bool ieee80211_is_first_frag(__le16 seq_ctrl) {
    return !(le16_to_cpu(seq_ctrl) & 0x000f);
}

static inline bool ieee80211_has_morefrags(__le16 fc) {
    return !!(le16_to_cpu(fc) & BIT(10));
}

static inline bool ieee80211_has_pm(__le16 fc) {
    return !!(le16_to_cpu(fc) & BIT(12));
}

static inline int ieee80211_get_hdrlen_from_skb(const struct sk_buff *skb) {
    (void)skb;
    return 24;
}

static inline unsigned int ieee80211_hdrlen(__le16 fc) {
    unsigned int hdrlen = 24;

    if (ieee80211_is_ctl(fc))
        return 16;
    if (ieee80211_is_mgmt(fc))
        return 24;
    if (ieee80211_has_tods(fc) && ieee80211_has_fromds(fc))
        hdrlen += ETH_ALEN;
    if (ieee80211_is_data_qos(fc))
        hdrlen += 2;
    return hdrlen;
}

static inline int
ieee80211_calc_rx_airtime(struct ieee80211_hw *hw,
                          const struct ieee80211_rx_status *status, int len) {
    (void)hw;
    (void)status;
    return len;
}

static inline void ieee80211_sta_register_airtime(struct ieee80211_sta *sta,
                                                  u8 tidno, u32 tx_airtime,
                                                  u32 rx_airtime) {
    (void)sta;
    (void)tidno;
    (void)tx_airtime;
    (void)rx_airtime;
}

static inline void ieee80211_sta_pspoll(struct ieee80211_sta *sta) {
    (void)sta;
}

static inline void ieee80211_sta_uapsd_trigger(struct ieee80211_sta *sta,
                                               u8 tid) {
    (void)sta;
    (void)tid;
}

static inline void ieee80211_sta_ps_transition(struct ieee80211_sta *sta,
                                               bool start) {
    (void)sta;
    (void)start;
}

static inline void ieee80211_rx_list(struct ieee80211_hw *hw,
                                     struct ieee80211_sta *sta,
                                     struct sk_buff *skb,
                                     struct list_head *list) {
    (void)hw;
    (void)sta;
    if (!skb || !list)
        return;
    list_add_tail(&skb->list, list);
}

static inline unsigned long
ieee80211_vif_usable_links(struct ieee80211_vif *vif) {
    return vif ? (vif->valid_links ? vif->valid_links : BIT(0)) : BIT(0);
}

static inline bool ieee80211_tx_prepare_skb(struct ieee80211_hw *hw,
                                            struct ieee80211_vif *vif,
                                            struct sk_buff *skb,
                                            enum nl80211_band band,
                                            struct ieee80211_sta **sta) {
    (void)hw;
    (void)vif;
    (void)skb;
    (void)band;
    if (sta)
        *sta = NULL;
    return true;
}

static inline int ieee80211_get_key_rx_seq(struct ieee80211_key_conf *key,
                                           int tid,
                                           struct ieee80211_key_seq *seq) {
    (void)key;
    (void)tid;
    if (seq)
        memset(seq, 0, sizeof(*seq));
    return 0;
}

static inline bool ieee80211_is_beacon(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x0080);
}

static inline bool ieee80211_is_probe_resp(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x0050);
}

static inline bool ieee80211_is_probe_req(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x0040);
}

static inline bool ieee80211_is_back_req(__le16 fc) {
    return ieee80211_is_ctl(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x0080);
}

static inline bool ieee80211_is_action(__le16 fc) {
    return ieee80211_is_mgmt(fc) &&
           ((le16_to_cpu(fc) & IEEE80211_FCTL_STYPE) == IEEE80211_STYPE_ACTION);
}

static inline bool ieee80211_is_robust_mgmt_frame(struct sk_buff *skb) {
    (void)skb;
    return false;
}

static inline void
ieee80211_refresh_tx_agg_session_timer(struct ieee80211_sta *sta, u16 tid) {
    (void)sta;
    (void)tid;
}

static inline bool ieee80211_is_deauth(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x00c0);
}

static inline bool ieee80211_is_auth(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x00b0);
}

static inline bool ieee80211_is_assoc_req(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x0000);
}

static inline bool ieee80211_is_assoc_resp(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x0010);
}

static inline bool ieee80211_is_disassoc(__le16 fc) {
    return ieee80211_is_mgmt(fc) && ((le16_to_cpu(fc) & 0x00f0) == 0x00a0);
}

static inline bool ieee80211_is_bufferable_mmpdu(struct sk_buff *skb) {
    (void)skb;
    return true;
}

static inline bool
ieee80211_beacon_cntdwn_is_complete(struct ieee80211_vif *vif,
                                    unsigned int link_id) {
    (void)vif;
    (void)link_id;
    return true;
}

static inline void ieee80211_csa_finish(struct ieee80211_vif *vif,
                                        unsigned int link_id) {
    (void)link_id;
    if (vif)
        vif->bss_conf.csa_active = false;
}

static inline void ieee80211_beacon_loss(struct ieee80211_vif *vif) {
    (void)vif;
}

static inline void ieee80211_sched_scan_results(struct ieee80211_hw *hw) {
    (void)hw;
}

static inline void ieee80211_chswitch_done(struct ieee80211_vif *vif,
                                           bool success, unsigned int link_id) {
    (void)link_id;
    if (vif)
        vif->bss_conf.csa_active = false;
    (void)success;
}

static inline void
ieee80211_remain_on_channel_expired(struct ieee80211_hw *hw) {
    (void)hw;
}

static inline void ieee80211_ready_on_channel(struct ieee80211_hw *hw) {
    (void)hw;
}

static inline void ieee80211_sta_eosp(struct ieee80211_sta *sta) { (void)sta; }

static inline void ieee80211_send_bar(struct ieee80211_vif *vif, const u8 *ra,
                                      u16 tid, u16 ssn) {
    (void)vif;
    (void)ra;
    (void)tid;
    (void)ssn;
}

static inline void ieee80211_get_tx_rates(struct ieee80211_vif *vif,
                                          struct ieee80211_sta *sta,
                                          struct sk_buff *skb,
                                          struct ieee80211_tx_rate *rates,
                                          int max_rates) {
    (void)vif;
    (void)sta;
    (void)skb;
    if (!rates || max_rates <= 0)
        return;
    rates[0].idx = -1;
    rates[0].count = 1;
    rates[0].flags = 0;
}

static inline u16 ieee80211_sn_inc(u16 sn) { return (sn + 1) & 0xfff; }

static inline bool ieee80211_sn_less(u16 sn1, u16 sn2) {
    return ((sn1 - sn2) & 0xfff) > 0x800;
}

static inline u16 ieee80211_sn_sub(u16 sn1, u16 sn2) {
    return (sn1 - sn2) & 0xfff;
}

#define IEEE80211_TPT_LEDTRIG_FL_RADIO BIT(0)

static const u8 rfc1042_header[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
static const u8 bridge_tunnel_header[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};

#define IEEE80211_TX_CTL_REQ_TX_STATUS BIT(0)
#define IEEE80211_TX_CTL_NO_ACK BIT(1)
#define IEEE80211_TX_CTL_AMPDU BIT(2)
#define IEEE80211_TX_CTL_RATE_CTRL_PROBE BIT(3)
#define IEEE80211_TX_CTL_HW_80211_ENCAP BIT(4)
#define IEEE80211_TX_CTL_TX_OFFCHAN BIT(5)
#define IEEE80211_TX_CTL_NO_CCK_RATE BIT(6)
#define IEEE80211_TX_CTL_LDPC BIT(7)
#define IEEE80211_TX_CTL_STBC BIT(8)
#define IEEE80211_TX_CTL_ASSIGN_SEQ BIT(9)
#define IEEE80211_TX_CTL_USE_MINRATE BIT(10)
#define IEEE80211_TX_CTL_INJECTED BIT(11)
#define IEEE80211_TX_STAT_NOACK_TRANSMITTED BIT(2)
#define IEEE80211_TX_STAT_ACK BIT(3)
#define IEEE80211_TX_STATUS_EOSP BIT(4)
#define IEEE80211_TX_STAT_AMPDU BIT(5)
#define IEEE80211_TX_CTRL_DONT_USE_RATE_MASK BIT(12)
#define IEEE80211_TX_CTRL_PS_RESPONSE BIT(13)
#define IEEE80211_QOS_CTL_TID_MASK 0x000f
#define IEEE80211_QOS_CTL_TAG1D_MASK 0x0007
#define IEEE80211_QOS_CTL_ACK_POLICY_MASK 0x0060
#define IEEE80211_QOS_CTL_ACK_POLICY_NOACK 0x0020
#define IEEE80211_SEQ_TO_SN(seq) (((seq) & IEEE80211_SCTL_SEQ) >> 4)
#define IEEE80211_MIN_AMPDU_BUF 0x8

static inline bool ieee80211_has_protected(__le16 fc) {
    return !!(le16_to_cpu(fc) & BIT(14));
}

static inline u8 *ieee80211_get_qos_ctl(struct ieee80211_hdr *hdr) {
    return hdr ? ((u8 *)hdr) + sizeof(*hdr) : NULL;
}

static inline u32 ieee80211_calc_tx_airtime(struct ieee80211_hw *hw,
                                            struct ieee80211_tx_info *info,
                                            int len) {
    (void)hw;
    (void)info;
    return len > 0 ? (u32)len : 0;
}

static inline void ieee80211_radar_detected(struct ieee80211_hw *hw,
                                            struct cfg80211_chan_def *chandef) {
    (void)hw;
    (void)chandef;
}

static inline const struct ieee80211_sta_he_cap *
ieee80211_get_he_iftype_cap(const struct ieee80211_supported_band *sband,
                            enum nl80211_iftype iftype) {
    int i;

    if (!sband)
        return NULL;
    for (i = 0; i < sband->n_iftype_data; i++) {
        if (sband->iftype_data[i].types_mask & BIT(iftype))
            return &sband->iftype_data[i].he_cap;
    }
    return sband ? &sband->he_cap : NULL;
}

static inline const struct ieee80211_sta_eht_cap *
ieee80211_get_eht_iftype_cap(const struct ieee80211_supported_band *sband,
                             enum nl80211_iftype iftype) {
    int i;

    if (!sband)
        return NULL;
    for (i = 0; i < sband->n_iftype_data; i++) {
        if (sband->iftype_data[i].types_mask & BIT(iftype))
            return &sband->iftype_data[i].eht_cap;
    }
    return sband ? &sband->eht_cap : NULL;
}

static inline void
_ieee80211_set_sband_iftype_data(struct ieee80211_supported_band *sband,
                                 const struct ieee80211_sband_iftype_data *data,
                                 int n) {
    if (!sband)
        return;
    sband->iftype_data = data;
    sband->n_iftype_data = n;
    if (data && n > 0) {
        sband->he_cap = data[0].he_cap;
        sband->eht_cap = data[0].eht_cap;
    }
}

static inline int ieee80211_frequency_to_channel(int freq) {
    if (freq == 2484)
        return 14;
    if (freq < 2484)
        return (freq - 2407) / 5;
    if (freq >= 5955)
        return (freq - 5950) / 5;
    return (freq - 5000) / 5;
}

static inline int
ieee80211_emulate_add_chanctx(struct ieee80211_hw *hw,
                              struct ieee80211_chanctx_conf *ctx) {
    (void)hw;
    (void)ctx;
    return 0;
}

static inline void
ieee80211_emulate_remove_chanctx(struct ieee80211_hw *hw,
                                 struct ieee80211_chanctx_conf *ctx) {
    (void)hw;
    (void)ctx;
}

static inline void ieee80211_emulate_change_chanctx(
    struct ieee80211_hw *hw, struct ieee80211_chanctx_conf *ctx, u32 changed) {
    (void)hw;
    (void)ctx;
    (void)changed;
}

static inline int ieee80211_emulate_switch_vif_chanctx(
    struct ieee80211_hw *hw, struct ieee80211_vif_chanctx_switch *vifs,
    int n_vifs, enum ieee80211_chanctx_switch_mode mode) {
    (void)hw;
    (void)vifs;
    (void)n_vifs;
    (void)mode;
    return 0;
}
