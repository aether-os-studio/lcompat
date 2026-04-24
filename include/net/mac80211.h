#pragma once

#include <asm/byteorder.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>
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

#define IEEE80211_VIF_BEACON_FILTER BIT(0)
#define IEEE80211_VIF_SUPPORTS_CQM_RSSI BIT(1)
#define IEEE80211_RATE_SHORT_PREAMBLE BIT(0)

#define FIF_ALLMULTI BIT(0)
#define FIF_OTHER_BSS BIT(1)
#define FIF_FCSFAIL BIT(2)
#define FIF_BCN_PRBRESP_PROMISC BIT(3)

#define BSS_CHANGED_ASSOC BIT(0)
#define BSS_CHANGED_BSSID BIT(1)
#define BSS_CHANGED_BEACON_INT BIT(2)
#define BSS_CHANGED_BEACON BIT(3)
#define BSS_CHANGED_BEACON_ENABLED BIT(4)
#define BSS_CHANGED_CQM BIT(5)
#define BSS_CHANGED_MU_GROUPS BIT(6)
#define BSS_CHANGED_ERP_SLOT BIT(7)
#define BSS_CHANGED_PS BIT(8)

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
#define IEEE80211_HT_MPDU_DENSITY_NONE 0
#define IEEE80211_HT_MPDU_DENSITY_025 1
#define IEEE80211_HT_MPDU_DENSITY_05 2
#define IEEE80211_HT_MPDU_DENSITY_1 3
#define IEEE80211_HT_MPDU_DENSITY_2 4
#define IEEE80211_HT_MPDU_DENSITY_4 5
#define IEEE80211_HT_MPDU_DENSITY_8 6
#define IEEE80211_HT_MPDU_DENSITY_16 7

#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 BIT(0)
#define IEEE80211_VHT_CAP_SHORT_GI_80 BIT(1)
#define IEEE80211_VHT_CAP_RXSTBC_1 BIT(2)
#define IEEE80211_VHT_CAP_RXSTBC_MASK IEEE80211_VHT_CAP_RXSTBC_1
#define IEEE80211_VHT_CAP_HTC_VHT BIT(3)
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK GENMASK(5, 4)
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT 4
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

#define IEEE80211_STA_RX_BW_20 0
#define IEEE80211_STA_RX_BW_40 1
#define IEEE80211_STA_RX_BW_80 2
#define IEEE80211_STA_RX_BW_160 3
#define IEEE80211_RC_BW_CHANGED BIT(0)

#define IEEE80211_SMPS_STATIC 1
#define IEEE80211_SMPS_OFF 2

#define SIGNAL_DBM 0
#define RX_INCLUDES_FCS 1
#define AMPDU_AGGREGATION 2
#define MFP_CAPABLE 3
#define REPORTS_TX_ACK_STATUS 4
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

#define IEEE80211_WMM_IE_STA_QOSINFO_SP_ALL 3

struct ieee80211_channel {
    int center_freq;
    int hw_value;
    enum nl80211_band band;
    u32 flags;
    int max_power;
};

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

struct ieee80211_supported_band {
    enum nl80211_band band;
    struct ieee80211_channel *channels;
    int n_channels;
    struct ieee80211_rate *bitrates;
    int n_bitrates;
    struct ieee80211_sta_ht_cap ht_cap;
    struct ieee80211_sta_vht_cap vht_cap;
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
};

struct ieee80211_tx_queue_params {
    u16 txop;
    u16 cw_min;
    u16 cw_max;
    u8 aifs;
};

struct ieee80211_vif;

struct ieee80211_conf {
    u32 flags;
    bool radar_enabled;
    struct cfg80211_chan_def chandef;
};

struct ieee80211_bss_conf {
    bool assoc;
    u16 beacon_int;
    u16 aid;
    u8 link_id;
    u8 bssid[ETH_ALEN];
    u8 transmitter_bssid[ETH_ALEN];
    u8 dtim_period;
    bool enable_beacon;
    bool use_short_slot;
    bool csa_active;
    bool nontransmitted;
    u32 basic_rates;
    s32 cqm_rssi_thold;
    u32 cqm_rssi_hyst;
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
    struct ieee80211_sta_ht_cap ht_cap;
    struct ieee80211_sta_vht_cap vht_cap;
    u8 supp_rates[IEEE80211_NUM_BANDS];
    u8 bandwidth;
    struct ieee80211_sta *sta;
};

struct ieee80211_sta {
    u8 addr[6];
    struct ieee80211_txq *txq[IEEE80211_NUM_TIDS];
    bool tdls;
    u16 valid_links;
    u8 supp_rates[IEEE80211_NUM_BANDS];
    u8 bandwidth;
    int max_rc_amsdu_len;
    struct ieee80211_sta_ht_cap ht_cap;
    struct ieee80211_sta_vht_cap vht_cap;
    struct ieee80211_link_sta deflink;
    char drv_priv[];
};

