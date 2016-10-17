/*
 *Took this source from wavemon-0.8.0 sources after removal of unnecessary
 *
 *Also remove function conf_ifname(), which return name of wifi interface (See function handle_cmd(struct cmd *cmd)) in iw_nl80211.c
 *Changed arguments for 3 functions in iw_nl80211.c:
 *      standard: handle_cmd(struct cmd *cmd)
 *      changed:  handle_cmd(struct cmd *cmd, const char * name_wifi_interface)
 *
 *      standard: iw_nl80211_get_linkstat(struct iw_nl80211_linkstat *ls)
 *      changed:  iw_nl80211_get_linkstat(struct iw_nl80211_linkstat *ls, const char * name_wifi_interface)
 *
 *      standard: iw_nl80211_get_survey(struct iw_nl80211_survey *sd)
 *      changed:  iw_nl80211_get_survey(struct iw_nl80211_survey *sd, const char * name_wifi_interface)
 *
 *Aim: get RSSI WiFi
 *To do it, use function iw_nl80211_get_linkstat(struct iw_nl80211_linkstat *ls, const char * name_wifi_interface) from iw_nl80211.c
 */


/*
 * FIXME:
 * PROTOTYPE: add nl80211 calls to iw_if. Mostly copied/stolen from iw
 */

#include <net/if.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "iw_nl80211.h"


/**
 * handle_cmd: process @cmd
 * Returns 0 if ok, -errno < 0 on failure
 * stolen/modified from iw:iw.c
 */
int handle_cmd(struct cmd *cmd, const char * name_wifi_interface)
{
	struct nl_cb *cb;
	struct nl_msg *msg;
	static int nl80211_id = -1;
	int ret;
	uint32_t ifindex, idx;

	/*
	 * Initialization of static components:
	 * - per-cmd socket
	 * - global nl80211 ID
	 * - per-cmd interface index (in case conf_ifname() changes)
	 */
	if (!cmd->sk) {
		cmd->sk = nl_socket_alloc();
		if (!cmd->sk) {
			fprintf(stderr, "failed to allocate netlink socket\n");
			exit(EXIT_FAILURE);
                }

		/* NB: not setting sk buffer size, using default 32Kb */
		if (genl_connect(cmd->sk)) {
			fprintf(stderr, "failed to connect to GeNetlink\n");
			exit(EXIT_FAILURE);
                }
	}

	if (nl80211_id < 0) {
		nl80211_id = genl_ctrl_resolve(cmd->sk, "nl80211");
		if (nl80211_id < 0) {
			fprintf(stderr, "nl80211 not found\n");
			exit(EXIT_FAILURE);
                }
	}

	ifindex = if_nametoindex(name_wifi_interface); //conf_ifname()
	if (ifindex == 0 && errno) {
                fprintf(stderr, "failed to look up interface %s\n", name_wifi_interface);
                exit(EXIT_FAILURE);
        }

	/*
	 * Message Preparation
	 */
	msg = nlmsg_alloc();
	if (!msg) {
                fprintf(stderr, "failed to allocate netlink message\n");
                exit(EXIT_FAILURE);
        }

	cb = nl_cb_alloc(IW_NL_CB_DEBUG ? NL_CB_DEBUG : NL_CB_DEFAULT);
	if (!cb) {
                fprintf(stderr, "failed to allocate netlink callback\n");
                exit(EXIT_FAILURE);
        }

	genlmsg_put(msg, 0, 0, nl80211_id, 0, cmd->flags, cmd->cmd, 0);

	/* netdev identifier: interface index */
	NLA_PUT(msg, NL80211_ATTR_IFINDEX, sizeof(ifindex), &ifindex);

	/* Additional attributes */
	if (cmd->msg_args) {
		for (idx = 0; idx < cmd->msg_args_len; idx++)
			NLA_PUT(msg, cmd->msg_args[idx].type,
				     cmd->msg_args[idx].len,
				     cmd->msg_args[idx].data);
	}

	ret = nl_send_auto_complete(cmd->sk, msg);
	if (ret < 0) {
                fprintf(stderr, "failed to send netlink message\n");
                exit(EXIT_FAILURE);
        }

	/*-------------------------------------------------------------------------
	 * Receive loop
	 *-------------------------------------------------------------------------*/
	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ret);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &ret);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &ret);
	if (cmd->handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, cmd->handler, cmd->handler_arg);

	while (ret > 0)
		nl_recvmsgs(cmd->sk, cb);

	nl_cb_put(cb);
	nlmsg_free(msg);
	goto out;

