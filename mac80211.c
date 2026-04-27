#include "lcompat_native.h"
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <net/mac80211.h>
#include <net/netdev.h>

#define LCOMPAT_IEEE80211_MTU 1500
#define LCOMPAT_IEEE80211_RTS_THRESHOLD 2347
#define LCOMPAT_LLC_SNAP_LEN 8
#define LCOMPAT_WLAN_CAPABILITY_ESS BIT(0)
#define LCOMPAT_WLAN_CAPABILITY_PRIVACY BIT(4)
#define LCOMPAT_WLAN_CAPABILITY_SHORT_SLOT_TIME BIT(10)
#define LCOMPAT_WLAN_EID_SSID 0
#define LCOMPAT_WLAN_EID_SUPP_RATES 1
#define LCOMPAT_WLAN_EID_DS_PARAMS 3
#define LCOMPAT_WLAN_EID_EXT_SUPP_RATES 50
#define LCOMPAT_WLAN_EID_HT_CAPABILITY 45
#define LCOMPAT_WLAN_EID_VHT_CAPABILITY 191
#define LCOMPAT_WLAN_EID_VENDOR_SPECIFIC 221
#define LCOMPAT_WLAN_AUTH_OPEN 0
#define LCOMPAT_WLAN_STATUS_SUCCESS 0
#define LCOMPAT_WLAN_REASON_DEAUTH_LEAVING 3
#define LCOMPAT_NL80211_AUTHTYPE_OPEN_SYSTEM 0
#define LCOMPAT_NL80211_AUTHTYPE_AUTOMATIC 8
#define LCOMPAT_IEEE80211_FTYPE_MGMT 0x0000
#define LCOMPAT_IEEE80211_STYPE_ASSOC_REQ 0x0000
#define LCOMPAT_IEEE80211_STYPE_ASSOC_RESP 0x0010
#define LCOMPAT_IEEE80211_STYPE_PROBE_REQ 0x0040
#define LCOMPAT_IEEE80211_STYPE_DISASSOC 0x00A0
#define LCOMPAT_IEEE80211_STYPE_AUTH 0x00B0
#define LCOMPAT_IEEE80211_STYPE_DEAUTH 0x00C0
#define LCOMPAT_IEEE80211_LISTEN_INTERVAL 10
#define LCOMPAT_SW_SCAN_CHANNEL_DWELL_MS 60
#define LCOMPAT_IEEE80211_QOS_CTL_A_MSDU_PRESENT BIT(7)
#define LCOMPAT_ETH_P_AARP 0x80F3
#define LCOMPAT_ETH_P_IPX 0x8137
#define LCOMPAT_WMM_IE_LEN 9

#define IEEE80211_FCTL_TODS cpu_to_le16(BIT(8))
#define IEEE80211_FCTL_FROMDS cpu_to_le16(BIT(9))
#define IEEE80211_STYPE_QOS_DATA 0x0080
#define LCOMPAT_IEEE80211_FCTL_PROTECTED BIT(14)
#define LCOMPAT_IEEE80211_FCTL_ORDER BIT(15)

struct lcompat_ethhdr {
    u8 h_dest[ETH_ALEN];
    u8 h_source[ETH_ALEN];
    __be16 h_proto;
} __attribute__((packed));

enum lcompat_ieee80211_conn_state {
    LCOMPAT_CONN_DISCONNECTED = 0,
    LCOMPAT_CONN_AUTHENTICATING,
    LCOMPAT_CONN_ASSOCIATING,
    LCOMPAT_CONN_CONNECTED,
};

typedef struct lcompat_ieee80211_runtime {
    struct ieee80211_hw *hw;
    struct ieee80211_vif *vif;
    struct ieee80211_txq *txq;
    struct ieee80211_scan_request *scan_req;
    struct ieee80211_sta *sta;
    netdev_t *netdev;
    struct sk_buff_head rx_queue;
    struct list_head list;
    spinlock_t lock;
    enum lcompat_ieee80211_conn_state conn_state;
    netdev_connect_params_t connect_params;
    uint32_t connect_request_portid;
    uint16_t beacon_interval;
    uint16_t capability;
    u16 tx_seq;
    unsigned int scan_filter_flags;
    bool use_qos;
    bool vif_added;
    bool started;
    bool scan_filter_active;
} lcompat_ieee80211_runtime_t;

struct lcompat_ieee80211_auth_frame {
    __le16 frame_control;
    __le16 duration;
    u8 da[ETH_ALEN];
    u8 sa[ETH_ALEN];
    u8 bssid[ETH_ALEN];
    __le16 seq_ctrl;
    __le16 auth_alg;
    __le16 auth_transaction;
    __le16 status_code;
} __packed;

struct lcompat_ieee80211_assoc_req_frame {
    __le16 frame_control;
    __le16 duration;
    u8 da[ETH_ALEN];
    u8 sa[ETH_ALEN];
    u8 bssid[ETH_ALEN];
    __le16 seq_ctrl;
    __le16 capab_info;
    __le16 listen_interval;
    u8 variable[];
} __packed;

struct lcompat_ieee80211_reason_frame {
    __le16 frame_control;
    __le16 duration;
    u8 da[ETH_ALEN];
    u8 sa[ETH_ALEN];
    u8 bssid[ETH_ALEN];
    __le16 seq_ctrl;
    __le16 reason_code;
} __packed;

static LIST_HEAD(lcompat_ieee80211_runtimes);
static spinlock_t lcompat_ieee80211_runtimes_lock = SPIN_INIT;

struct lcompat_tx_status_work {
    struct work_struct work;
    struct ieee80211_hw *hw;
    struct sk_buff *skb;
};

void netlink_publish_connect_result(struct netdev *dev, uint32_t request_portid,
                                    uint16_t status_code);
void netlink_publish_disconnect_event(struct netdev *dev,
                                      uint32_t request_portid,
                                      uint16_t reason_code, bool by_ap);

static void lcompat_ieee80211_tx_status_work(struct work_struct *work) {
    struct lcompat_tx_status_work *txs =
        container_of(work, struct lcompat_tx_status_work, work);

    if (!txs)
        return;

    /*
     * Linux hands tx status ownership back to mac80211 asynchronously.
     * The compat layer still only needs the terminal ownership transfer,
     * but it must not happen in the caller's immediate context.
     */
    if (txs->skb)
        dev_kfree_skb_any(txs->skb);

    kfree(txs);
}

static const u8 lcompat_rfc1042_header[6] = {0xaa, 0xaa, 0x03,
                                             0x00, 0x00, 0x00};
static const u8 lcompat_bridge_tunnel_header[6] = {0xaa, 0xaa, 0x03,
                                                   0x00, 0x00, 0xf8};
static const u8 lcompat_wmm_info_ie[LCOMPAT_WMM_IE_LEN] = {
    LCOMPAT_WLAN_EID_VENDOR_SPECIFIC,
    7,
    0x00,
    0x50,
    0xf2,
    0x02,
    0x00,
    0x01,
    0x00};

static bool lcompat_ieee80211_snap_to_ethertype(const u8 *payload,
                                                u32 payload_len, __be16 *proto,
                                                u32 *skip);

static u32 lcompat_ieee80211_data_hdr_len(__le16 fc) {
    u16 fc_cpu = le16_to_cpu(fc);
    u32 hdr_len = sizeof(struct ieee80211_hdr);

    if (ieee80211_has_tods(fc) && ieee80211_has_fromds(fc))
        hdr_len += ETH_ALEN;
    if (ieee80211_is_data_qos(fc)) {
        hdr_len += 2;
        if (fc_cpu & LCOMPAT_IEEE80211_FCTL_ORDER)
            hdr_len += 4;
    }

    return hdr_len;
}

static inline lcompat_ieee80211_runtime_t *
lcompat_ieee80211_runtime(struct ieee80211_hw *hw) {
    return hw ? (lcompat_ieee80211_runtime_t *)hw->lcompat_runtime : NULL;
}

static bool lcompat_size_add(size_t a, size_t b, size_t *out) {
    if (a > SIZE_MAX - b)
        return true;
    *out = a + b;
    return false;
}

static bool lcompat_size_add_ie(size_t *total, size_t len) {
    size_t ie_len;

    if (len > U8_MAX)
        return true;
    if (lcompat_size_add(2, len, &ie_len))
        return true;
    return lcompat_size_add(*total, ie_len, total);
}

static void
lcompat_ieee80211_fill_wireless_info(lcompat_ieee80211_runtime_t *rt,
                                     netdev_wireless_info_t *info);

static int
lcompat_ieee80211_update_wireless_info(lcompat_ieee80211_runtime_t *rt) {
    netdev_wireless_info_t info;

    if (!rt || !rt->netdev)
        return -EINVAL;

    lcompat_ieee80211_fill_wireless_info(rt, &info);
    return netdev_set_wireless_info(rt->netdev, &info);
}

static struct sk_buff *
lcompat_ieee80211_alloc_tx_skb(lcompat_ieee80211_runtime_t *rt,
                               size_t payload_len) {
    struct sk_buff *skb;
    size_t headroom = 0;
    size_t skb_len;

    if (!rt || !rt->hw)
        return NULL;

    if (rt->hw->extra_tx_headroom < 0)
        return NULL;
    if (rt->hw->extra_tx_headroom > 0)
        headroom = (size_t)rt->hw->extra_tx_headroom;
    if (headroom > UINT_MAX ||
        lcompat_size_add(headroom, payload_len, &skb_len) || skb_len > UINT_MAX)
        return NULL;

    skb = alloc_skb(skb_len, GFP_KERNEL);
    if (!skb)
        return NULL;

    if (headroom)
        skb_reserve(skb, (unsigned int)headroom);

    return skb;
}

static void
lcompat_ieee80211_configure_scan_filter(lcompat_ieee80211_runtime_t *rt,
                                        bool enable) {
    unsigned int changed_flags;
    unsigned int new_flags;

    if (!rt || !rt->hw || !rt->hw->ops || !rt->hw->ops->configure_filter)
        return;

    changed_flags = FIF_OTHER_BSS | FIF_BCN_PRBRESP_PROMISC;
    new_flags = enable ? changed_flags : 0;
    rt->hw->ops->configure_filter(rt->hw, changed_flags, &new_flags, 0);
    rt->scan_filter_flags = new_flags;
    rt->scan_filter_active = enable;
}

static void
lcompat_ieee80211_clear_connect_params(lcompat_ieee80211_runtime_t *rt) {
    if (!rt)
        return;

    memset(&rt->connect_params, 0, sizeof(rt->connect_params));
    rt->connect_request_portid = 0;
    rt->beacon_interval = 0;
    rt->capability = 0;
    rt->use_qos = false;
}

static struct ieee80211_channel *
lcompat_ieee80211_find_channel_by_freq(struct ieee80211_hw *hw,
                                       uint32_t frequency) {
    int band;
    int i;

    if (!hw || !hw->wiphy || !frequency)
        return NULL;

    for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
        struct ieee80211_supported_band *sband = hw->wiphy->bands[band];

        if (!sband || !sband->channels)
            continue;

        for (i = 0; i < sband->n_channels; i++) {
            if ((uint32_t)sband->channels[i].center_freq == frequency)
                return &sband->channels[i];
        }
    }

    return NULL;
}

