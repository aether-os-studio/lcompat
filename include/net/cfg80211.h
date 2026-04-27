#pragma once

#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/types.h>

#define IEEE80211_MAX_SSID_LEN 32
#define NL80211_SCAN_FLAG_RANDOM_SN BIT(0)
#define NL80211_SCAN_FLAG_RANDOM_ADDR BIT(1)
#define NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH 1
#define NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW 2
#define NL80211_FEATURE_ACTIVE_MONITOR BIT(1)
#define NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE BIT(2)
#define NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR BIT(3)

enum nl80211_band {
    NL80211_BAND_2GHZ = 0,
    NL80211_BAND_5GHZ = 1,
    NL80211_BAND_60GHZ = 2,
    NL80211_BAND_6GHZ = 3,
    IEEE80211_NUM_BANDS = 4,
};

enum nl80211_iftype {
    NL80211_IFTYPE_ADHOC = 1,
    NL80211_IFTYPE_STATION = 2,
    NL80211_IFTYPE_AP = 3,
    NL80211_IFTYPE_MONITOR = 6,
    NL80211_IFTYPE_MESH_POINT = 7,
    NL80211_IFTYPE_P2P_CLIENT = 8,
    NL80211_IFTYPE_P2P_GO = 9,
    NL80211_IFTYPE_P2P_DEVICE = 10,
};

#define NUM_NL80211_IFTYPES 16

enum nl80211_chan_width {
    NL80211_CHAN_WIDTH_20_NOHT = 0,
    NL80211_CHAN_WIDTH_20 = 1,
    NL80211_CHAN_WIDTH_40 = 2,
    NL80211_CHAN_WIDTH_80 = 3,
    NL80211_CHAN_WIDTH_80P80 = 4,
    NL80211_CHAN_WIDTH_160 = 5,
    NL80211_CHAN_WIDTH_5 = 6,
    NL80211_CHAN_WIDTH_10 = 7,
    NL80211_CHAN_WIDTH_320 = 8,
};

#define NL80211_CHAN_HT20 NL80211_CHAN_WIDTH_20

enum nl80211_dfs_regions {
    NL80211_DFS_UNSET = 0,
    NL80211_DFS_FCC = 1,
    NL80211_DFS_ETSI = 2,
    NL80211_DFS_JP = 3,
};

enum nl80211_dfs_state {
    NL80211_DFS_USABLE = 0,
    NL80211_DFS_UNAVAILABLE = 1,
    NL80211_DFS_AVAILABLE = 2,
};

enum nl80211_cqm_rssi_threshold_event {
    NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW_ENUM =
        NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW,
    NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH_ENUM =
        NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH,
};

enum nl80211_reg_initiator {
    NL80211_REGDOM_SET_BY_CORE = 0,
    NL80211_REGDOM_SET_BY_USER = 1,
    NL80211_REGDOM_SET_BY_DRIVER = 2,
    NL80211_REGDOM_SET_BY_COUNTRY_IE = 3,
};

enum nl80211_ext_feature_index {
    NL80211_EXT_FEATURE_CAN_REPLACE_PTK0 = 0,
    NL80211_EXT_FEATURE_SCAN_RANDOM_SN = 1,
    NL80211_EXT_FEATURE_SET_SCAN_DWELL = 2,
    NL80211_EXT_FEATURE_CQM_RSSI_LIST = 3,
    NL80211_EXT_FEATURE_AIRTIME_FAIRNESS = 4,
    NL80211_EXT_FEATURE_AQL = 5,
    NL80211_EXT_FEATURE_VHT_IBSS = 6,
    NL80211_EXT_FEATURE_BEACON_RATE_LEGACY = 7,
    NL80211_EXT_FEATURE_BEACON_RATE_HT = 8,
    NL80211_EXT_FEATURE_BEACON_RATE_VHT = 9,
    NL80211_EXT_FEATURE_BEACON_RATE_HE = 10,
    NL80211_EXT_FEATURE_ACK_SIGNAL_SUPPORT = 11,
};

#define NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR BIT(0)
#define NL80211_KEK_LEN 16
#define NL80211_KCK_LEN 16
#define NL80211_REPLAY_CTR_LEN 8
#define CFG80211_TESTMODE_CMD(fn)
#define CFG80211_TESTMODE_DUMP(fn)

#define WLAN_EID_DS_PARAMS 3
#define WLAN_EID_HT_OPERATION 61
#define WLAN_EID_VHT_OPERATION 192

