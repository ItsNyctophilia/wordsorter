#include "sort.h"

int ascii_sort(const void *str_1, const void *str_2)
{
	// Syntax from stackoverflow.com/a/5484006 from jxh
	return strcmp(*(char *const *)str_1, *(char *const *)str_2);
}

int len_sort(const void *str_1, const void *str_2)
{
	return (0);
}

int num_sort(const void *str_1, const void *str_2)
{
	return (0);
}

int scrabble_sort(const void *str_1, const void *str_2)
{
	return (0);

}
