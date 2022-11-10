#include "sort.h"

int ascii_sort(const void *str_1, const void *str_2)
// Sort two strings by ASCII codepoint in ascending order
{
	// Syntax from stackoverflow.com/a/5484006 from jxh
	return (strcmp(*(char *const *)str_1, *(char *const *)str_2));
}

int insensitive_ascii_sort(const void *str_1, const void *str_2)
// Sort two strings by ASCII codepoint in ascending order,
// case insensitively
{
	return (strcasecmp(*(char *const *)str_1, *(char *const *)str_2));
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
	long int num_1 = strtol(*(char *const *)str_1, NULL, 10);
	long int num_2 = strtol(*(char *const *)str_2, NULL, 10);
	if (num_1 == num_2) {
		return (0);
	}
	return (num_1 > num_2 ? 1 : -1);
}

int scrabble_sort(const void *str_1, const void *str_2)
{
	int num_1 = scrabble_sort_helper(str_1);
	int num_2 = scrabble_sort_helper(str_2);
	if (num_1 == num_2) {
		return (0);
	}
	return (num_1 > num_2 ? 1 : -1);
}

int scrabble_sort_helper(const void *str)
{
	int num_tiles[26] = { 9, 2, 2, 4, 12, 2, 3, 2, 9, 1,
		1, 4, 2, 6, 8, 2, 1, 6, 4, 6,
		4, 2, 2, 1, 2, 1
	};			// Number of tiles for each letter in the alphabet, in order
	int score = 0;
	for (size_t i = 0; i < strlen(*(char *const *)str); ++i) {
		char tmp = tolower(*(*(char *const *)str + i));
		if (!isalpha(tmp)) {
			continue;
		}
		int alpha_index = tmp - 'a';
		if (num_tiles[alpha_index] > 0) {
			--num_tiles[alpha_index];
		} else {
			continue;
		}
		if (strchr("aeilnorstu", tmp)) {
			score += 1;
			continue;
		}
		if ('d' == tmp || 'g' == tmp) {
			score += 2;
			continue;
		}
		if (strchr("bcmp", tmp)) {
			score += 3;
			continue;
		}
		if (strchr("fhvwy", tmp)) {
			score += 4;
			continue;
		}
		if ('k' == tmp) {
			score += 5;
			continue;
		}
		if ('j' == tmp || 'x' == tmp) {
			score += 8;
			continue;
		}
		if ('q' == tmp || 'z' == tmp) {
			score += 10;
			continue;
		}
	}
	printf("%d<-- score word -->%s\n", score, *(char *const *)str);
	return (score);
}
