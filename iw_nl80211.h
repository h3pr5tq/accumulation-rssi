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
 * Definitions and functions for nl80211 based routines.
 */
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include <netinet/ether.h>
#include <stdbool.h>


/*
 * Use local copy of nl80211.h rather than the one shipped with the distro in
 * /usr/include/linux. There are different versions, local one may be out of date.
 */
#include "nl80211.h"


#define BIT(x) (1ULL<<(x))		/* from iw:iw.h */

/* Set to 1 to enable callback debugging */
#define IW_NL_CB_DEBUG 0


/**
 * struct msg_attribute - attributes to nla_put into the message
 * @type:	type of the attribute
 * @len:	attribute length
 * @data:	pointer to data area of length @len
 */
struct msg_attribute {
	int		type,
			len;
	const void	*data;
};


/**
 * struct cmd - stolen and modified from iw:iw.h
 * @cmd:	  nl80211 command to send via GeNetlink
 * @sk:		  netlink socket to be used for this command
 * @flags:	  flags to set in the GeNetlink message
 * @handler:	  netlink callback handler
 * @handler_arg:  argument for @handler
 * @msg_args:	  additional attributes to pass into message
 * @msg_args_len: number of elements in @msg_args
 */
struct cmd {
	enum nl80211_commands	cmd;
	struct nl_sock		*sk;
	int			flags;
	int (*handler)(struct nl_msg *msg, void *arg);
	void 			*handler_arg;

	struct msg_attribute	*msg_args;
	uint8_t			msg_args_len;
};
extern int handle_cmd(struct cmd *cmd, const char * name_wifi_interface);



/* struct iw_nl80211_linkstat - aggregate link statistics
 * @status:           association status (%nl80211_bss_status)
 * @bssid:            station MAC address
 * @rx_packets: byte/packet counter for RX direction
 * @signal:           signal strength in dBm (0 if not present)
 */
struct iw_nl80211_linkstat {
	uint32_t	  	status;
	struct ether_addr	bssid;
	uint32_t		rx_packets;
	int8_t			signal;


};
extern void iw_nl80211_get_linkstat(struct iw_nl80211_linkstat *ls, const char * name_wifi_interface);
void iw_nl80211_get_mac(struct iw_nl80211_linkstat *ls, const char * name_wifi_interface);


/*
 * utils.c
 */
extern bool ether_addr_is_zero(const struct ether_addr *ea);


/* Predefined handlers, stolen from iw:iw.c */
static inline int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_STOP;
}

static inline int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static inline int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}
