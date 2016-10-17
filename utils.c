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
 * General-purpose utilities used by multiple files.
 */


#include <netinet/ether.h>
#include <stdbool.h>
#include <string.h>


/* Return true if all ethernet octets are zero. */
bool ether_addr_is_zero(const struct ether_addr *ea)
{
	static const struct ether_addr zero = {{0}};

	return memcmp(ea, &zero, sizeof(zero)) == 0;
}

