#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define DEBUG 1

#if DEBUG
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif

using Byte = uint8_t;
using Word = uint16_t;

#define BIT0 0b00000001
#define BIT1 0b00000010
#define BIT2 0b00000100
#define BIT3 0b00001000
#define BIT4 0b00010000
#define BIT5 0b00100000
#define BIT6 0b01000000
#define BIT7 0b10000000

#endif
