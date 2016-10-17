#ifndef ARGUMENTS_H
#define ARGUMENTS_H

union str_or_int {
        const char *str;
        int        integer;
};

/*
 * struct arguments - input parameters of program
 * @interface - wifi interface name
 * @file - file name, where save accumulation rssi value
 * @time - accumulation time in sec
 * @speed - accumulation speed in number of rssi value per second
 */
struct arguments {
        const char *interface;
        const char *file;
        union str_or_int time;
        union str_or_int speed;
};

/* handle options and arguments of command line,
 * check that all necessary options is present,
 * return 0 if all is right,else return 1
 */
int handle_arguments(const int argc, char *argv[], struct arguments *config);

int conv_arguments(struct arguments *config);

void printf_help(void);
void printf_arguments(const struct arguments * config);

#endif