static int lcompat_ieee80211_set_channel(lcompat_ieee80211_runtime_t *rt,
                                         uint32_t frequency) {
    struct ieee80211_channel *chan;

    if (!rt || !rt->hw)
        return -EINVAL;
    if (!frequency)
        return 0;

    chan = lcompat_ieee80211_find_channel_by_freq(rt->hw, frequency);
    if (!chan)
        return -ENOENT;
    if (rt->hw->conf.chandef.chan == chan &&
        rt->hw->conf.chandef.center_freq1 == frequency)
        return 0;

    rt->hw->conf.chandef.chan = chan;
    rt->hw->conf.chandef.width = NL80211_CHAN_WIDTH_20;
    rt->hw->conf.chandef.center_freq1 = chan->center_freq;
    rt->hw->conf.chandef.center_freq2 = 0;

    if (rt->hw->ops && rt->hw->ops->config)
        return rt->hw->ops->config(rt->hw, -1, IEEE80211_CONF_CHANGE_CHANNEL);

    return 0;
}

static const u8 *lcompat_ieee80211_find_ie(const u8 *ies, size_t len, u8 eid,
                                           u8 *out_len) {
    size_t pos = 0;

    while (ies && pos + 2 <= len) {
        u8 id = ies[pos];
        u8 ie_len = ies[pos + 1];

        pos += 2;
        if (pos + ie_len > len)
            break;
        if (id == eid) {
            if (out_len)
                *out_len = ie_len;
            return ies + pos;
        }
        pos += ie_len;
    }

    return NULL;
}

static bool lcompat_ieee80211_bss_has_wmm(const netdev_scan_result_t *bss) {
    size_t pos = 0;

    while (bss && pos + 2 <= bss->ie_len) {
        const u8 *ie = bss->ies + pos + 2;
        u8 id = bss->ies[pos];
        u8 ie_len = bss->ies[pos + 1];

        pos += 2;
        if (pos + ie_len > bss->ie_len)
            break;
        if (id == LCOMPAT_WLAN_EID_VENDOR_SPECIFIC && ie_len >= 6 &&
            ie[0] == 0x00 && ie[1] == 0x50 && ie[2] == 0xf2 && ie[3] == 0x02 &&
            (ie[4] == 0x00 || ie[4] == 0x01) && ie[5] == 0x01)
            return true;
        pos += ie_len;
    }

    return false;
}

static bool
lcompat_ieee80211_bss_privacy_matches(const netdev_scan_result_t *bss,
                                      const netdev_connect_params_t *params) {
    bool bss_privacy;

    if (!bss || !params)
        return false;

    bss_privacy = !!(bss->capability & LCOMPAT_WLAN_CAPABILITY_PRIVACY);
    if (bss_privacy != params->privacy)
        return false;

    return true;
}

static bool lcompat_ieee80211_ssid_matches(const netdev_scan_result_t *result,
                                           const u8 *ssid, u8 ssid_len) {
    const u8 *ie;
    u8 ie_len = 0;

    if (!result || !ssid)
        return false;

    ie = lcompat_ieee80211_find_ie(result->ies, result->ie_len,
                                   LCOMPAT_WLAN_EID_SSID, &ie_len);
    if (!ie || ie_len != ssid_len)
        return false;

    return memcmp(ie, ssid, ssid_len) == 0;
}

static void lcompat_ieee80211_fill_scan_ssid(const netdev_scan_result_t *result,
                                             netdev_connect_params_t *params) {
    const u8 *ie;
    u8 ie_len = 0;

    if (!result || !params || params->ssid_len)
        return;

    ie = lcompat_ieee80211_find_ie(result->ies, result->ie_len,
                                   LCOMPAT_WLAN_EID_SSID, &ie_len);
    if (!ie || !ie_len)
        return;

    if (ie_len > sizeof(params->ssid))
        ie_len = sizeof(params->ssid);
    memcpy(params->ssid, ie, ie_len);
    params->ssid_len = ie_len;
}

static int lcompat_ieee80211_select_bss(lcompat_ieee80211_runtime_t *rt,
                                        netdev_connect_params_t *params,
                                        netdev_scan_result_t *selected) {
    netdev_scan_state_t scan;
    uint32_t i;

    if (!rt || !rt->netdev || !params || !selected)
        return -EINVAL;

    memset(selected, 0, sizeof(*selected));
    if (!netdev_get_scan_state(rt->netdev, &scan)) {
        if (params->has_bssid && params->frequency) {
            selected->valid = true;
            selected->frequency = params->frequency;
            memcpy(selected->bssid, params->bssid, sizeof(selected->bssid));
            return 0;
        }
        return -ENOENT;
    }

    for (i = 0; i < NETDEV_MAX_SCAN_RESULTS; i++) {
        if (!scan.results[i].valid)
            continue;
        if (params->has_bssid && memcmp(scan.results[i].bssid, params->bssid,
                                        sizeof(params->bssid)) != 0)
            continue;
        if (params->ssid_len &&
            !lcompat_ieee80211_ssid_matches(&scan.results[i], params->ssid,
                                            params->ssid_len))
            continue;
        if (!lcompat_ieee80211_bss_privacy_matches(&scan.results[i], params))
            continue;
        *selected = scan.results[i];
        lcompat_ieee80211_fill_scan_ssid(selected, params);
        return 0;
    }

    for (i = 0; i < NETDEV_MAX_SCAN_RESULTS; i++) {
        if (!scan.results[i].valid)
            continue;
        if (params->ssid_len &&
            lcompat_ieee80211_ssid_matches(&scan.results[i], params->ssid,
                                           params->ssid_len)) {
            if (!lcompat_ieee80211_bss_privacy_matches(&scan.results[i],
                                                       params))
                continue;
            *selected = scan.results[i];
            if (!params->has_bssid) {
                memcpy(params->bssid, selected->bssid, sizeof(params->bssid));
                params->has_bssid = true;
            }
            return 0;
        }
    }

    return -ENOENT;
}

static u16 lcompat_ieee80211_next_seq_ctrl(lcompat_ieee80211_runtime_t *rt) {
    return cpu_to_le16((rt->tx_seq++ & 0x0fffU) << 4);
}

static int lcompat_ieee80211_send_mgmt(lcompat_ieee80211_runtime_t *rt,
                                       struct sk_buff *skb) {
    struct ieee80211_tx_control control = {0};
    struct ieee80211_tx_info *info;

    if (!rt || !rt->hw || !rt->hw->ops || !rt->hw->ops->tx || !skb)
        return -EINVAL;

    info = IEEE80211_SKB_CB(skb);
    memset(info, 0, sizeof(*info));
    info->control.vif = rt->vif;
    info->flags |= IEEE80211_TX_CTL_REQ_TX_STATUS;

    if (rt->hw->ops->mgd_prepare_tx) {
        struct ieee80211_prep_tx_info prep = {0};

        rt->hw->ops->mgd_prepare_tx(rt->hw, rt->vif, &prep);
    }

    rt->hw->ops->tx(rt->hw, &control, skb);
    return 0;
}

static int lcompat_ieee80211_send_auth(lcompat_ieee80211_runtime_t *rt) {
    struct lcompat_ieee80211_auth_frame *auth;
    struct sk_buff *skb;

    if (!rt || !rt->connect_params.has_bssid)
        return -EINVAL;

    skb = lcompat_ieee80211_alloc_tx_skb(rt, sizeof(*auth));
    if (!skb)
        return -ENOMEM;

    auth = skb_put_zero(skb, sizeof(*auth));
    if (!auth) {
        dev_kfree_skb_any(skb);
        return -ENOMEM;
    }
    auth->frame_control = cpu_to_le16(LCOMPAT_IEEE80211_FTYPE_MGMT |
                                      LCOMPAT_IEEE80211_STYPE_AUTH);
    memcpy(auth->da, rt->connect_params.bssid, ETH_ALEN);
    memcpy(auth->sa, rt->vif->addr, ETH_ALEN);
    memcpy(auth->bssid, rt->connect_params.bssid, ETH_ALEN);
    auth->seq_ctrl = lcompat_ieee80211_next_seq_ctrl(rt);
    auth->auth_alg = cpu_to_le16(LCOMPAT_WLAN_AUTH_OPEN);
    auth->auth_transaction = cpu_to_le16(1);
    auth->status_code = cpu_to_le16(LCOMPAT_WLAN_STATUS_SUCCESS);
    return lcompat_ieee80211_send_mgmt(rt, skb);
}

static size_t lcompat_ieee80211_build_supported_rates_ie(
    lcompat_ieee80211_runtime_t *rt, const netdev_scan_result_t *bss,
    u8 *supp_rates, size_t supp_rates_size, u8 *ext_rates,
    size_t ext_rates_size) {
    struct ieee80211_supported_band *sband;
    const u8 *scan_rates;
    const u8 *scan_ext_rates;
    u8 scan_rates_len = 0;
    u8 scan_ext_rates_len = 0;
    enum nl80211_band band;
    size_t supp_count = 0;
    size_t ext_count = 0;
    int i;

    if (!rt || !rt->hw || !rt->hw->conf.chandef.chan)
        return 0;

    band = rt->hw->conf.chandef.chan->band;
    sband = rt->hw->wiphy->bands[band];
    if (!sband || !sband->bitrates)
        return 0;

    scan_rates = lcompat_ieee80211_find_ie(
        bss->ies, bss->ie_len, LCOMPAT_WLAN_EID_SUPP_RATES, &scan_rates_len);
    scan_ext_rates = lcompat_ieee80211_find_ie(bss->ies, bss->ie_len,
                                               LCOMPAT_WLAN_EID_EXT_SUPP_RATES,
                                               &scan_ext_rates_len);

    for (i = 0; i < sband->n_bitrates; i++) {
        u8 rate = (u8)(sband->bitrates[i].bitrate / 5);
        bool allowed = true;
        bool basic = false;
        u8 j;

        if (scan_rates || scan_ext_rates) {
            allowed = false;
            for (j = 0; scan_rates && j < scan_rates_len; j++) {
                if ((scan_rates[j] & 0x7f) == rate) {
                    allowed = true;
                    basic = !!(scan_rates[j] & 0x80);
                    break;
                }
            }
            for (j = 0; !allowed && scan_ext_rates && j < scan_ext_rates_len;
                 j++) {
                if ((scan_ext_rates[j] & 0x7f) == rate) {
                    allowed = true;
                    basic = !!(scan_ext_rates[j] & 0x80);
                    break;
                }
            }
        }

        if (!allowed)
            continue;

        if (supp_count < supp_rates_size) {
            supp_rates[supp_count++] = rate | (basic ? 0x80 : 0);
        } else if (ext_count < ext_rates_size) {
            ext_rates[ext_count++] = rate | (basic ? 0x80 : 0);
        }
    }

    return supp_count + ext_count;
}

