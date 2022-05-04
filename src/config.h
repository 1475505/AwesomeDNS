# ifndef CONFIG_H
# define CONFIG_H

#include <stdint.h>

void config(int argc, char* argv[]);
uint32_t findIP(char* name, uint8_t* found);

#endif