nla_put_failure:
	fprintf(stderr, "failed to add attribute to netlink message\n");
out:
	return ret;
}


static int link_handler(struct nl_msg *msg, void *arg)
{
	struct iw_nl80211_linkstat *ls = arg;
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *bss[NL80211_BSS_MAX + 1];
	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_BSSID] = { },
		[NL80211_BSS_STATUS] = { .type = NLA_U32 },
	};
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_BSS])
		return NL_SKIP;

	if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy))
		return NL_SKIP;

	if (!bss[NL80211_BSS_BSSID])
		return NL_SKIP;

	if (!bss[NL80211_BSS_STATUS]) //If sta not associate with this BBS, return NL_SKIP (== this message is throwed)
		return NL_SKIP;       //For each BSSes (after scan) - own individual netlink-message

	ls->status = nla_get_u32(bss[NL80211_BSS_STATUS]);
	switch (ls->status) {
	case NL80211_BSS_STATUS_ASSOCIATED:	/* apparently no longer used */
	case NL80211_BSS_STATUS_AUTHENTICATED:
	case NL80211_BSS_STATUS_IBSS_JOINED:
		memcpy(&ls->bssid, nla_data(bss[NL80211_BSS_BSSID]), ETH_ALEN);
	}

	return NL_SKIP;
}


/*
 * STATION COMMANDS
 */
/* stolen from iw:station.c */
static int link_sta_handler(struct nl_msg *msg, void *arg)
{
	struct iw_nl80211_linkstat *ls = arg;
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
	};

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_STA_INFO])
		return NL_SKIP;

	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO],
			     stats_policy))
		return NL_SKIP;

	if (sinfo[NL80211_STA_INFO_RX_PACKETS])
		ls->rx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]);

	if (sinfo[NL80211_STA_INFO_SIGNAL])
		ls->signal = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);

	return NL_SKIP;
}


/*
 * COMMAND HANDLERS
 */
void iw_nl80211_get_linkstat(struct iw_nl80211_linkstat *ls, const char * name_wifi_interface)
{
	static struct cmd cmd_getstation = {
		.cmd	 = NL80211_CMD_GET_STATION,
		.flags	 = 0,
		.handler = link_sta_handler
	};

	struct msg_attribute station_addr = {
		.type = NL80211_ATTR_MAC, //BSSID's MAC!
		.len  = sizeof(ls->bssid),
		.data = &ls->bssid
	};

	/*
	 * Details of the associated station
	 */
	cmd_getstation.handler_arg  = ls;
	cmd_getstation.msg_args     = &station_addr;
	cmd_getstation.msg_args_len = 1;

	handle_cmd(&cmd_getstation, name_wifi_interface);
}


void iw_nl80211_get_mac(struct iw_nl80211_linkstat *ls, const char * name_wifi_interface)
{
        static struct cmd cmd_linkstat = {
		.cmd	 = NL80211_CMD_GET_SCAN, //Get last results of scan
		.flags	 = NLM_F_DUMP,
		.handler = link_handler
	};

        cmd_linkstat.handler_arg = ls;
	memset(ls, 0, sizeof(*ls)); //struct ls is filled null
	handle_cmd(&cmd_linkstat, name_wifi_interface);

	/* If not associated to another station, the bssid is zeroed out */
	if (ether_addr_is_zero(&ls->bssid)) {
                fprintf(stderr, "system error: couldn't get BSSID's MAC, may be sta didn't associate\n");
                exit(EXIT_FAILURE);
	} else {
                printf("info: BSSID's MAC: ");
                for (int i = 0; i < ETH_ALEN; i++) printf("%X ", (int)(ls->bssid.ether_addr_octet[i]));
                printf("\n");
	}
}