static u32 lcompat_ieee80211_basic_rates_mask(lcompat_ieee80211_runtime_t *rt,
                                              const netdev_scan_result_t *bss) {
    struct ieee80211_supported_band *sband;
    const u8 *scan_rates;
    const u8 *scan_ext_rates;
    u8 scan_rates_len = 0;
    u8 scan_ext_rates_len = 0;
    u32 mask = 0;
    int i;
    int j;

    if (!rt || !rt->hw || !rt->hw->conf.chandef.chan)
        return 0;

    sband = rt->hw->wiphy->bands[rt->hw->conf.chandef.chan->band];
    if (!sband || !sband->bitrates)
        return 0;

    scan_rates = lcompat_ieee80211_find_ie(
        bss->ies, bss->ie_len, LCOMPAT_WLAN_EID_SUPP_RATES, &scan_rates_len);
    scan_ext_rates = lcompat_ieee80211_find_ie(bss->ies, bss->ie_len,
                                               LCOMPAT_WLAN_EID_EXT_SUPP_RATES,
                                               &scan_ext_rates_len);

    for (i = 0; i < sband->n_bitrates && i < 32; i++) {
        u8 rate = (u8)(sband->bitrates[i].bitrate / 5);

        for (j = 0; scan_rates && j < scan_rates_len; j++) {
            if ((scan_rates[j] & 0x7f) == rate && (scan_rates[j] & 0x80))
                mask |= BIT(i);
        }
        for (j = 0; scan_ext_rates && j < scan_ext_rates_len; j++) {
            if ((scan_ext_rates[j] & 0x7f) == rate &&
                (scan_ext_rates[j] & 0x80))
                mask |= BIT(i);
        }
    }

    return mask;
}

static u32
lcompat_ieee80211_supported_rates_mask(lcompat_ieee80211_runtime_t *rt,
                                       const netdev_scan_result_t *bss) {
    struct ieee80211_supported_band *sband;
    const u8 *scan_rates;
    const u8 *scan_ext_rates;
    u8 scan_rates_len = 0;
    u8 scan_ext_rates_len = 0;
    u32 mask = 0;
    int i;
    int j;

    if (!rt || !rt->hw || !rt->hw->conf.chandef.chan)
        return 0;

    sband = rt->hw->wiphy->bands[rt->hw->conf.chandef.chan->band];
    if (!sband || !sband->bitrates)
        return 0;
    if (!bss || !bss->ies || !bss->ie_len)
        return sband->n_bitrates >= 32 ? U32_MAX : (BIT(sband->n_bitrates) - 1);

    scan_rates = lcompat_ieee80211_find_ie(
        bss->ies, bss->ie_len, LCOMPAT_WLAN_EID_SUPP_RATES, &scan_rates_len);
    scan_ext_rates = lcompat_ieee80211_find_ie(bss->ies, bss->ie_len,
                                               LCOMPAT_WLAN_EID_EXT_SUPP_RATES,
                                               &scan_ext_rates_len);

    for (i = 0; i < sband->n_bitrates && i < 32; i++) {
        u8 rate = (u8)(sband->bitrates[i].bitrate / 5);

        for (j = 0; scan_rates && j < scan_rates_len; j++) {
            if ((scan_rates[j] & 0x7f) == rate)
                mask |= BIT(i);
        }
        for (j = 0; scan_ext_rates && j < scan_ext_rates_len; j++) {
            if ((scan_ext_rates[j] & 0x7f) == rate)
                mask |= BIT(i);
        }
    }

    return mask;
}

static int lcompat_ieee80211_send_assoc_req(lcompat_ieee80211_runtime_t *rt,
                                            const netdev_scan_result_t *bss) {
    struct lcompat_ieee80211_assoc_req_frame *assoc;
    struct ieee80211_supported_band *sband;
    struct sk_buff *skb;
    u8 supp_rates[8] = {0};
    u8 ext_rates[16] = {0};
    u8 *ies;
    size_t total_len;
    size_t supp_count = 0;
    size_t ext_count = 0;
    size_t pos = 0;
    u16 capability;

    if (!rt || !rt->connect_params.has_bssid || !bss)
        return -EINVAL;

    sband = rt->hw->wiphy->bands[rt->hw->conf.chandef.chan->band];
    if (!sband)
        return -EINVAL;

    lcompat_ieee80211_build_supported_rates_ie(
        rt, bss, supp_rates, sizeof(supp_rates), ext_rates, sizeof(ext_rates));
    while (supp_count < sizeof(supp_rates) && supp_rates[supp_count])
        supp_count++;
    while (ext_count < sizeof(ext_rates) && ext_rates[ext_count])
        ext_count++;

    total_len = sizeof(*assoc);
    if (lcompat_size_add_ie(&total_len, rt->connect_params.ssid_len))
        return -EOVERFLOW;
    if (supp_count && lcompat_size_add_ie(&total_len, supp_count))
        return -EOVERFLOW;
    if (ext_count && lcompat_size_add_ie(&total_len, ext_count))
        return -EOVERFLOW;
    if (lcompat_ieee80211_bss_has_wmm(bss) &&
        lcompat_size_add(total_len, LCOMPAT_WMM_IE_LEN, &total_len))
        return -EOVERFLOW;
    if (total_len > UINT_MAX)
        return -EOVERFLOW;

    skb = lcompat_ieee80211_alloc_tx_skb(rt, total_len);
    if (!skb)
        return -ENOMEM;

    assoc = skb_put_zero(skb, sizeof(*assoc));
    if (!assoc) {
        dev_kfree_skb_any(skb);
        return -ENOMEM;
    }
    assoc->frame_control = cpu_to_le16(LCOMPAT_IEEE80211_FTYPE_MGMT |
                                       LCOMPAT_IEEE80211_STYPE_ASSOC_REQ);
    memcpy(assoc->da, rt->connect_params.bssid, ETH_ALEN);
    memcpy(assoc->sa, rt->vif->addr, ETH_ALEN);
    memcpy(assoc->bssid, rt->connect_params.bssid, ETH_ALEN);
    assoc->seq_ctrl = lcompat_ieee80211_next_seq_ctrl(rt);
    capability = LCOMPAT_WLAN_CAPABILITY_ESS;
    if (rt->connect_params.privacy)
        capability |= LCOMPAT_WLAN_CAPABILITY_PRIVACY;
    if (rt->capability & LCOMPAT_WLAN_CAPABILITY_SHORT_SLOT_TIME)
        capability |= LCOMPAT_WLAN_CAPABILITY_SHORT_SLOT_TIME;
    assoc->capab_info = cpu_to_le16(capability);
    assoc->listen_interval = cpu_to_le16(LCOMPAT_IEEE80211_LISTEN_INTERVAL);

    ies = skb_put(skb, (unsigned int)(total_len - sizeof(*assoc)));
    if (!ies) {
        dev_kfree_skb_any(skb);
        return -ENOMEM;
    }

    pos = 0;
    ies[pos++] = LCOMPAT_WLAN_EID_SSID;
    ies[pos++] = rt->connect_params.ssid_len;
    memcpy(ies + pos, rt->connect_params.ssid, rt->connect_params.ssid_len);
    pos += rt->connect_params.ssid_len;

    if (supp_count) {
        ies[pos++] = LCOMPAT_WLAN_EID_SUPP_RATES;
        ies[pos++] = (u8)supp_count;
        memcpy(ies + pos, supp_rates, supp_count);
        pos += supp_count;
    }

    if (ext_count) {
        ies[pos++] = LCOMPAT_WLAN_EID_EXT_SUPP_RATES;
        ies[pos++] = (u8)ext_count;
        memcpy(ies + pos, ext_rates, ext_count);
        pos += ext_count;
    }

    if (lcompat_ieee80211_bss_has_wmm(bss)) {
        memcpy(ies + pos, lcompat_wmm_info_ie, sizeof(lcompat_wmm_info_ie));
        pos += sizeof(lcompat_wmm_info_ie);
    }

    if (pos != total_len - sizeof(*assoc)) {
        dev_kfree_skb_any(skb);
        return -EINVAL;
    }

    return lcompat_ieee80211_send_mgmt(rt, skb);
}

static int lcompat_ieee80211_send_probe_req(lcompat_ieee80211_runtime_t *rt,
                                            const u8 *ssid, u8 ssid_len) {
    struct ieee80211_supported_band *sband;
    struct ieee80211_hdr *hdr;
    struct sk_buff *skb;
    u8 *ies;
    u8 rates[16] = {0};
    size_t rates_count = 0;
    size_t frame_len;
    size_t pos;
    int i;

    if (!rt || !rt->hw || !rt->vif || !rt->hw->conf.chandef.chan)
        return -EINVAL;

    sband = rt->hw->wiphy->bands[rt->hw->conf.chandef.chan->band];
    if (!sband)
        return -EINVAL;

    for (i = 0; i < sband->n_bitrates && rates_count < sizeof(rates); i++)
        rates[rates_count++] = (u8)(sband->bitrates[i].bitrate / 5);

    frame_len = sizeof(*hdr);
    if (lcompat_size_add_ie(&frame_len, ssid_len))
        return -EOVERFLOW;
    if (rates_count) {
        if (lcompat_size_add_ie(&frame_len, MIN(rates_count, (size_t)8)))
            return -EOVERFLOW;
        if (rates_count > 8 && lcompat_size_add_ie(&frame_len, rates_count - 8))
            return -EOVERFLOW;
    }
    if (frame_len > UINT_MAX)
        return -EOVERFLOW;

    skb = lcompat_ieee80211_alloc_tx_skb(rt, frame_len);
    if (!skb)
        return -ENOMEM;

    hdr = skb_put_zero(skb, sizeof(*hdr));
    if (!hdr) {
        dev_kfree_skb_any(skb);
        return -ENOMEM;
    }
    hdr->frame_control = cpu_to_le16(LCOMPAT_IEEE80211_FTYPE_MGMT |
                                     LCOMPAT_IEEE80211_STYPE_PROBE_REQ);
    eth_broadcast_addr(hdr->addr1);
    memcpy(hdr->addr2, rt->vif->addr, ETH_ALEN);
    eth_broadcast_addr(hdr->addr3);
    hdr->seq_ctrl = lcompat_ieee80211_next_seq_ctrl(rt);

    ies = skb_put(skb, (unsigned int)(frame_len - sizeof(*hdr)));
    if (!ies) {
        dev_kfree_skb_any(skb);
        return -ENOMEM;
    }

    pos = 0;
    ies[pos++] = LCOMPAT_WLAN_EID_SSID;
    ies[pos++] = ssid_len;
    if (ssid_len) {
        memcpy(ies + pos, ssid, ssid_len);
        pos += ssid_len;
    }

    if (rates_count) {
        size_t supp_count = MIN(rates_count, (size_t)8);

        ies[pos++] = LCOMPAT_WLAN_EID_SUPP_RATES;
        ies[pos++] = (u8)supp_count;
        memcpy(ies + pos, rates, supp_count);
        pos += supp_count;

        if (rates_count > supp_count) {
            ies[pos++] = LCOMPAT_WLAN_EID_EXT_SUPP_RATES;
            ies[pos++] = (u8)(rates_count - supp_count);
            memcpy(ies + pos, rates + supp_count, rates_count - supp_count);
            pos += rates_count - supp_count;
        }
    }

    if (pos != frame_len - sizeof(*hdr)) {
        dev_kfree_skb_any(skb);
        return -EINVAL;
    }
    return lcompat_ieee80211_send_mgmt(rt, skb);
}

