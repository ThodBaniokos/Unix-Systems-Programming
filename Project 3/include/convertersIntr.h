// file: convertersIntr.h
// interface of the used converters
// include guard
#pragma once

// custom headers below
#include "Types.h"

// int related converters
char *int_to_string(const int num);

// bloom filter related converters
char *blm_to_string(blm to_convert, int blm_size);
blm string_to_blm(char *bloom_str);