#define WIPHY_FLAG_SUPPORTS_TDLS BIT(0)
#define WIPHY_FLAG_TDLS_EXTERNAL_SETUP BIT(1)
#define WIPHY_FLAG_HAS_CHANNEL_SWITCH BIT(2)
#define WIPHY_FLAG_AP_UAPSD BIT(3)
#define WIPHY_FLAG_IBSS_RSN BIT(4)
#define WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL BIT(5)
#define WIPHY_FLAG_4ADDR_AP BIT(6)
#define WIPHY_FLAG_4ADDR_STATION BIT(7)
#define WIPHY_FLAG_SPLIT_SCAN_6GHZ BIT(8)

#define REGULATORY_STRICT_REG BIT(0)
#define REGULATORY_COUNTRY_IE_IGNORE BIT(1)

struct cfg80211_ssid {
    u8 ssid[IEEE80211_MAX_SSID_LEN];
    u8 ssid_len;
};

static inline bool cfg80211_ssid_eq(struct cfg80211_ssid *a,
                                    struct cfg80211_ssid *b) {
    if (!a || !b)
        return false;
    if (a->ssid_len != b->ssid_len)
        return false;
    return memcmp(a->ssid, b->ssid, a->ssid_len) == 0;
}

struct rate_info;
struct ieee80211_hw;

struct cfg80211_bitrate_mask_control {
    u32 legacy;
    u8 ht_mcs[10];
    u16 vht_mcs[8];
    u16 he_mcs[8];
};

struct cfg80211_bitrate_mask {
    struct cfg80211_bitrate_mask_control control[IEEE80211_NUM_BANDS];
};

struct cfg80211_match_set {
    struct cfg80211_ssid ssid;
    s32 rssi_thold;
};

struct cfg80211_sched_scan_plan {
    u32 interval;
    u32 iterations;
};

struct cfg80211_scan_request;
#define cfg80211_scan_ssid cfg80211_ssid

struct cfg80211_scan_6ghz_params {
    u8 channel_idx;
    u8 bssid[ETH_ALEN];
};

struct cfg80211_scan_request {
    struct ieee80211_channel **channels;
    int n_channels;
    struct cfg80211_scan_ssid *ssids;
    int n_ssids;
    const u8 *ie;
    size_t ie_len;
    bool duration_mandatory;
    u16 duration;
    u32 flags;
    bool no_cck;
    u8 mac_addr[6];
    u8 mac_addr_mask[6];
    u8 bssid[6];
    int n_6ghz_params;
    struct cfg80211_scan_6ghz_params *scan_6ghz_params;
};

struct cfg80211_scan_info {
    bool aborted;
};

static inline int cfg80211_get_ies_channel_number(const u8 *ies, size_t len,
                                                  enum nl80211_band band) {
    size_t pos = 0;
    int ht_primary = -1;
    int vht_primary = -1;

    (void)band;
    while (ies && pos + 2 <= len) {
        u8 id = ies[pos];
        u8 elen = ies[pos + 1];
        const u8 *data = &ies[pos + 2];

        pos += 2;
        if (elen > len - pos)
            break;

        if (id == WLAN_EID_DS_PARAMS && elen >= 1)
            return data[0];
        if (id == WLAN_EID_HT_OPERATION && elen >= 1)
            ht_primary = data[0];
        if (id == WLAN_EID_VHT_OPERATION && elen >= 2)
            vht_primary = data[1];

        pos += elen;
    }

    if (ht_primary > 0)
        return ht_primary;
    if (vht_primary > 0)
        return vht_primary;
    return -1;
}

struct cfg80211_chan_def {
    struct ieee80211_channel *chan;
    u32 width;
    u32 center_freq1;
    u32 center_freq2;
};

struct mac_address {
    u8 addr[6];
};

enum environment_cap {
    ENVIRON_ANY = 0,
    ENVIRON_INDOOR = 1,
};

struct cfg80211_pkt_pattern {
    const u8 *mask;
    const u8 *pattern;
    u32 pattern_len;
    u32 pkt_offset;
};

struct cfg80211_wowlan_nd_info;
struct cfg80211_sched_scan_request {
    struct ieee80211_channel **channels;
    int n_channels;
    struct cfg80211_ssid *ssids;
    int n_ssids;
    struct cfg80211_match_set *match_sets;
    int n_match_sets;
    const u8 *ie;
    size_t ie_len;
    u32 flags;
    u8 mac_addr[ETH_ALEN];
    u8 mac_addr_mask[ETH_ALEN];
    u32 delay;
    struct cfg80211_sched_scan_plan *scan_plans;
    int n_scan_plans;
};
struct cfg80211_wowlan {
    struct cfg80211_sched_scan_request *nd_config;
};

struct cfg80211_wowlan_wakeup {
    bool disconnect;
    bool gtk_rekey_failure;
    bool eap_identity_req;
    bool four_way_handshake;
};