static int lcompat_ieee80211_run_sw_scan(lcompat_ieee80211_runtime_t *rt) {
    struct ieee80211_channel *orig_chan;
    u32 orig_center_freq1;
    u32 orig_center_freq2;
    u32 orig_width;
    int ret = 0;
    int ch;

    if (!rt || !rt->scan_req || !rt->hw || !rt->vif)
        return -EINVAL;
    if (!rt->hw->ops || !rt->hw->ops->sw_scan_start ||
        !rt->hw->ops->sw_scan_complete)
        return -EOPNOTSUPP;

    orig_chan = rt->hw->conf.chandef.chan;
    orig_center_freq1 = rt->hw->conf.chandef.center_freq1;
    orig_center_freq2 = rt->hw->conf.chandef.center_freq2;
    orig_width = rt->hw->conf.chandef.width;

    rt->hw->ops->sw_scan_start(rt->hw, rt->vif, rt->vif->addr);

    for (ch = 0; ch < rt->scan_req->req.n_channels; ch++) {
        struct ieee80211_channel *chan = rt->scan_req->req.channels[ch];
        int s;

        if (!chan)
            continue;

        ret = lcompat_ieee80211_set_channel(rt, (uint32_t)chan->center_freq);
        if (ret)
            break;

        if (rt->scan_req->req.n_ssids <= 0) {
            ret = lcompat_ieee80211_send_probe_req(rt, NULL, 0);
            if (ret)
                break;
        } else {
            for (s = 0; s < rt->scan_req->req.n_ssids; s++) {
                struct cfg80211_scan_ssid *ssid = &rt->scan_req->req.ssids[s];

                ret = lcompat_ieee80211_send_probe_req(rt, ssid->ssid,
                                                       ssid->ssid_len);
                if (ret)
                    break;
            }
            if (ret)
                break;
        }

        msleep(LCOMPAT_SW_SCAN_CHANNEL_DWELL_MS);
    }

    rt->hw->conf.chandef.chan = orig_chan;
    rt->hw->conf.chandef.center_freq1 = orig_center_freq1;
    rt->hw->conf.chandef.center_freq2 = orig_center_freq2;
    rt->hw->conf.chandef.width = orig_width;
    if (rt->hw->ops->config)
        (void)rt->hw->ops->config(rt->hw, -1, IEEE80211_CONF_CHANGE_CHANNEL);

    rt->hw->ops->sw_scan_complete(rt->hw, rt->vif);
    return ret;
}

static void lcompat_ieee80211_free_sta(lcompat_ieee80211_runtime_t *rt) {
    int i;

    if (!rt || !rt->sta)
        return;

    for (i = 0; i < IEEE80211_NUM_TIDS; i++) {
        kfree(rt->sta->txq[i]);
        rt->sta->txq[i] = NULL;
    }
    kfree(rt->sta);
    rt->sta = NULL;
}

static int lcompat_ieee80211_alloc_sta(lcompat_ieee80211_runtime_t *rt,
                                       const netdev_scan_result_t *bss) {
    struct ieee80211_supported_band *sband;
    struct ieee80211_sta *sta;
    size_t sta_size;
    size_t txq_size;
    int i;

    if (!rt || !rt->hw || !rt->connect_params.has_bssid)
        return -EINVAL;
    if (rt->hw->sta_data_size < 0 || rt->hw->txq_data_size < 0)
        return -EINVAL;

    if (lcompat_size_add(sizeof(*sta), (size_t)rt->hw->sta_data_size,
                         &sta_size) ||
        lcompat_size_add(sizeof(struct ieee80211_txq),
                         (size_t)rt->hw->txq_data_size, &txq_size))
        return -EOVERFLOW;
    sta = kzalloc(sta_size, GFP_KERNEL);
    if (!sta)
        return -ENOMEM;

    for (i = 0; i < IEEE80211_NUM_TIDS; i++) {
        sta->txq[i] = kzalloc(txq_size, GFP_KERNEL);
        if (!sta->txq[i]) {
            while (--i >= 0) {
                kfree(sta->txq[i]);
                sta->txq[i] = NULL;
            }
            kfree(sta);
            return -ENOMEM;
        }
        sta->txq[i]->sta = sta;
        sta->txq[i]->vif = rt->vif;
        sta->txq[i]->tid = (u8)i;
        sta->txq[i]->ac = IEEE80211_AC_BE;
    }

    memcpy(sta->addr, rt->connect_params.bssid, ETH_ALEN);
    sta->bandwidth = IEEE80211_STA_RX_BW_20;
    sta->deflink.bandwidth = IEEE80211_STA_RX_BW_20;
    sta->deflink.sta = sta;
    sta->link[0] = &sta->deflink;

    sband = rt->hw->wiphy->bands[rt->hw->conf.chandef.chan->band];
    if (sband) {
        u32 all_rates =
            sband->n_bitrates >= 32 ? U32_MAX : (BIT(sband->n_bitrates) - 1);

        sta->supp_rates[sband->band] = all_rates;
        sta->deflink.supp_rates[sband->band] = all_rates;
    }
    if (bss) {
        u32 rates_mask = lcompat_ieee80211_supported_rates_mask(rt, bss);
        if (rates_mask) {
            sta->supp_rates[rt->hw->conf.chandef.chan->band] = rates_mask;
            sta->deflink.supp_rates[rt->hw->conf.chandef.chan->band] =
                rates_mask;
        }
    }

    rt->sta = sta;
    return 0;
}

static void lcompat_ieee80211_disconnect_local(lcompat_ieee80211_runtime_t *rt,
                                               uint16_t reason_code,
                                               uint32_t request_portid,
                                               bool by_ap) {
    bool have_wireless;

    if (!rt || !rt->vif)
        return;

    have_wireless = rt->netdev && rt->connect_params.has_bssid;

    if (rt->sta && rt->hw->ops && rt->hw->ops->sta_remove)
        rt->hw->ops->sta_remove(rt->hw, rt->vif, rt->sta);

    lcompat_ieee80211_free_sta(rt);

    rt->vif->cfg.assoc = false;
    rt->vif->cfg.aid = 0;
    rt->vif->bss_conf.assoc = false;
    rt->vif->bss_conf.aid = 0;
    eth_zero_addr(rt->vif->bss_conf.bssid);
    eth_zero_addr(rt->vif->bss_conf.transmitter_bssid);
    rt->vif->bss_conf.beacon_int = 0;
    rt->vif->bss_conf.basic_rates = 0;

    if (rt->hw->ops && rt->hw->ops->bss_info_changed) {
        rt->hw->ops->bss_info_changed(rt->hw, rt->vif, &rt->vif->bss_conf,
                                      BSS_CHANGED_ASSOC | BSS_CHANGED_BSSID |
                                          BSS_CHANGED_BEACON_INT);
    }

    if (have_wireless)
        netlink_publish_disconnect_event(rt->netdev, request_portid,
                                         reason_code, by_ap);

    rt->conn_state = LCOMPAT_CONN_DISCONNECTED;
    lcompat_ieee80211_clear_connect_params(rt);
    if (rt->netdev)
        netdev_set_link_state(rt->netdev, false);
    (void)lcompat_ieee80211_update_wireless_info(rt);
}

static void lcompat_ieee80211_connect_failed(lcompat_ieee80211_runtime_t *rt,
                                             uint16_t status_code) {
    uint32_t request_portid;

    if (!rt)
        return;

    request_portid = rt->connect_request_portid;
    rt->conn_state = LCOMPAT_CONN_DISCONNECTED;
    if (rt->netdev)
        netdev_set_link_state(rt->netdev, false);
    (void)lcompat_ieee80211_update_wireless_info(rt);
    netlink_publish_connect_result(rt->netdev, request_portid, status_code);
    lcompat_ieee80211_clear_connect_params(rt);
    (void)lcompat_ieee80211_update_wireless_info(rt);
}

static int lcompat_ieee80211_connect_complete(lcompat_ieee80211_runtime_t *rt,
                                              const netdev_scan_result_t *bss,
                                              uint16_t aid) {
    int ret;
    uint32_t request_portid;

    if (!rt || !bss || !rt->netdev)
        return -EINVAL;

    if (rt->sta)
        lcompat_ieee80211_free_sta(rt);

    ret = lcompat_ieee80211_alloc_sta(rt, bss);
    if (ret)
        return ret;

    if (rt->hw->ops && rt->hw->ops->sta_add) {
        ret = rt->hw->ops->sta_add(rt->hw, rt->vif, rt->sta);
        if (ret) {
            lcompat_ieee80211_free_sta(rt);
            return ret;
        }
    }

    request_portid = rt->connect_request_portid;
    rt->conn_state = LCOMPAT_CONN_CONNECTED;
    rt->vif->cfg.assoc = true;
    rt->vif->cfg.aid = aid;
    rt->vif->bss_conf.assoc = true;
    rt->vif->bss_conf.aid = aid;
    rt->vif->bss_conf.beacon_int = rt->beacon_interval;
    rt->vif->bss_conf.basic_rates = lcompat_ieee80211_basic_rates_mask(rt, bss);
    memcpy(rt->vif->bss_conf.bssid, rt->connect_params.bssid, ETH_ALEN);
    memcpy(rt->vif->bss_conf.transmitter_bssid, rt->connect_params.bssid,
           ETH_ALEN);

    if (rt->hw->ops && rt->hw->ops->bss_info_changed) {
        rt->hw->ops->bss_info_changed(rt->hw, rt->vif, &rt->vif->bss_conf,
                                      BSS_CHANGED_BSSID |
                                          BSS_CHANGED_BEACON_INT);
        rt->hw->ops->bss_info_changed(rt->hw, rt->vif, &rt->vif->bss_conf,
                                      BSS_CHANGED_ASSOC);
    }

    netdev_set_link_state(rt->netdev, true);
    (void)lcompat_ieee80211_update_wireless_info(rt);
    netlink_publish_connect_result(rt->netdev, request_portid,
                                   LCOMPAT_WLAN_STATUS_SUCCESS);
    rt->connect_request_portid = 0;
    return 0;
}

static int lcompat_netdev_connect(void *desc,
                                  const netdev_connect_params_t *params,
                                  uint32_t request_portid) {
    lcompat_ieee80211_runtime_t *rt = (lcompat_ieee80211_runtime_t *)desc;
    netdev_scan_result_t selected;
    int ret;

    if (!rt || !params || !rt->started)
        return -ENODEV;
    if (params->privacy)
        return -EOPNOTSUPP;
    if (params->auth_type != LCOMPAT_NL80211_AUTHTYPE_AUTOMATIC &&
        params->auth_type != LCOMPAT_NL80211_AUTHTYPE_OPEN_SYSTEM)
        return -EOPNOTSUPP;

    spin_lock(&rt->lock);
    if (rt->conn_state != LCOMPAT_CONN_DISCONNECTED) {
        spin_unlock(&rt->lock);
        return -EBUSY;
    }
    rt->connect_params = *params;
    rt->connect_request_portid = request_portid;
    spin_unlock(&rt->lock);

    ret = lcompat_ieee80211_select_bss(rt, &rt->connect_params, &selected);
    if (ret)
        goto err_connect;

    if (!rt->connect_params.has_bssid) {
        memcpy(rt->connect_params.bssid, selected.bssid, ETH_ALEN);
        rt->connect_params.has_bssid = true;
    }
    if (!rt->connect_params.frequency)
        rt->connect_params.frequency = selected.frequency;
    rt->beacon_interval = selected.beacon_interval;
    rt->capability = selected.capability;
    rt->use_qos = lcompat_ieee80211_bss_has_wmm(&selected);

    ret = lcompat_ieee80211_set_channel(rt, rt->connect_params.frequency);
    if (ret)
        goto err_connect;

    rt->conn_state = LCOMPAT_CONN_AUTHENTICATING;
    (void)lcompat_ieee80211_update_wireless_info(rt);
    ret = lcompat_ieee80211_send_auth(rt);
    if (ret)
        goto err_connect;

    return 0;

err_connect:
    lcompat_ieee80211_connect_failed(rt, (uint16_t)(ret < 0 ? 1 : ret));
    return ret;
}

