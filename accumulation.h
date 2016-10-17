#ifndef ACCUMULATION_H
#define ACCUMULATION_H

#include <stdint.h>
#include <time.h>

#include "arguments.h"

/*
Number of numbers for calculating the average time for one iteration
Use in get_time_iteration()
*/
#define ACCURACY_AVERAGE 10000


struct iw_nl80211_linkstat_tiny {
        uint32_t rx_packets;
        int8_t   signal;
};

void accumulation_rssi(const struct arguments *config);

//Calculate ideal-theor delay of iteration of main loop in accumulation_rssi(...)
void get_ideal_delay(struct timespec *ideal_delay, const int accumualtion_speed);

void get_diff_timespec(struct timespec *diff_time, const struct timespec *start_time, const struct timespec *finish_time);

//Estimate real delay of iteration of main loop in accumulation_rssi(...)
void get_real_delay(struct timespec *real_delay, const char * const name_interface, const int accumualtion_speed);


#endif
