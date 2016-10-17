#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "accumulation.h"
#include "iw_nl80211.h"

void accumulation_rssi(const struct arguments *config)
{
        FILE *fp;
        int i,
            num_rssi = config->time.integer * config->speed.integer;

        struct timespec                   start_time,
                                          finish_time,
                                          real_delay;

        struct iw_nl80211_linkstat        wifi_link_info;
        struct iw_nl80211_linkstat_tiny   *ptr_wifi_link_info_tiny;

        if ( !(fp = fopen(config->file, "w")) ) {
                fprintf(stderr, "system error: can't init FILE *\n");
                exit(EXIT_FAILURE);
        }

        //Allocate memory for buffer for rssi and rx_packets value
        ptr_wifi_link_info_tiny = (struct iw_nl80211_linkstat_tiny *)calloc( (size_t)num_rssi, sizeof(struct iw_nl80211_linkstat_tiny) );
        if (ptr_wifi_link_info_tiny == NULL) {
                fprintf(stderr, "system error: can't allocate memory for buffer for rssi and rx_packets value\n");
                exit(EXIT_FAILURE);
        }

        //Get BSSID's MAC, it is necessary for get rssi value
        iw_nl80211_get_mac(&wifi_link_info, config->interface);

        //Get delay value for main loop (see below)
        get_real_delay(&real_delay, config->interface, config->speed.integer);

        i = 0;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        //main loop
        while (i < num_rssi) {
                iw_nl80211_get_linkstat(&wifi_link_info, config->interface); ///ЕСЛИ ЗНАЧЕНИЯ НУЛТ ->> СОЕДИНЕНИЕ БЫЛО РАЗОРВАНО
                (ptr_wifi_link_info_tiny + i)->rx_packets = wifi_link_info.rx_packets;
                (ptr_wifi_link_info_tiny + i)->signal     = wifi_link_info.signal;
                i++;
                clock_nanosleep (CLOCK_MONOTONIC, 0, &real_delay, NULL);
        }
        clock_gettime(CLOCK_MONOTONIC, &finish_time);
        get_diff_timespec(&real_delay, &start_time, &finish_time);
        printf("info: real accumulation time: sec=%ld, nsec=%ld\n", real_delay.tv_sec, real_delay.tv_nsec);

        fprintf(fp, "Colummn \"number of received frames\" - very useful. This column reports: the rssi value has been updated or not.\n"
                    "rssi == 0\n\n"
                    "rssi:   number of received frames:\n\n");
        for (i = 0; i < num_rssi; i++)
                fprintf(fp, "%-8d%d\n", (ptr_wifi_link_info_tiny + i)->signal, (ptr_wifi_link_info_tiny + i)->rx_packets);

        if(fclose(fp)) {

                fprintf(stderr, "system error: can't close FILE *\n");
                exit(EXIT_FAILURE);
        }

        free(ptr_wifi_link_info_tiny);
}


void get_ideal_delay(struct timespec *ideal_delay, const int accumualtion_speed)
{
        if (accumualtion_speed == 1) {
                ideal_delay->tv_sec  = 1;
                ideal_delay->tv_nsec = 0;
        } else {
                ideal_delay->tv_sec  = 0;
                ideal_delay->tv_nsec = lround(1.0e+9 / accumualtion_speed);
        }
}


void get_diff_timespec(struct timespec *diff_time, const struct timespec *start_time, const struct timespec *finish_time)
{
        if (finish_time->tv_nsec > start_time->tv_nsec) {
                diff_time->tv_sec  = finish_time->tv_sec  - start_time->tv_sec;
                diff_time->tv_nsec = finish_time->tv_nsec - start_time->tv_nsec;
        } else {
                diff_time->tv_sec  = finish_time->tv_sec  - start_time->tv_sec  - 1;
                diff_time->tv_nsec = finish_time->tv_nsec - start_time->tv_nsec + 1000000000; //1e+9
        }
}


void get_real_delay(struct timespec *real_delay, const char * const name_interface, const int accumualtion_speed)
{
        struct timespec                   start_time,
                                          finish_time,
                                          iteration_without_delay,
                                          ideal_delay;

        struct iw_nl80211_linkstat        wifi_link_info;
        struct iw_nl80211_linkstat_tiny   ptr_wifi_link_info_tiny[ACCURACY_AVERAGE];

        double elapsed_time_ns = 0;

        //Ideal (theor) time of main loop's iteration in accumulation_rssi(...) (with delay)
        get_ideal_delay(&ideal_delay, accumualtion_speed);
        printf("info: ideal (theor) time of main loop's iteration (with delay): sec=%ld, nsec=%ld\n",
               ideal_delay.tv_sec, ideal_delay.tv_nsec);

        //Get BSSID's MAC, it is necessary for get rssi value
        iw_nl80211_get_mac(&wifi_link_info, name_interface);

        //Assess of time of main loop's iteration in accumulation_rssi(...) without delay
        for (int i = 0, j = 0; j < ACCURACY_AVERAGE; j++) {
                clock_gettime(CLOCK_MONOTONIC, &start_time);
                //Piece of iteration of main loop in accumulation_rssi(...) : start
                iw_nl80211_get_linkstat(&wifi_link_info, name_interface);
                (ptr_wifi_link_info_tiny + i)->rx_packets = wifi_link_info.rx_packets;
                (ptr_wifi_link_info_tiny + i)->signal     = wifi_link_info.signal;
                i++;
                //Piece of iteration of main loop in accumulation_rssi(...) : finish
                clock_gettime(CLOCK_MONOTONIC, &finish_time);
                get_diff_timespec(&iteration_without_delay, &start_time, &finish_time);
                elapsed_time_ns += iteration_without_delay.tv_sec * 1.0e+9 + iteration_without_delay.tv_nsec;
        }
        iteration_without_delay.tv_sec  = 0;
        iteration_without_delay.tv_nsec = lround(elapsed_time_ns / ACCURACY_AVERAGE);
        printf("info: estimated time of main loop's iteration without delay: sec=%ld, nsec=%ld\n",
               iteration_without_delay.tv_sec, iteration_without_delay.tv_nsec);

        //Calculate delay
        if (ideal_delay.tv_sec == 1) {
                real_delay->tv_sec  = 0;
                real_delay->tv_nsec = ideal_delay.tv_nsec - iteration_without_delay.tv_nsec + 1000000000; //1e+9
        } else {
                real_delay->tv_sec  = ideal_delay.tv_sec  - iteration_without_delay.tv_sec;
                real_delay->tv_nsec = ideal_delay.tv_nsec - iteration_without_delay.tv_nsec;
        }
        printf("info: real time of main loop's iteration (with delay): sec=%ld, nsec=%ld\n",
               real_delay->tv_sec, real_delay->tv_nsec);
}