static int lcompat_netdev_disconnect(void *desc, uint16_t reason_code,
                                     uint32_t request_portid) {
    lcompat_ieee80211_runtime_t *rt = (lcompat_ieee80211_runtime_t *)desc;
    struct lcompat_ieee80211_reason_frame *deauth;
    struct sk_buff *skb = NULL;

    if (!rt || !rt->started)
        return -ENODEV;

    if (rt->conn_state != LCOMPAT_CONN_CONNECTED)
        return -ENOTCONN;

    skb = lcompat_ieee80211_alloc_tx_skb(rt, sizeof(*deauth));
    if (skb) {
        deauth = skb_put_zero(skb, sizeof(*deauth));
        if (!deauth) {
            dev_kfree_skb_any(skb);
            skb = NULL;
            goto disconnect_local;
        }
        deauth->frame_control = cpu_to_le16(LCOMPAT_IEEE80211_FTYPE_MGMT |
                                            LCOMPAT_IEEE80211_STYPE_DEAUTH);
        memcpy(deauth->da, rt->connect_params.bssid, ETH_ALEN);
        memcpy(deauth->sa, rt->vif->addr, ETH_ALEN);
        memcpy(deauth->bssid, rt->connect_params.bssid, ETH_ALEN);
        deauth->seq_ctrl = lcompat_ieee80211_next_seq_ctrl(rt);
        deauth->reason_code = cpu_to_le16(reason_code);
        (void)lcompat_ieee80211_send_mgmt(rt, skb);
    }

disconnect_local:
    lcompat_ieee80211_disconnect_local(
        rt, reason_code ? reason_code : LCOMPAT_WLAN_REASON_DEAUTH_LEAVING,
        request_portid, false);
    return 0;
}

struct ieee80211_sta *lcompat_ieee80211_find_sta(struct ieee80211_vif *vif,
                                                 const u8 *addr) {
    lcompat_ieee80211_runtime_t *rt;
    struct ieee80211_sta *sta = NULL;

    spin_lock(&lcompat_ieee80211_runtimes_lock);
    list_for_each_entry(rt, &lcompat_ieee80211_runtimes, list) {
        if (rt->vif != vif || !rt->sta)
            continue;
        if (addr && memcmp(rt->sta->addr, addr, ETH_ALEN) != 0)
            continue;
        sta = rt->sta;
        break;
    }
    spin_unlock(&lcompat_ieee80211_runtimes_lock);

    return sta;
}

struct ieee80211_sta *
lcompat_ieee80211_find_sta_by_ifaddr(struct ieee80211_hw *hw, const u8 *addr,
                                     const u8 *localaddr) {
    lcompat_ieee80211_runtime_t *rt = lcompat_ieee80211_runtime(hw);

    if (!rt || !rt->sta)
        return NULL;
    if (localaddr && memcmp(rt->vif->addr, localaddr, ETH_ALEN) != 0)
        return NULL;
    if (addr && memcmp(rt->sta->addr, addr, ETH_ALEN) != 0)
        return NULL;
    return rt->sta;
}

void ieee80211_iterate_active_interfaces_atomic(
    struct ieee80211_hw *hw, int iterator_flags,
    void (*iterator)(void *data, u8 *mac, struct ieee80211_vif *vif),
    void *data) {
    lcompat_ieee80211_runtime_t *rt = lcompat_ieee80211_runtime(hw);

    (void)iterator_flags;

    if (!rt || !iterator)
        return;

    spin_lock(&rt->lock);
    if (rt->vif_added && rt->vif)
        iterator(data, rt->vif->addr, rt->vif);
    spin_unlock(&rt->lock);
}

void ieee80211_iterate_active_interfaces(
    struct ieee80211_hw *hw, int iterator_flags,
    void (*iterator)(void *data, u8 *mac, struct ieee80211_vif *vif),
    void *data) {
    ieee80211_iterate_active_interfaces_atomic(hw, iterator_flags, iterator,
                                               data);
}

void ieee80211_iterate_interfaces(struct ieee80211_hw *hw, int iterator_flags,
                                  void (*iterator)(void *data, u8 *mac,
                                                   struct ieee80211_vif *vif),
                                  void *data) {
    ieee80211_iterate_active_interfaces_atomic(hw, iterator_flags, iterator,
                                               data);
}

void ieee80211_disconnect(struct ieee80211_vif *vif, bool reconnect) {
    lcompat_ieee80211_runtime_t *rt;

    (void)reconnect;

    spin_lock(&lcompat_ieee80211_runtimes_lock);
    list_for_each_entry(rt, &lcompat_ieee80211_runtimes, list) {
        if (rt->vif != vif)
            continue;
        spin_unlock(&lcompat_ieee80211_runtimes_lock);
        lcompat_ieee80211_disconnect_local(
            rt, LCOMPAT_WLAN_REASON_DEAUTH_LEAVING, 0, false);
        return;
    }
    spin_unlock(&lcompat_ieee80211_runtimes_lock);
}

void ieee80211_iterate_stations_atomic(
    struct ieee80211_hw *hw,
    void (*iterator)(void *data, struct ieee80211_sta *sta), void *data) {
    lcompat_ieee80211_runtime_t *rt = lcompat_ieee80211_runtime(hw);

    if (!rt || !iterator)
        return;

    spin_lock(&rt->lock);
    if (rt->sta && rt->conn_state == LCOMPAT_CONN_CONNECTED)
        iterator(data, rt->sta);
    spin_unlock(&rt->lock);
}

static int lcompat_netdev_recv(void *desc, void *data, uint32_t len) {
    lcompat_ieee80211_runtime_t *rt = (lcompat_ieee80211_runtime_t *)desc;
    struct sk_buff *skb;

    if (!rt || !data)
        return -EINVAL;

    skb = skb_dequeue(&rt->rx_queue);
    if (!skb)
        return -EAGAIN;

    if (len < skb->len) {
        dev_kfree_skb_any(skb);
        return -EMSGSIZE;
    }

    memcpy(data, skb->data, skb->len);
    len = skb->len;
    dev_kfree_skb_any(skb);
    return (int)len;
}

static int lcompat_ieee80211_build_data_frame(lcompat_ieee80211_runtime_t *rt,
                                              const void *data, uint32_t len,
                                              struct sk_buff **out_skb) {
    const struct lcompat_ethhdr *eth;
    struct ieee80211_tx_info *info;
    struct ieee80211_hdr *hdr;
    struct sk_buff *skb;
    size_t payload_len;
    size_t hdr_len = sizeof(*hdr);
    size_t frame_len;
    u8 *pos;

    if (!rt || !rt->hw || !rt->vif || !out_skb || len < sizeof(*eth))
        return -EINVAL;
    if (!rt->started || !rt->hw->ops || !rt->hw->ops->tx)
        return -ENODEV;
    if (rt->conn_state != LCOMPAT_CONN_CONNECTED ||
        is_zero_ether_addr(rt->vif->bss_conf.bssid))
        return -ENOTCONN;

    eth = (const struct lcompat_ethhdr *)data;
    payload_len = len - sizeof(*eth);
    if (rt->use_qos)
        hdr_len += sizeof(__le16);
    if (lcompat_size_add(hdr_len, LCOMPAT_LLC_SNAP_LEN, &frame_len) ||
        lcompat_size_add(frame_len, payload_len, &frame_len))
        return -EOVERFLOW;
    if (frame_len > UINT_MAX || payload_len > UINT_MAX)
        return -EOVERFLOW;

    skb = lcompat_ieee80211_alloc_tx_skb(rt, frame_len);
    if (!skb)
        return -ENOMEM;

    hdr = skb_put_zero(skb, sizeof(*hdr));
    if (!hdr) {
        dev_kfree_skb_any(skb);
        return -ENOMEM;
    }

    hdr->frame_control =
        cpu_to_le16(0x0008 | (rt->use_qos ? IEEE80211_STYPE_QOS_DATA : 0)) |
        IEEE80211_FCTL_TODS;
    memcpy(hdr->addr1, rt->vif->bss_conf.bssid, ETH_ALEN);
    memcpy(hdr->addr2, rt->vif->addr, ETH_ALEN);
    memcpy(hdr->addr3, eth->h_dest, ETH_ALEN);
    hdr->seq_ctrl = cpu_to_le16((rt->tx_seq++ & 0xfff) << 4);

    if (rt->use_qos) {
        __le16 *qos = skb_put_zero(skb, sizeof(*qos));

        if (!qos) {
            dev_kfree_skb_any(skb);
            return -ENOMEM;
        }
    }

    pos = skb_put(skb, LCOMPAT_LLC_SNAP_LEN);
    if (!pos) {
        dev_kfree_skb_any(skb);
        return -ENOMEM;
    }
    memcpy(pos, lcompat_rfc1042_header, sizeof(lcompat_rfc1042_header));
    memcpy(pos + sizeof(lcompat_rfc1042_header), &eth->h_proto,
           sizeof(eth->h_proto));

    skb_put_data(skb, (const u8 *)data + sizeof(*eth),
                 (unsigned int)payload_len);

    skb->protocol = eth->h_proto;
    skb_set_queue_mapping(skb, IEEE80211_AC_BE);
    skb->priority = 0; /* Linux uses skb priority as 802.1D TID; TID0 is BE. */

    info = IEEE80211_SKB_CB(skb);
    memset(info, 0, sizeof(*info));
    info->control.vif = rt->vif;

    *out_skb = skb;
    return 0;
}

static int lcompat_netdev_send(void *desc, void *data, uint32_t len) {
    lcompat_ieee80211_runtime_t *rt = (lcompat_ieee80211_runtime_t *)desc;
    struct ieee80211_tx_control control = {0};
    struct sk_buff *skb = NULL;
    int ret;

    ret = lcompat_ieee80211_build_data_frame(rt, data, len, &skb);
    if (ret)
        return ret;

    control.sta = rt->sta;
    rt->hw->ops->tx(rt->hw, &control, skb);
    return 0;
}

