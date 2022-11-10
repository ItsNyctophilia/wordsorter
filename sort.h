#ifndef SORT_H
#define SORT_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int ascii_sort(const void *str_1, const void *str_2);

int insensitive_ascii_sort(const void *str_1, const void *str_2);

int len_sort(const void *str_1, const void *str_2);

int num_sort(const void *str_1, const void *str_2);

int scrabble_sort(const void *str_1, const void *str_2);

int scrabble_sort_helper(const void *str);

#endif