struct ieee80211_sband_iftype_data {
    u32 types_mask;
};

struct ieee80211_chanctx_conf {
    void *drv_priv;
    bool radar_enabled;
    struct cfg80211_chan_def def;
};

#define IEEE80211_CHANCTX_CHANGE_WIDTH BIT(0)
#define IEEE80211_CHANCTX_CHANGE_RADAR BIT(1)

struct rate_info {
    u32 flags;
    u16 legacy;
    u8 mcs;
    u8 nss;
    u8 bw;
};

#define RATE_INFO_FLAGS_VHT_MCS BIT(0)
#define RATE_INFO_FLAGS_MCS BIT(1)
#define RATE_INFO_FLAGS_SHORT_GI BIT(2)
#define RATE_INFO_BW_20 0
#define RATE_INFO_BW_40 1
#define RATE_INFO_BW_80 2

enum mac80211_rx_encoding {
    RX_ENC_LEGACY = 0,
    RX_ENC_HT = 1,
    RX_ENC_VHT = 2,
    RX_ENC_EHT = 3,
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

struct ieee80211_ampdu_params {
    struct ieee80211_sta *sta;
    u16 tid;
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

struct ieee80211_tx_rate {
    s8 idx;
    u8 count;
    u16 flags;
};

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
#define IEEE80211_TX_MAX_RATES 4

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
    u8 perm_addr[ETH_ALEN];
    void *lcompat_runtime;
    void *priv;
};

struct ieee80211_ops {
    void (*tx)(struct ieee80211_hw *, struct ieee80211_tx_control *,
               struct sk_buff *);
    void (*wake_tx_queue)(struct ieee80211_hw *, struct ieee80211_txq *);
    int (*start)(struct ieee80211_hw *);
    void (*stop)(struct ieee80211_hw *);
    int (*config)(struct ieee80211_hw *, u32);
    int (*add_interface)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*remove_interface)(struct ieee80211_hw *, struct ieee80211_vif *);
    int (*change_interface)(struct ieee80211_hw *, struct ieee80211_vif *,
                            enum nl80211_iftype, bool);
    void (*configure_filter)(struct ieee80211_hw *, unsigned int,
                             unsigned int *, u64);
    void (*bss_info_changed)(struct ieee80211_hw *, struct ieee80211_vif *,
                             struct ieee80211_bss_conf *, u32);
    int (*start_ap)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*stop_ap)(struct ieee80211_hw *, struct ieee80211_vif *);
    int (*conf_tx)(struct ieee80211_hw *, struct ieee80211_vif *, u16,
                   const struct ieee80211_tx_queue_params *);
    int (*sta_add)(struct ieee80211_hw *, struct ieee80211_vif *,
                   struct ieee80211_sta *);
    int (*sta_remove)(struct ieee80211_hw *, struct ieee80211_vif *,
                      struct ieee80211_sta *);
    int (*set_tim)(struct ieee80211_hw *, struct ieee80211_sta *, bool);
    int (*set_key)(struct ieee80211_hw *, enum set_key_cmd,
                   struct ieee80211_vif *, struct ieee80211_sta *,
                   struct ieee80211_key_conf *);
    int (*ampdu_action)(struct ieee80211_hw *, struct ieee80211_vif *,
                        struct ieee80211_ampdu_params *);
    bool (*can_aggregate_in_amsdu)(struct ieee80211_hw *, struct sk_buff *,
                                   struct sk_buff *);
    void (*sw_scan_start)(struct ieee80211_hw *, struct ieee80211_vif *,
                          const u8 *);
    void (*sw_scan_complete)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*mgd_prepare_tx)(struct ieee80211_hw *, struct ieee80211_vif *, u16);
    int (*set_rts_threshold)(struct ieee80211_hw *, u32);
    void (*sta_statistics)(struct ieee80211_hw *, struct ieee80211_vif *,
                           struct ieee80211_sta *, struct station_info *);
    void (*flush)(struct ieee80211_hw *, struct ieee80211_vif *, u32, bool);
    int (*set_bitrate_mask)(struct ieee80211_hw *, struct ieee80211_vif *,
                            const struct cfg80211_bitrate_mask *);
    int (*set_antenna)(struct ieee80211_hw *, u32, u32);
    int (*get_antenna)(struct ieee80211_hw *, u32 *, u32 *);
    void (*reconfig_complete)(struct ieee80211_hw *,
                              enum ieee80211_reconfig_type);
    int (*hw_scan)(struct ieee80211_hw *, struct ieee80211_vif *,
                   struct ieee80211_scan_request *);
    void (*cancel_hw_scan)(struct ieee80211_hw *, struct ieee80211_vif *);
    void (*sta_rc_update)(struct ieee80211_hw *, struct ieee80211_vif *,
                          struct ieee80211_sta *, u32);
    int (*set_sar_specs)(struct ieee80211_hw *,
                         const struct cfg80211_sar_specs *);
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
    u8 data[12];
};