static int lcompat_ieee80211_default_channel(struct ieee80211_hw *hw) {
    struct ieee80211_supported_band *sband;

    if (!hw || !hw->wiphy)
        return -EINVAL;
    if (hw->conf.chandef.chan)
        return 0;

    sband = hw->wiphy->bands[NL80211_BAND_2GHZ];
    if (!sband || sband->n_channels <= 0)
        sband = hw->wiphy->bands[NL80211_BAND_5GHZ];
    if (!sband || sband->n_channels <= 0)
        return -EINVAL;

    hw->conf.chandef.chan = &sband->channels[0];
    hw->conf.chandef.width = NL80211_CHAN_WIDTH_20;
    hw->conf.chandef.center_freq1 = sband->channels[0].center_freq;
    hw->conf.chandef.center_freq2 = 0;
    return 0;
}

static int lcompat_ieee80211_alloc_vif(lcompat_ieee80211_runtime_t *rt) {
    size_t vif_size;
    size_t txq_size;

    if (!rt || !rt->hw)
        return -EINVAL;
    if (rt->hw->vif_data_size < 0 || rt->hw->txq_data_size < 0)
        return -EINVAL;
    if (lcompat_size_add(sizeof(*rt->vif), (size_t)rt->hw->vif_data_size,
                         &vif_size) ||
        lcompat_size_add(sizeof(*rt->txq), (size_t)rt->hw->txq_data_size,
                         &txq_size))
        return -EOVERFLOW;

    rt->vif = kzalloc(vif_size, GFP_KERNEL);
    rt->txq = kzalloc(txq_size, GFP_KERNEL);
    if (!rt->vif || !rt->txq) {
        kfree(rt->vif);
        kfree(rt->txq);
        rt->vif = NULL;
        rt->txq = NULL;
        return -ENOMEM;
    }

    rt->vif->type = NL80211_IFTYPE_STATION;
    ether_addr_copy(rt->vif->addr, rt->hw->perm_addr);
    rt->vif->bss_conf.vif = rt->vif;
    rt->vif->txq = rt->txq;

    rt->txq->vif = rt->vif;
    rt->txq->ac = IEEE80211_AC_BE;
    rt->txq->tid = 0;

    return 0;
}

static void
lcompat_ieee80211_fill_wireless_info(lcompat_ieee80211_runtime_t *rt,
                                     netdev_wireless_info_t *info) {
    if (!rt || !rt->hw || !rt->hw->wiphy || !rt->netdev || !info)
        return;

    memset(info, 0, sizeof(*info));
    info->present = true;
    info->connected = rt->conn_state == LCOMPAT_CONN_CONNECTED;
    info->wiphy_index = rt->netdev->id + 1;
    info->iftype = rt->vif ? (uint32_t)rt->vif->type : NL80211_IFTYPE_STATION;
    info->interface_modes = rt->hw->wiphy->interface_modes;
    info->max_scan_ssids = rt->hw->wiphy->max_scan_ssids;
    info->frequency = rt->connect_params.frequency;
    info->ssid_len = rt->connect_params.ssid_len;
    memcpy(info->ssid, rt->connect_params.ssid, info->ssid_len);
    if (rt->connect_params.has_bssid)
        memcpy(info->bssid, rt->connect_params.bssid, ETH_ALEN);
    snprintf(info->wiphy_name, sizeof(info->wiphy_name), "phy%u",
             rt->netdev->id);
}

static void lcompat_ieee80211_free_vif(lcompat_ieee80211_runtime_t *rt) {
    if (!rt)
        return;
    kfree(rt->txq);
    rt->txq = NULL;
    kfree(rt->vif);
    rt->vif = NULL;
}

static void lcompat_ieee80211_free_scan_req(lcompat_ieee80211_runtime_t *rt) {
    if (!rt || !rt->scan_req)
        return;

    kfree(rt->scan_req->req.channels);
    kfree(rt->scan_req->req.ssids);
    kfree(rt->scan_req);
    rt->scan_req = NULL;
}

static int
lcompat_ieee80211_alloc_scan_req(lcompat_ieee80211_runtime_t *rt,
                                 const netdev_scan_params_t *params) {
    struct ieee80211_channel **channels = NULL;
    struct cfg80211_scan_ssid *ssids = NULL;
    struct ieee80211_scan_request *scan_req = NULL;
    uint32_t n_channels = 0;
    uint32_t ssid_count = 0;
    int ret = -ENOMEM;

    if (!rt || !rt->hw || !rt->hw->wiphy)
        return -EINVAL;
    if (rt->scan_req)
        return -EBUSY;

    for (int band = 0; band < IEEE80211_NUM_BANDS; band++) {
        struct ieee80211_supported_band *sband = rt->hw->wiphy->bands[band];

        if (!sband || sband->n_channels <= 0)
            continue;
        if ((uint32_t)sband->n_channels > (uint32_t)INT_MAX - n_channels)
            return -EOVERFLOW;
        n_channels += (uint32_t)sband->n_channels;
    }

    if (n_channels == 0)
        return -EINVAL;

    scan_req = kzalloc(sizeof(*scan_req), GFP_KERNEL);
    channels = kcalloc(n_channels, sizeof(*channels), GFP_KERNEL);
    ssid_count = params && params->n_ssids ? params->n_ssids : 1;
    if (ssid_count > NETDEV_MAX_SCAN_SSIDS)
        ssid_count = NETDEV_MAX_SCAN_SSIDS;
    ssids = kcalloc(ssid_count, sizeof(*ssids), GFP_KERNEL);
    if (!scan_req || !channels || !ssids)
        goto out;

    scan_req->req.channels = channels;
    scan_req->req.n_channels = (int)n_channels;
    scan_req->req.ssids = ssids;
    scan_req->req.n_ssids = (int)ssid_count;
    scan_req->req.duration_mandatory = false;
    scan_req->req.no_cck = false;
    eth_broadcast_addr(scan_req->req.bssid);

    n_channels = 0;
    for (int band = 0; band < IEEE80211_NUM_BANDS; band++) {
        struct ieee80211_supported_band *sband = rt->hw->wiphy->bands[band];

        if (!sband || sband->n_channels <= 0)
            continue;
        for (int i = 0; i < sband->n_channels; i++)
            channels[n_channels++] = &sband->channels[i];
    }

    if (params && params->n_ssids) {
        for (uint32_t i = 0; i < ssid_count; i++) {
            if (params->ssids[i].len > sizeof(ssids[i].ssid)) {
                ret = -EINVAL;
                goto out;
            }
            ssids[i].ssid_len = params->ssids[i].len;
            memcpy(ssids[i].ssid, params->ssids[i].ssid, params->ssids[i].len);
        }
    } else {
        ssids[0].ssid_len = 0;
    }

    rt->scan_req = scan_req;
    return 0;

out:
    kfree(ssids);
    kfree(channels);
    kfree(scan_req);
    return ret;
}

static int lcompat_netdev_trigger_scan(void *desc,
                                       const netdev_scan_params_t *params,
                                       uint32_t request_portid) {
    lcompat_ieee80211_runtime_t *rt = (lcompat_ieee80211_runtime_t *)desc;
    int ret;

    if (!rt || !rt->hw || !rt->vif || !rt->netdev)
        return -ENODEV;
    if (!rt->started || !rt->hw->ops || !rt->hw->ops->hw_scan)
        return -EOPNOTSUPP;

    ret = lcompat_ieee80211_alloc_scan_req(rt, params);
    if (ret)
        return ret;

    lcompat_ieee80211_configure_scan_filter(rt, true);

    ret = netdev_scan_begin(rt->netdev, request_portid);
    if (ret)
        goto err_disable_filter;

    ret = rt->hw->ops->hw_scan(rt->hw, rt->vif, rt->scan_req);
    if (ret == 1) {
        ret = lcompat_ieee80211_run_sw_scan(rt);
        lcompat_ieee80211_configure_scan_filter(rt, false);
        netdev_scan_complete(rt->netdev, ret != 0);
        lcompat_ieee80211_free_scan_req(rt);
        return ret;
    }
    if (ret)
        goto err_complete_scan;

    return 0;

err_complete_scan:
    netdev_scan_complete(rt->netdev, true);
err_disable_filter:
    lcompat_ieee80211_configure_scan_filter(rt, false);
err_free_scan_req:
    lcompat_ieee80211_free_scan_req(rt);
    return ret;
}

static int lcompat_ieee80211_store_scan_result(lcompat_ieee80211_runtime_t *rt,
                                               struct sk_buff *skb) {
    struct ieee80211_rx_status *status;
    struct ieee80211_mgmt *mgmt;
    netdev_scan_result_t result;
    size_t hdr_len;
    size_t ie_len;
    __le16 fc;

    if (!rt || !rt->netdev || !lcompat_skb_valid_bounds(skb) ||
        skb->len < sizeof(*mgmt))
        return -EINVAL;

    mgmt = (struct ieee80211_mgmt *)skb->data;
    fc = mgmt->frame_control;
    if (ieee80211_is_beacon(fc))
        hdr_len = offsetof(struct ieee80211_mgmt, u.beacon.variable);
    else if (ieee80211_is_probe_resp(fc))
        hdr_len = offsetof(struct ieee80211_mgmt, u.probe_resp.variable);
    else
        return -EINVAL;
    if (skb->len <= hdr_len)
        return -EINVAL;

    memset(&result, 0, sizeof(result));
    status = IEEE80211_SKB_RXCB(skb);
    result.valid = true;
    ether_addr_copy(result.bssid, mgmt->bssid);
    result.frequency = status->freq
                           ? (uint32_t)status->freq
                           : (uint32_t)rt->hw->conf.chandef.center_freq1;
    result.signal_mbm = status->signal * 100;
    if (ieee80211_is_beacon(fc)) {
        result.tsf = le64_to_cpu(mgmt->u.beacon.timestamp);
        result.beacon_interval = le16_to_cpu(mgmt->u.beacon.beacon_int);
        result.capability = le16_to_cpu(mgmt->u.beacon.capab_info);
    } else {
        result.tsf = le64_to_cpu(mgmt->u.probe_resp.timestamp);
        result.beacon_interval = le16_to_cpu(mgmt->u.probe_resp.beacon_int);
        result.capability = le16_to_cpu(mgmt->u.probe_resp.capab_info);
    }
    result.seen_ms_ago = 0;
    ie_len = skb->len - hdr_len;
    if (ie_len > NETDEV_MAX_SCAN_IE_LEN)
        ie_len = NETDEV_MAX_SCAN_IE_LEN;
    result.ie_len = (uint16_t)ie_len;
    memcpy(result.ies, skb->data + hdr_len, ie_len);

    return netdev_scan_store_result(rt->netdev, &result);
}

struct ieee80211_hw *ieee80211_alloc_hw(size_t priv_len,
                                        const struct ieee80211_ops *ops) {
    struct ieee80211_hw *hw = kzalloc(sizeof(*hw), GFP_KERNEL);

    if (!hw)
        return NULL;

    hw->wiphy = kzalloc(sizeof(*hw->wiphy), GFP_KERNEL);
    if (!hw->wiphy) {
        kfree(hw);
        return NULL;
    }

    hw->priv = kzalloc(priv_len ? priv_len : 1, GFP_KERNEL);
    if (!hw->priv) {
        kfree(hw->wiphy);
        kfree(hw);
        return NULL;
    }

    hw->ops = ops;
    hw->queues = IEEE80211_NUM_ACS;
    hw->wiphy->hw = hw;
    hw->wiphy->rts_threshold = LCOMPAT_IEEE80211_RTS_THRESHOLD;

    return hw;
}

