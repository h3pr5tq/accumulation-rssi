#include <stdio.h>
#include <stdlib.h>

#include "arguments.h"
#include "accumulation.h"

int main(int argc, char *argv[])
{
        struct arguments config;

        if (handle_arguments(argc, argv, &config)) {
                printf_help();
                exit(EXIT_FAILURE);
        }

        if (conv_arguments(&config)) {
                printf_help();
                exit(EXIT_FAILURE);
        }

        printf_arguments(&config);

        accumulation_rssi(&config);

        return 0;
}