struct ieee80211_radiotap_he_mu {
    u8 data[12];
};

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
        if (hw->wiphy)
            hw->wiphy->dev = dev;
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

static inline struct sk_buff *
ieee80211_probereq_get(struct ieee80211_hw *hw, const u8 *src_addr,
                       const u8 *ssid, size_t ssid_len, size_t tailroom) {
    (void)hw;
    (void)src_addr;
    (void)ssid;
    (void)ssid_len;
    return alloc_skb(tailroom ? tailroom : 64, GFP_KERNEL);
}

static inline struct sk_buff *
ieee80211_proberesp_get(struct ieee80211_hw *hw, struct ieee80211_vif *vif) {
    (void)hw;
    (void)vif;
    return alloc_skb(64, GFP_KERNEL);
}

static inline struct sk_buff *
ieee80211_beacon_get_tim(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
                         u16 *tim_offset, void *tim_length) {
    (void)hw;
    (void)vif;
    (void)tim_length;
    if (tim_offset)
        *tim_offset = 0;
    return alloc_skb(128, GFP_KERNEL);
}

static inline struct sk_buff *ieee80211_pspoll_get(struct ieee80211_hw *hw,
                                                   struct ieee80211_vif *vif) {
    (void)hw;
    (void)vif;
    return alloc_skb(32, GFP_KERNEL);
}

static inline struct sk_buff *ieee80211_nullfunc_get(struct ieee80211_hw *hw,
                                                     struct ieee80211_vif *vif,
                                                     bool qos) {
    (void)hw;
    (void)vif;
    (void)qos;
    return alloc_skb(32, GFP_KERNEL);
}

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

static inline void ieee80211_iterate_active_interfaces_atomic(
    struct ieee80211_hw *hw, int iterator_flags,
    void (*iterator)(void *data, u8 *mac, struct ieee80211_vif *vif),
    void *data) {
    (void)hw;
    (void)iterator_flags;
    (void)iterator;
    (void)data;
}

static inline void ieee80211_iterate_stations_atomic(
    struct ieee80211_hw *hw,
    void (*iterator)(void *data, struct ieee80211_sta *sta), void *data) {
    (void)hw;
    (void)iterator;
    (void)data;
}

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
    (void)fc;
    return false;
}

static inline bool ieee80211_is_data_qos(__le16 fc) {
    return ieee80211_is_data(fc);
}

static inline bool ieee80211_is_data_present(__le16 fc) {
    return ieee80211_is_data(fc) && !ieee80211_is_any_nullfunc(fc);
}

static inline bool ieee80211_is_qos_nullfunc(__le16 fc) {
    (void)fc;
    return false;
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
    return sizeof(struct ieee80211_hdr);
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

#define IEEE80211_TX_CTL_REQ_TX_STATUS BIT(0)
#define IEEE80211_TX_CTL_NO_ACK BIT(1)
#define IEEE80211_TX_CTL_AMPDU BIT(2)
#define IEEE80211_TX_CTL_RATE_CTRL_PROBE BIT(3)
#define IEEE80211_TX_CTL_HW_80211_ENCAP BIT(4)
#define IEEE80211_TX_CTL_TX_OFFCHAN BIT(5)
#define IEEE80211_TX_CTL_NO_CCK_RATE BIT(6)
#define IEEE80211_TX_STAT_NOACK_TRANSMITTED BIT(2)
#define IEEE80211_TX_STAT_ACK BIT(3)
#define IEEE80211_TX_STATUS_EOSP BIT(4)
#define IEEE80211_TX_CTRL_DONT_USE_RATE_MASK BIT(12)
#define IEEE80211_TX_CTRL_PS_RESPONSE BIT(13)
#define IEEE80211_QOS_CTL_TID_MASK 0x000f
#define IEEE80211_QOS_CTL_TAG1D_MASK 0x0007
#define IEEE80211_QOS_CTL_ACK_POLICY_MASK 0x0060
#define IEEE80211_QOS_CTL_ACK_POLICY_NOACK 0x0020
#define IEEE80211_SEQ_TO_SN(seq) (((seq) & IEEE80211_SCTL_SEQ) >> 4)