void ieee80211_free_hw(struct ieee80211_hw *hw) {
    if (!hw)
        return;
    if (hw->lcompat_runtime)
        ieee80211_unregister_hw(hw);
    kfree(hw->priv);
    kfree(hw->wiphy);
    kfree(hw);
}

int ieee80211_register_hw(struct ieee80211_hw *hw) {
    lcompat_ieee80211_runtime_t *rt;
    netdev_wireless_info_t wireless_info;
    int ret;

    if (!hw || !hw->ops)
        return -EINVAL;

    ret = lcompat_ieee80211_default_channel(hw);
    if (ret)
        return ret;

    rt = kzalloc(sizeof(*rt), GFP_KERNEL);
    if (!rt)
        return -ENOMEM;

    rt->hw = hw;
    rt->lock = SPIN_INIT;
    INIT_LIST_HEAD(&rt->list);
    skb_queue_head_init(&rt->rx_queue);
    rt->conn_state = LCOMPAT_CONN_DISCONNECTED;
    hw->lcompat_runtime = rt;

    ret = lcompat_ieee80211_alloc_vif(rt);
    if (ret)
        goto err_runtime;

    rt->netdev = netdev_register(NULL, NETDEV_TYPE_WIFI, rt, hw->perm_addr,
                                 LCOMPAT_IEEE80211_MTU, lcompat_netdev_send,
                                 lcompat_netdev_recv);
    if (!rt->netdev) {
        ret = -ENOMEM;
        goto err_vif;
    }

    lcompat_ieee80211_fill_wireless_info(rt, &wireless_info);
    ret = netdev_set_wireless_info(rt->netdev, &wireless_info);
    if (ret)
        goto err_netdev;

    ret = netdev_set_trigger_scan(rt->netdev, lcompat_netdev_trigger_scan);
    if (ret)
        goto err_netdev;

    ret = netdev_set_trigger_connect(rt->netdev, lcompat_netdev_connect);
    if (ret)
        goto err_netdev;

    ret = netdev_set_trigger_disconnect(rt->netdev, lcompat_netdev_disconnect);
    if (ret)
        goto err_netdev;

    ret = hw->ops->add_interface ? hw->ops->add_interface(hw, rt->vif) : 0;
    if (ret)
        goto err_netdev;
    rt->vif_added = true;

    ret = hw->ops->start ? hw->ops->start(hw) : 0;
    if (ret)
        goto err_remove_vif;
    rt->started = true;

    netdev_set_admin_state(rt->netdev, true);
    netdev_set_link_state(rt->netdev, false);
    spin_lock(&lcompat_ieee80211_runtimes_lock);
    list_add_tail(&rt->list, &lcompat_ieee80211_runtimes);
    spin_unlock(&lcompat_ieee80211_runtimes_lock);
    return 0;

err_remove_vif:
    if (rt->vif_added && hw->ops->remove_interface)
        hw->ops->remove_interface(hw, rt->vif);
    rt->vif_added = false;
err_netdev:
    if (rt->netdev) {
        netdev_unregister(rt->netdev);
        rt->netdev = NULL;
    }
err_vif:
    lcompat_ieee80211_free_vif(rt);
err_runtime:
    hw->lcompat_runtime = NULL;
    kfree(rt);
    return ret;
}

void ieee80211_unregister_hw(struct ieee80211_hw *hw) {
    lcompat_ieee80211_runtime_t *rt = lcompat_ieee80211_runtime(hw);

    if (!rt)
        return;

    spin_lock(&lcompat_ieee80211_runtimes_lock);
    if (!list_empty(&rt->list))
        list_del_init(&rt->list);
    spin_unlock(&lcompat_ieee80211_runtimes_lock);

    if (rt->netdev)
        netdev_set_admin_state(rt->netdev, false);

    if (rt->conn_state != LCOMPAT_CONN_DISCONNECTED)
        lcompat_ieee80211_disconnect_local(
            rt, LCOMPAT_WLAN_REASON_DEAUTH_LEAVING, 0, false);

    if (rt->started && hw->ops && hw->ops->stop)
        hw->ops->stop(hw, false);
    rt->started = false;

    if (rt->vif_added && hw->ops && hw->ops->remove_interface)
        hw->ops->remove_interface(hw, rt->vif);
    rt->vif_added = false;

    if (rt->netdev) {
        netdev_unregister(rt->netdev);
        rt->netdev = NULL;
    }

    lcompat_ieee80211_configure_scan_filter(rt, false);
    lcompat_ieee80211_free_scan_req(rt);
    lcompat_ieee80211_free_sta(rt);
    skb_queue_purge(&rt->rx_queue);
    lcompat_ieee80211_free_vif(rt);
    hw->lcompat_runtime = NULL;
    kfree(rt);
}

void ieee80211_tx_status_irqsafe(struct ieee80211_hw *hw, struct sk_buff *skb) {
    struct lcompat_tx_status_work *txs;

    (void)hw;
    if (!skb)
        return;

    txs = kzalloc(sizeof(*txs), GFP_ATOMIC);
    if (!txs) {
        dev_kfree_skb_any(skb);
        return;
    }

    txs->hw = hw;
    txs->skb = skb;
    INIT_WORK(&txs->work, lcompat_ieee80211_tx_status_work);
    if (!queue_work(system_wq, &txs->work)) {
        dev_kfree_skb_any(skb);
        kfree(txs);
    }
}

void ieee80211_scan_completed(struct ieee80211_hw *hw,
                              struct cfg80211_scan_info *info) {
    lcompat_ieee80211_runtime_t *rt = lcompat_ieee80211_runtime(hw);

    if (!rt || !rt->netdev)
        return;

    lcompat_ieee80211_configure_scan_filter(rt, false);
    netdev_scan_complete(rt->netdev, info ? info->aborted : false);
    lcompat_ieee80211_free_scan_req(rt);
}

static const u8 *lcompat_ieee80211_data_bssid(const struct ieee80211_hdr *hdr) {
    if (!hdr)
        return NULL;
    if (!ieee80211_has_tods(hdr->frame_control) &&
        ieee80211_has_fromds(hdr->frame_control))
        return hdr->addr2;
    if (ieee80211_has_tods(hdr->frame_control) &&
        !ieee80211_has_fromds(hdr->frame_control))
        return hdr->addr1;
    if (!ieee80211_has_tods(hdr->frame_control) &&
        !ieee80211_has_fromds(hdr->frame_control))
        return hdr->addr3;
    return NULL;
}

static bool lcompat_ieee80211_rx_data_matches_bss(
    lcompat_ieee80211_runtime_t *rt, const struct ieee80211_hdr *hdr, u32 len) {
    const u8 *bssid;

    (void)len;
    if (!rt || !rt->vif || !hdr || rt->conn_state != LCOMPAT_CONN_CONNECTED ||
        !rt->connect_params.has_bssid)
        return false;

    if (ieee80211_has_tods(hdr->frame_control) ||
        !ieee80211_has_fromds(hdr->frame_control))
        return false;

    bssid = lcompat_ieee80211_data_bssid(hdr);
    if (!bssid || !ether_addr_equal(bssid, rt->connect_params.bssid)) {
        if (is_multicast_ether_addr(hdr->addr1))
            return false;

        return false;
    }

    if (!ether_addr_equal(hdr->addr1, rt->vif->addr) &&
        !is_multicast_ether_addr(hdr->addr1))
        return false;

    return true;
}

static int lcompat_ieee80211_data_to_ethernet(struct sk_buff *skb,
                                              u8 local_addr[ETH_ALEN]) {
    struct ieee80211_hdr *hdr;
    struct lcompat_ethhdr *eth;
    const u8 *payload;
    u8 src[ETH_ALEN];
    u8 dst[ETH_ALEN];
    __le16 fc;
    u32 hdr_len;
    u32 payload_len;
    __be16 proto;
    u32 skip;
    bool tods;
    bool fromds;

    if (!lcompat_skb_valid_bounds(skb) ||
        skb->len < sizeof(*hdr) + LCOMPAT_LLC_SNAP_LEN)
        return -EINVAL;

    hdr = (struct ieee80211_hdr *)skb->data;
    fc = hdr->frame_control;
    tods = ieee80211_has_tods(fc);
    fromds = ieee80211_has_fromds(fc);

    if (le16_to_cpu(fc) & LCOMPAT_IEEE80211_FCTL_PROTECTED)
        return -EOPNOTSUPP;

    if (!ieee80211_is_data_present(fc) || (tods && fromds))
        return -EINVAL;
    hdr_len = lcompat_ieee80211_data_hdr_len(fc);
    if (skb->len < hdr_len + LCOMPAT_LLC_SNAP_LEN)
        return -EINVAL;

    if (!tods && fromds) {
        ether_addr_copy(dst, hdr->addr1);
        ether_addr_copy(src, hdr->addr3);
    } else if (tods && !fromds) {
        ether_addr_copy(dst, hdr->addr3);
        ether_addr_copy(src, hdr->addr2);
    } else {
        ether_addr_copy(dst, hdr->addr1);
        ether_addr_copy(src, hdr->addr2);
    }

    payload = skb->data + hdr_len;
    payload_len = skb->len - hdr_len;
    if (!lcompat_ieee80211_snap_to_ethertype(payload, payload_len, &proto,
                                             &skip))
        return -EINVAL;

    payload_len -= skip;
    memmove(skb->data + sizeof(*eth), payload + skip, payload_len);

    eth = (struct lcompat_ethhdr *)skb->data;
    ether_addr_copy(eth->h_dest, dst);
    ether_addr_copy(eth->h_source, src);
    eth->h_proto = proto;

    skb_trim(skb, sizeof(*eth) + payload_len);
    skb->protocol = eth->h_proto;
    (void)local_addr;
    return 0;
}

static bool lcompat_ieee80211_is_amsdu(const struct sk_buff *skb, u32 hdr_len) {
    const u8 *qos;

    if (!skb || skb->len < hdr_len ||
        hdr_len < sizeof(struct ieee80211_hdr) + 2)
        return false;

    qos = skb->data + hdr_len - 2;
    return !!(qos[0] & LCOMPAT_IEEE80211_QOS_CTL_A_MSDU_PRESENT);
}

static bool lcompat_ieee80211_snap_to_ethertype(const u8 *payload,
                                                u32 payload_len, __be16 *proto,
                                                u32 *skip) {
    u16 ethertype;

    if (!payload || !proto || !skip || payload_len < LCOMPAT_LLC_SNAP_LEN)
        return false;

    ethertype = get_unaligned_be16(payload + sizeof(lcompat_rfc1042_header));
    if (memcmp(payload, lcompat_rfc1042_header,
               sizeof(lcompat_rfc1042_header)) == 0 &&
        ethertype != LCOMPAT_ETH_P_AARP && ethertype != LCOMPAT_ETH_P_IPX) {
        *proto = cpu_to_be16(ethertype);
        *skip = LCOMPAT_LLC_SNAP_LEN;
        return true;
    }

    if (memcmp(payload, lcompat_bridge_tunnel_header,
               sizeof(lcompat_bridge_tunnel_header)) == 0) {
        *proto = cpu_to_be16(ethertype);
        *skip = LCOMPAT_LLC_SNAP_LEN;
        return true;
    }

    return false;
}