struct cfg80211_gtk_rekey_data {
    u8 kek[NL80211_KEK_LEN];
    u8 kck[NL80211_KCK_LEN];
    u8 replay_ctr[NL80211_REPLAY_CTR_LEN];
};

struct cfg80211_sar_specs {
    u32 type;
    u32 num_sub_specs;
    struct {
        u32 freq_range_index;
        s32 power;
    } sub_specs[8];
};

struct cfg80211_sar_freq_ranges {
    u32 start_freq;
    u32 end_freq;
};

struct cfg80211_sar_capa {
    const struct cfg80211_sar_freq_ranges *freq_ranges;
    u32 num_freq_ranges;
    u32 type;
};

#define NL80211_SAR_TYPE_POWER 0

#define NL80211_STA_INFO_TX_BITRATE 0
#define NL80211_STA_INFO_TX_FAILED 1
#define NL80211_STA_INFO_TX_RETRIES 2
#define NL80211_STA_INFO_ACK_SIGNAL 3
#define NL80211_STA_INFO_ACK_SIGNAL_AVG 4

struct regulatory_request {
    char alpha2[2];
    enum nl80211_reg_initiator initiator;
    enum nl80211_dfs_regions dfs_region;
    enum environment_cap country_ie_env;
};

struct wiphy_wowlan_support {
    u32 flags;
};

struct ieee80211_supported_band;

struct wiphy {
    struct device dev;
    struct ieee80211_hw *hw;
    struct ieee80211_supported_band *bands[IEEE80211_NUM_BANDS];
    u32 interface_modes;
    u32 available_antennas_tx;
    u32 available_antennas_rx;
    u32 flags;
    u32 features;
    u32 max_scan_ssids;
    u32 max_scan_ie_len;
    u32 max_remain_on_channel_duration;
    u32 max_sched_scan_plan_interval;
    u32 max_sched_scan_ie_len;
    u32 max_match_sets;
    u32 max_sched_scan_reqs;
    const void *iface_combinations;
    int n_iface_combinations;
    const struct mac_address *addresses;
    int n_addresses;
    int n_radio;
    u32 regulatory_flags;
    char fw_version[32];
    void *debugfsdir;
    u32 rts_threshold;
    const struct wiphy_wowlan_support *wowlan;
    u32 max_sched_scan_ssids;
    u32 mbssid_max_interfaces;
    const struct cfg80211_sar_capa *sar_capa;
    void (*reg_notifier)(struct wiphy *wiphy,
                         struct regulatory_request *request);
};

static inline const char *wiphy_name(struct wiphy *wiphy) {
    return (wiphy && wiphy->dev.kobj_name) ? wiphy->dev.kobj_name : "wiphy";
}

#define wiphy_info(wiphy, fmt, ...)                                            \
    pr_info("%s: " fmt, wiphy_name(wiphy), ##__VA_ARGS__)

static inline void wiphy_ext_feature_set(struct wiphy *wiphy,
                                         enum nl80211_ext_feature_index ext) {
    (void)wiphy;
    (void)ext;
}

static inline void wiphy_rfkill_start_polling(struct wiphy *wiphy) {
    (void)wiphy;
}

static inline void wiphy_rfkill_set_hw_state(struct wiphy *wiphy,
                                             bool blocked) {
    (void)wiphy;
    (void)blocked;
}

static inline int regulatory_hint(struct wiphy *wiphy, const char *alpha2) {
    (void)wiphy;
    (void)alpha2;
    return 0;
}

static inline bool
cfg80211_reg_can_beacon(struct wiphy *wiphy,
                        const struct cfg80211_chan_def *chandef,
                        enum nl80211_iftype iftype) {
    (void)wiphy;
    (void)chandef;
    (void)iftype;
    return true;
}

static inline void cfg80211_chandef_create(struct cfg80211_chan_def *chandef,
                                           struct ieee80211_channel *chan,
                                           u32 width) {
    if (!chandef)
        return;
    chandef->chan = chan;
    chandef->width = width;
    chandef->center_freq1 = 0;
    chandef->center_freq2 = 0;
}

static inline bool
cfg80211_chandef_valid(const struct cfg80211_chan_def *chandef) {
    return chandef && chandef->chan;
}

static inline bool
cfg80211_chandef_identical(const struct cfg80211_chan_def *a,
                           const struct cfg80211_chan_def *b) {
    if (!a || !b)
        return false;
    return a->chan == b->chan && a->width == b->width &&
           a->center_freq1 == b->center_freq1 &&
           a->center_freq2 == b->center_freq2;
}
