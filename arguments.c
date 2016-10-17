#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <getopt.h>
#include "arguments.h"


int handle_arguments(const int argc, char *argv[], struct arguments *config)
{
        int opt, longind;
        uint_least32_t is_all_arguments = 0;
        const struct option longopts[] = {
                {"interface", required_argument, NULL, 'i'},
                {"file",      required_argument, NULL, 'f'},
                {"time",      required_argument, NULL, 't'},
                {"speed",     required_argument, NULL, 's'},
                {NULL,        0,                 NULL,  0 }
        };

        extern int opterr;
        opterr = 0;

        while ( (opt = getopt_long(argc, (char * const *)argv, "i:f:t:s:", longopts, &longind)) != -1 ) {
                switch (opt) {
                        case 'i': config->interface = optarg; is_all_arguments |= 0x0001; break;
                        case 'f': config->file = optarg;      is_all_arguments |= 0x0010; break;
                        case 't': config->time.str = optarg;  is_all_arguments |= 0x0100; break;
                        case 's': config->speed.str = optarg; is_all_arguments |= 0x1000; break;
                        default : return 1;
                }
        }

        return (is_all_arguments == 0x1111) ? 0 : 1;
}

int conv_arguments(struct arguments * config)
{
        //config.time and config.speed: [char *] to [int]
        config->time.integer = atoi(config->time.str);
        if (config->time.integer <= 0) {
                fprintf(stderr, "user error: accumulation time - is not positive integer\n");
                return 1;
        }

        config->speed.integer = atoi(config->speed.str);
        if (config->speed.integer <= 0) {
                fprintf(stderr, "user error: accumulation speed - is not positive integer\n");
                return 1;
        }

        return 0;
}

void printf_help(void)
{
	printf("Accumulation-rssi v0.2 2016. Questions and bugs send to <h3pr5tq@gmail.com>\n"
               "Executes accumulation of wifi rssi value (signal strength of access point), saving them in text file\n"
               "NOTE: the association with an access point is required condition\n\n"
	       "Usage:\n"
	       " accumulation-rssi  -i <interface name>  -f <file name>  -t <accumulation time>  -s <accumulation speed>\n"
	       "Required arguments:\n"
	       " -i,                            Wifi interface name\n"
	       " --interface=<wlan0>            For found out the name, use iw or iwconfig\n\n"

	       " -f,                            Path to file,\n"
	       " --file=<~/wifi/rssi.txt>       where will save accumulation rssi value\n\n"

	       " -t,                            Accumulation time in sec (positive integer)\n"
	       " --time=<300>                   Approximately correspond program's runtime\n\n"

	       " -s,                            Accumulation speed in\n"
	       " --speed=<30>                   number of rssi value per second (positive integer)\n\n"
	       "Example:\n"
	       " accumulation-rssi -i wlan0 -f ~/rssi.txt -t 100 -s 10\n");
}

void printf_arguments(const struct arguments *config)
{
        printf("Your configuration:\n"
               "  wifi interface name:       %s\n"
               "  file for save rssi value:  %s\n"
               "  accumulation time:         %d (sec)\n"
               "  accumulation speed:        %d (rssi value per second)\n",
               config->interface, config->file, config->time.integer, config->speed.integer);
}