static struct sk_buff *lcompat_ieee80211_build_eth_skb(const u8 *dst,
                                                       const u8 *src,
                                                       const u8 *payload,
                                                       u32 payload_len) {
    struct lcompat_ethhdr *eth;
    struct sk_buff *skb;
    __be16 proto;
    u32 skip;
    u32 eth_payload_len;

    if (!dst || !src || !payload)
        return NULL;
    if (!lcompat_ieee80211_snap_to_ethertype(payload, payload_len, &proto,
                                             &skip))
        return NULL;
    eth_payload_len = payload_len - skip;
    if (eth_payload_len > UINT_MAX - sizeof(*eth))
        return NULL;

    skb = alloc_skb(sizeof(*eth) + eth_payload_len, GFP_ATOMIC);
    if (!skb)
        return NULL;

    eth = skb_put_zero(skb, sizeof(*eth));
    if (!eth)
        goto err;
    ether_addr_copy(eth->h_dest, dst);
    ether_addr_copy(eth->h_source, src);
    eth->h_proto = proto;

    if (eth_payload_len && !skb_put_data(skb, payload + skip, eth_payload_len))
        goto err;

    skb->protocol = proto;
    return skb;

err:
    dev_kfree_skb_any(skb);
    return NULL;
}

static int lcompat_ieee80211_enqueue_amsdu(lcompat_ieee80211_runtime_t *rt,
                                           struct sk_buff *skb, u32 hdr_len) {
    const u8 *pos;
    u32 remaining;
    u32 queued = 0;

    if (!rt || !skb || skb->len < hdr_len)
        return -EINVAL;

    pos = skb->data + hdr_len;
    remaining = skb->len - hdr_len;

    while (remaining >= sizeof(struct lcompat_ethhdr)) {
        const u8 *dst = pos;
        const u8 *src = pos + ETH_ALEN;
        u16 msdu_len = get_unaligned_be16(pos + ETH_ALEN * 2);
        struct sk_buff *eth_skb;
        u32 subframe_len;
        u32 padded_len;

        pos += sizeof(struct lcompat_ethhdr);
        remaining -= sizeof(struct lcompat_ethhdr);
        if (msdu_len == 0 || msdu_len > remaining)
            break;

        eth_skb = lcompat_ieee80211_build_eth_skb(dst, src, pos, msdu_len);
        if (eth_skb) {
            skb_queue_tail(&rt->rx_queue, eth_skb);
            queued++;
        }

        subframe_len = sizeof(struct lcompat_ethhdr) + msdu_len;
        padded_len = (subframe_len + 3U) & ~3U;
        if (padded_len < subframe_len ||
            padded_len - sizeof(struct lcompat_ethhdr) > remaining)
            break;

        pos += padded_len - sizeof(struct lcompat_ethhdr);
        remaining -= padded_len - sizeof(struct lcompat_ethhdr);
    }

    return queued ? 0 : -EINVAL;
}

static bool lcompat_ieee80211_mgmt_matches(lcompat_ieee80211_runtime_t *rt,
                                           struct ieee80211_mgmt *mgmt) {
    if (!rt || !mgmt)
        return false;
    if (memcmp(mgmt->da, rt->vif->addr, ETH_ALEN) != 0)
        return false;
    if (!rt->connect_params.has_bssid)
        return true;
    return memcmp(mgmt->bssid, rt->connect_params.bssid, ETH_ALEN) == 0;
}

static bool lcompat_ieee80211_handle_auth_rx(lcompat_ieee80211_runtime_t *rt,
                                             struct sk_buff *skb) {
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
    netdev_scan_result_t selected;
    u16 status_code;
    u16 transaction;

    if (skb->len < offsetof(struct ieee80211_mgmt, u.auth.variable))
        return false;
    if (rt->conn_state != LCOMPAT_CONN_AUTHENTICATING ||
        !lcompat_ieee80211_mgmt_matches(rt, mgmt))
        return true;

    status_code = le16_to_cpu(mgmt->u.auth.status_code);
    transaction = le16_to_cpu(mgmt->u.auth.auth_transaction);
    if (status_code != LCOMPAT_WLAN_STATUS_SUCCESS || transaction != 2) {
        lcompat_ieee80211_connect_failed(rt, status_code);
        return true;
    }

    if (lcompat_ieee80211_select_bss(rt, &rt->connect_params, &selected) < 0) {
        memset(&selected, 0, sizeof(selected));
        selected.valid = true;
        selected.frequency = rt->connect_params.frequency;
        memcpy(selected.bssid, rt->connect_params.bssid, ETH_ALEN);
        selected.beacon_interval = rt->beacon_interval;
        selected.capability = rt->capability;
    }

    rt->conn_state = LCOMPAT_CONN_ASSOCIATING;
    if (lcompat_ieee80211_send_assoc_req(rt, &selected) < 0)
        lcompat_ieee80211_connect_failed(rt, 1);
    return true;
}

static bool
lcompat_ieee80211_handle_assoc_resp_rx(lcompat_ieee80211_runtime_t *rt,
                                       struct sk_buff *skb) {
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
    netdev_scan_result_t selected;
    u16 status_code;
    u16 aid;

    if (skb->len < offsetof(struct ieee80211_mgmt, u.assoc_resp.variable))
        return false;
    if (rt->conn_state != LCOMPAT_CONN_ASSOCIATING ||
        !lcompat_ieee80211_mgmt_matches(rt, mgmt))
        return true;

    status_code = le16_to_cpu(mgmt->u.assoc_resp.status_code);
    aid = le16_to_cpu(mgmt->u.assoc_resp.aid) & 0x3fff;
    if (status_code != LCOMPAT_WLAN_STATUS_SUCCESS) {
        lcompat_ieee80211_connect_failed(rt, status_code);
        return true;
    }

    if (lcompat_ieee80211_select_bss(rt, &rt->connect_params, &selected) < 0) {
        memset(&selected, 0, sizeof(selected));
        selected.valid = true;
        selected.frequency = rt->connect_params.frequency;
        selected.beacon_interval = rt->beacon_interval;
        selected.capability = rt->capability;
        memcpy(selected.bssid, rt->connect_params.bssid, ETH_ALEN);
    }

    if (lcompat_ieee80211_connect_complete(rt, &selected, aid) < 0)
        lcompat_ieee80211_connect_failed(rt, 1);
    return true;
}

static bool
lcompat_ieee80211_handle_disconnect_rx(lcompat_ieee80211_runtime_t *rt,
                                       struct sk_buff *skb, bool deauth) {
    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
    u16 reason_code;

    if (deauth) {
        if (skb->len < offsetof(struct ieee80211_mgmt, u.deauth.variable))
            return false;
        reason_code = le16_to_cpu(mgmt->u.deauth.reason_code);
    } else {
        if (skb->len < offsetof(struct ieee80211_mgmt, u.disassoc.variable))
            return false;
        reason_code = le16_to_cpu(mgmt->u.disassoc.reason_code);
    }

    if (rt->conn_state == LCOMPAT_CONN_DISCONNECTED ||
        !lcompat_ieee80211_mgmt_matches(rt, mgmt))
        return true;

    lcompat_ieee80211_disconnect_local(rt, reason_code, 0, true);
    return true;
}

static bool lcompat_ieee80211_handle_mgmt_rx(lcompat_ieee80211_runtime_t *rt,
                                             struct sk_buff *skb) {
    struct ieee80211_hdr *hdr;

    if (!rt || !lcompat_skb_valid_bounds(skb) || skb->len < sizeof(*hdr))
        return false;

    hdr = (struct ieee80211_hdr *)skb->data;
    if (ieee80211_is_beacon(hdr->frame_control) ||
        ieee80211_is_probe_resp(hdr->frame_control)) {
        if (rt->scan_req)
            (void)lcompat_ieee80211_store_scan_result(rt, skb);
        return true;
    }
    if (ieee80211_is_probe_req(hdr->frame_control))
        return true;
    if (ieee80211_is_auth(hdr->frame_control))
        return lcompat_ieee80211_handle_auth_rx(rt, skb);
    if (ieee80211_is_assoc_resp(hdr->frame_control))
        return lcompat_ieee80211_handle_assoc_resp_rx(rt, skb);
    if (ieee80211_is_deauth(hdr->frame_control))
        return lcompat_ieee80211_handle_disconnect_rx(rt, skb, true);
    if (ieee80211_is_disassoc(hdr->frame_control))
        return lcompat_ieee80211_handle_disconnect_rx(rt, skb, false);
    return false;
}

void ieee80211_rx_irqsafe(struct ieee80211_hw *hw, struct sk_buff *skb) {
    lcompat_ieee80211_runtime_t *rt = lcompat_ieee80211_runtime(hw);
    struct ieee80211_rx_status *status;
    struct ieee80211_hdr *hdr;
    __le16 fc;
    u32 hdr_len;

    if (!rt || !lcompat_skb_valid_bounds(skb)) {
        dev_kfree_skb_any(skb);
        return;
    }

    status = (struct ieee80211_rx_status *)skb->cb;
    if (status->flag & (RX_FLAG_FAILED_FCS_CRC | RX_FLAG_NO_PSDU)) {
        dev_kfree_skb_any(skb);
        return;
    }
    if (ieee80211_hw_check(hw, RX_INCLUDES_FCS)) {
        if (skb->len <= FCS_LEN) {
            dev_kfree_skb_any(skb);
            return;
        }
        skb_trim(skb, skb->len - FCS_LEN);
    }

    if (skb->len >= sizeof(fc)) {
        fc = cpu_to_le16(get_unaligned_le16(skb->data));
        if (ieee80211_is_ctl(fc)) {
            dev_kfree_skb_any(skb);
            return;
        }
    }

    if (skb->len < sizeof(*hdr)) {
        dev_kfree_skb_any(skb);
        return;
    }

    hdr = (struct ieee80211_hdr *)skb->data;
    fc = hdr->frame_control;
    if (ieee80211_is_mgmt(hdr->frame_control)) {
        if (lcompat_ieee80211_handle_mgmt_rx(rt, skb)) {
            dev_kfree_skb_any(skb);
            return;
        }
        dev_kfree_skb_any(skb);
        return;
    }

    if (ieee80211_is_ctl(hdr->frame_control)) {
        dev_kfree_skb_any(skb);
        return;
    }

    if (!ieee80211_is_data_present(hdr->frame_control)) {
        dev_kfree_skb_any(skb);
        return;
    }

    if (!lcompat_ieee80211_rx_data_matches_bss(rt, hdr, skb->len)) {
        dev_kfree_skb_any(skb);
        return;
    }

    hdr_len = lcompat_ieee80211_data_hdr_len(hdr->frame_control);
    if (lcompat_ieee80211_is_amsdu(skb, hdr_len)) {
        if (lcompat_ieee80211_enqueue_amsdu(rt, skb, hdr_len) == 0) {
            dev_kfree_skb_any(skb);
            return;
        }
    }

    if (lcompat_ieee80211_data_to_ethernet(skb, hw->perm_addr) != 0) {
        dev_kfree_skb_any(skb);
        return;
    }

    skb_queue_tail(&rt->rx_queue, skb);
}
