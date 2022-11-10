#include "sort.h"

int ascii_sort(const void *str_1, const void *str_2)
// Sort two strings by ASCII codepoint in ascending order
{
	// Syntax from stackoverflow.com/a/5484006 from jxh
	return (strcmp(*(char *const *)str_1, *(char *const *)str_2));
}

int len_sort(const void *str_1, const void *str_2)
// Sort two strings by length in ascending order.
{
	size_t len_1 = strlen(*(char *const *)str_1);
	size_t len_2 = strlen(*(char *const *)str_2);
	if (len_1 == len_2) {
		return (0);
	}
	return (len_1 > len_2 ? 1 : -1);
}

int num_sort(const void *str_1, const void *str_2)
// Sorts two strings as if they were numbers in
// ascending order, ignoring values after the first
// non-digit character in a given string.
{
	long int num_1 = strtol(*(char *const *)str_1, NULL, 0);
	long int num_2 = strtol(*(char *const *)str_2, NULL, 0);
	if (num_1 == num_2) {
		return (0);
	}
	return (num_1 > num_2 ? 1 : -1);
}

int scrabble_sort(const void *str_1, const void *str_2)
{
	return (0);

}
