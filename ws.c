#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sort.h"

enum return_codes {
	SUCCESS = 0,
	INVOCATION_ERROR = 1,
	FILE_ERROR = 2,
	MEMORY_ERROR = 3
};

enum buffer_sizes {
	DEFAULT_WORD_COUNT = 32 // Arbitrary starting buffer size for words
	// array
};

static struct {
	int (*algorithm)(const void *, const void *);
	size_t top_count; // n from bottom of sort
	size_t bottom_count; // n from bottom of sort
	bool top_to_bottom;	// Applicable only if both -c 
	// and -C are passed indicates the order is to prune 
	// from the top first, then from the bottom if true, 
	// else reversed
	bool top_flag;
	bool bottom_flag;
	bool case_insens;
	bool scrabble_validation;
	bool reversed;
	bool unique;
} options =
    { ascii_sort, 0, 0, true, false, false, false, false, false, false };

struct words_array {
	char **words;
	size_t words_len;
};

struct words_array *load_words(char **input_files, size_t count_files);
struct words_array *load_words_interactively(void);
void prune_scrabble_words(struct words_array *current_array);
void prune_num_words(struct words_array *current_array, size_t num_from_top,
		     size_t num_from_bottom, bool top_to_bottom, bool top_flag,
		     bool bottom_flag);
void prune_duplicates(struct words_array *current_array, bool case_sensitive);
void resize_array(struct words_array *current_array);

int main(int argc, char *argv[])
{
	int opt;
	char *err = '\0';
	// Option-handling syntax borrowed from Liam Echlin in
	// getopt-demo.c
	while ((opt = getopt(argc, argv, "ac:C:hilnrsSu")) != -1) {

		switch (opt) {
			// a[scii sort]
		case 'a':
			options.algorithm = ascii_sort;
			if (options.case_insens == true) {
				options.algorithm = insensitive_ascii_sort;
			}
			break;
			// i[nsensitive ascii sort]
		case 'i':
			options.case_insens = true;
			if (options.algorithm == ascii_sort) {
				options.algorithm = insensitive_ascii_sort;
			}
			break;
			// l[ength sort]
		case 'l':
			options.algorithm = len_sort;
			break;
			// s[crabble sort w/o validation]
		case 's':
			options.algorithm = scrabble_sort;
			break;
			// S[crabble sort w/ validation]
		case 'S':
			options.algorithm = scrabble_sort;
			options.scrabble_validation = true;
			break;
			// n[umerical sort]
		case 'n':
			options.algorithm = num_sort;
			break;
			// u[nique]
		case 'u':
			options.unique = true;
			break;
			// r[everse order]
		case 'r':
			options.reversed = !(options.reversed);
			break;
			// c[ount from top]
		case 'c':
			err = '\0';
			options.top_count = strtol(optarg, &err, 10);
			if (*err) {
				fprintf(stderr, "%s is not a number.\n",
					optarg);
				exit(INVOCATION_ERROR);
			}
			options.top_to_bottom = false;
			options.top_flag = true;
			break;
			// C[ount from bottom]
		case 'C':
			err = '\0';
			options.bottom_count = strtol(optarg, &err, 10);
			if (*err) {
				fprintf(stderr, "%s is not a number.\n",
					optarg);
				exit(INVOCATION_ERROR);
			}
			options.top_to_bottom = true;
			options.bottom_flag = true;
			break;
			// h[elp message]
		case 'h':
			printf("Usage: %s [OPTION]... [FILE]...\n", argv[0]);
			puts("Sort strings from FILE(s) and print to standard output."
				 "\nWith no file, read standard input.\n\n"
				 "Sorting algorithms:\n\n"
				 "  -a,          ASCII codepoint sort\n" 
				 "  -l,          Length-of-word sort\n" 
				 "  -n,          Numerical sort\n" 
				 "  -s,          Scrabble-score sort\n" 
				 "  -S,          Scrabble-score sort, removing invalid words\n\n"
				 "Other options:\n\n" 
				 "  -u,          Display only unique words\n" 
				 "  -i,          Case insensitive sort\n" 
				 "  -c NUM,      Prints only first NUM lines from sorted output.\n" 
				 "  -C NUM,      Prints only last NUM lines from sorted output.\n" 
				 "               When -c and -C are combined, operations are applied\n" 
				 "                 in order, for example, -c 20 -C 4 prints the last\n" 
				 "                 4 of the first 20 sorted words.\n" 
				 "  -h           Display this help message and exit.\n\n" 
				 "Examples:\n" 
				 "  ws -i -u [FILE]   Print contents of FILE, removing duplicate\n" 
				 "                      words, case-insensitively.\n" 
				 "  ws -l             Sorts from standard input by length.");
			return (SUCCESS);
		case '?':
			return (INVOCATION_ERROR);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		// Validates that given files are able to be opened
		bool close_flag = false;
		for (int i = 0; i < argc; ++i) {
			FILE *fo = fopen(argv[i], "r");
			if (!fo) {	// If file could not be opened for reading
				close_flag = true;	// Close after all files checked
				fprintf(stderr, "%s could not be opened",
					argv[i]);
				perror(" \b");	// Backspace to format perror string
			} else {
				fclose(fo);
			}
		}
		if (close_flag == true) {
			return (INVOCATION_ERROR);
		}
	}

	struct words_array *current_array;
	if (argc > 0) {
		current_array = load_words(argv, argc);
	} else {
		current_array = load_words_interactively();
	}

	if (current_array->words_len) {
		// Case: Number of valid words across all files > 0

		qsort(current_array->words, current_array->words_len,
		      sizeof(*(current_array->words)), options.algorithm);

		if (options.scrabble_validation) {
			prune_scrabble_words(current_array);
			resize_array(current_array);
		}

		if (options.unique) {
			prune_duplicates(current_array, options.case_insens);
			resize_array(current_array);
		}

		if (options.top_flag || options.bottom_flag) {
			prune_num_words(current_array, options.top_count,
					options.bottom_count,
					options.top_to_bottom, options.top_flag,
					options.bottom_flag);
			resize_array(current_array);
		}

		// Print block
		if (!options.reversed) {
			// Case: Normal print
			for (size_t i = 0; i < current_array->words_len; ++i) {
				printf("%s\n", current_array->words[i]);
			}
		} else {
			// Case: Reversed print
			for (size_t i = current_array->words_len; i > 0; --i) {
				if (current_array->words[i]) {
				printf("%s\n", current_array->words[i]);
				}
			}
			printf("%s\n", current_array->words[0]);
		}
	} else {
		free(current_array->words);
		free(current_array);
		return (SUCCESS);
	}
	// Free all allocated memory to current_array
	for (size_t i = 0; i < current_array->words_len; ++i) {
		free(current_array->words[i]);
	}
	free(current_array->words);
	free(current_array);

	// Case: Valid words sorted
	return (SUCCESS);
}

struct words_array *load_words(char **input_files, size_t count_files)
// Iterates through each file passed to it, tokenizing individual
// words on any whitespace character and returning a pointer to a
// struct words_array with pointers to strings and a count of 
// total words populated.
{
	char **words = calloc(DEFAULT_WORD_COUNT, sizeof(*words));
	if (!words) {
		// Case: Out of memory
		fprintf(stderr, "Memory allocation error.\n");
		exit(MEMORY_ERROR);
	}
	size_t words_len = 0;
	size_t current_max = DEFAULT_WORD_COUNT;
	for (size_t i = 0; i < count_files; ++i) {
		FILE *fo = fopen(input_files[i], "r");
		if (!fo) {
			fprintf(stderr, "%s could not be opened",
				input_files[0]);
			perror(" \b");
			exit(FILE_ERROR);
		}
		char *line_buf = NULL;
		size_t buf_size = 0;
		while (getline(&line_buf, &buf_size, fo) != -1) {
			if (line_buf[0] == '\n') {
				continue;
			}
			char *current_word = strtok(line_buf, " \t\n\v\f\r");
			char *current_word_stored;
			if (words_len == current_max) {
				// Realloc syntax from Liam Echlin
				char **tmp = realloc(words,
						     (2 * current_max *
						      sizeof(*words)));
				if (!tmp) {
					for (size_t i = 0; i < words_len; ++i) {
						free(words[i]);
					}
					free(words);
					fprintf(stderr,
						"Memory allocation error.\n");
					exit(MEMORY_ERROR);
				}
				current_max *= 2;
				words = tmp;
			}
			if (current_word) {
				current_word_stored =
				    calloc((strlen(current_word) +
					    1), sizeof(*current_word));
				if (!current_word_stored) {
					// Case: Out of memory
					for (size_t i = 0; i < words_len; ++i) {
						free(words[i]);
					}
					free(words);
					fprintf(stderr,
						"Memory allocation error.\n");
					exit(MEMORY_ERROR);
				}
				strncpy(current_word_stored, current_word, strlen(current_word));
				words[words_len] = current_word_stored;
				++words_len;
			}

			while ((current_word =
				strtok(NULL, " \t\n\v\f\r")) != NULL) {
				// Reallocate memory if needed
				if (words_len == current_max) {
					char **tmp = realloc(words,
							     (2 *
							      current_max
							      *
							      sizeof(*words)));
					if (!tmp) {
						for (size_t i = 0;
						     i < words_len; ++i) {
							free(words[i]);
						}
						free(words);
						fprintf(stderr,
							"Memory allocation error.\n");
						exit(MEMORY_ERROR);
					}
					current_max *= 2;
					printf("\nRealloc'd %zu\n",
					       2 * current_max *
					       sizeof(*words));
					words = tmp;
				}
				current_word_stored = NULL;
				if (current_word) {
					current_word_stored =
					    calloc(strlen(current_word) + 1,
						   sizeof(char));
					if (!current_word_stored) {
						// Case: Out of memory
						for (size_t i = 0;
						     i < words_len; ++i) {
							free(words[i]);
						}
						free(words);
						fprintf(stderr,
							"Memory allocation error.\n");
						exit(MEMORY_ERROR);
					}

					strncpy(current_word_stored,
						current_word,
						strlen(current_word) +
						1);
					words[words_len] = current_word_stored;
					++words_len;
				}
			}
		}
		if (line_buf) {
			free(line_buf);
		}
		fclose(fo);
	}
	struct words_array *current_array = malloc(sizeof(*current_array));
	if (!current_array) {
		// Case: Out of memory
		for (size_t i = 0; i < words_len; ++i) {
			free(current_array->words[i]);
		}
		free(words);
		fprintf(stderr, "Memory allocation error.\n");
		exit(MEMORY_ERROR);
	}
	current_array->words_len = words_len;
	current_array->words = words;
	return (current_array);
}

struct words_array *load_words_interactively(void)
// This is functionally the same as load words, but accepting from
// stdin instead of from a file.
{
	char **words = calloc(DEFAULT_WORD_COUNT, sizeof(*words));
	if (!words) {
		// Case: Out of memory
		fprintf(stderr, "Memory allocation error.\n");
		exit(MEMORY_ERROR);
	}
	size_t words_len = 0;
	size_t current_max = DEFAULT_WORD_COUNT;

	char *line_buf = NULL;
	size_t buf_size = 0;
	while (getline(&line_buf, &buf_size, stdin) != -1) {
		if (line_buf[0] == '\n') {
			continue;
		}
		char *current_word = strtok(line_buf, " \t\n\v\f\r");
		char *current_word_stored;
		if (words_len == current_max) {
			// Realloc syntax from Liam Echlin
			char **tmp = realloc(words,
					     (2 * current_max *
					      sizeof(*words)));
			if (!tmp) {
				for (size_t i = 0; i < words_len; ++i) {
					free(words[i]);
				}
				free(words);
				fprintf(stderr, "Memory allocation error.\n");
				exit(MEMORY_ERROR);
			}
			current_max *= 2;
			words = tmp;
		}
		if (current_word) {
			current_word_stored = calloc((strlen(current_word) + 1), sizeof(*current_word));
			if (!current_word_stored) {
				// Case: Out of memory
				for (size_t i = 0; i < words_len; ++i) {
					free(words[i]);
				}
				free(words);
				fprintf(stderr, "Memory allocation error.\n");
				exit(MEMORY_ERROR);
			}
			strncpy(current_word_stored, current_word, strlen(current_word));
			words[words_len] = current_word_stored;
			++words_len;
		}

		while ((current_word = strtok(NULL, " \t\n\v\f\r")) != NULL) {
			// Reallocate memory if needed
			if (words_len == current_max) {
				char **tmp = realloc(words,
						     (2 *
						      current_max
						      * sizeof(*words)));
				if (!tmp) {
					for (size_t i = 0; i < words_len; ++i) {
						free(words[i]);
					}
					free(words);
					fprintf(stderr,
						"Memory allocation error.\n");
					exit(MEMORY_ERROR);
				}
				current_max *= 2;
				printf("\nRealloc'd %zu\n",
				       2 * current_max * sizeof(*words));
				words = tmp;
			}
			current_word_stored = NULL;
			if (current_word) {
				current_word_stored =
				    calloc(strlen(current_word)
					   + 1, sizeof(char));
				if (!current_word_stored) {
					// Case: Out of memory
					for (size_t i = 0; i < words_len; ++i) {
						free(words[i]);
					}
					free(words);
					fprintf(stderr,
						"Memory allocation error.\n");
					exit(MEMORY_ERROR);
				}

				strncpy(current_word_stored, current_word, strlen(current_word));
				words[words_len] = current_word_stored;
				++words_len;
			}
		}
	}
	if (line_buf) {
		free(line_buf);
	}
	struct words_array *current_array = malloc(sizeof(*current_array));
	if (!current_array) {
		// Case: Out of memory
		for (size_t i = 0; i < words_len; ++i) {
			free(current_array->words[i]);
		}
		free(words);
		fprintf(stderr, "Memory allocation error.\n");
		exit(MEMORY_ERROR);
	}
	current_array->words_len = words_len;
	current_array->words = words;
	return (current_array);
}

void prune_num_words(struct words_array *current_array, size_t num_from_top,
		     size_t num_from_bottom, bool top_to_bottom, bool top_flag,
		     bool bottom_flag)
// Prunes the top or bottom n numbers from the file based on the given
// -c and -C options by setting everything outside of those values to
// NULL.
{
	if (top_flag && bottom_flag) {
		if (num_from_top > current_array->words_len) {
			num_from_top = current_array->words_len;
		}
		if (num_from_bottom > current_array->words_len) {
			num_from_bottom = current_array->words_len;
		}
		if (top_to_bottom) {
			if (num_from_bottom > num_from_top) {
				num_from_bottom = num_from_top;
			}
			for (size_t i = num_from_top;
			     i < current_array->words_len; ++i) {
				// Set everything outside of first i elements to NULL
				free(current_array->words[i]);
				current_array->words[i] = NULL;
			}
			for (size_t i = 0; i < num_from_top - num_from_bottom;
			     ++i) {
				// Then set everything outside of last i elements to NULL
				free(current_array->words[i]);
				current_array->words[i] = NULL;
			}

		} else {
			if (num_from_top > num_from_bottom) {
				num_from_top = num_from_bottom;
			}
			for (size_t i = 0;
			     i < current_array->words_len - num_from_bottom;
			     ++i) {
				// Set everything outside of last i elements to NULL
				free(current_array->words[i]);
				current_array->words[i] = NULL;
			}
			for (size_t i =
			     current_array->words_len - num_from_bottom +
			     num_from_top; i < current_array->words_len; ++i) {
				// Then set everything outside of first i elements to NULL
				free(current_array->words[i]);
				current_array->words[i] = NULL;
			}
		}
	}
	if (top_flag && !bottom_flag) {
		if (num_from_top > current_array->words_len) {
			num_from_top = current_array->words_len;
		}
		for (size_t i = num_from_top; i < current_array->words_len; ++i) {
			// Set everything outside of first i elements to NULL
			free(current_array->words[i]);
			current_array->words[i] = NULL;
		}
	}
	if (bottom_flag && !top_flag) {
		if (num_from_bottom > current_array->words_len) {
			num_from_bottom = current_array->words_len;
		}
		for (size_t i = 0;
		     i < current_array->words_len - num_from_bottom; ++i) {
			// Set everything outside of last i elements to NULL
			free(current_array->words[i]);
			current_array->words[i] = NULL;
		}
	}
}

void resize_array(struct words_array *current_array)
// Allocates space for a new array of words, copies all
// non-null words into it and then resizes it. Modifies
// the passed argument in-place. This MUST be called after
// any of the prune functions to remove the null pointers.
{
	char **tmp_words =
	    calloc(current_array->words_len, sizeof(*current_array->words));
	if (!tmp_words) {
		// Case: Out of memory
		for (size_t i = 0; i < current_array->words_len; ++i) {
			free(current_array->words[i]);
		}
		free(current_array->words);
		free(current_array);
		fprintf(stderr, "Memory allocation error.\n");
		exit(MEMORY_ERROR);
	}
	size_t new_size = 0;
	for (size_t cur_word = 0; cur_word < current_array->words_len;
	     ++cur_word) {
		if (current_array->words[cur_word]) {
			char *tmp_cur_word =
			    calloc((strlen(current_array->words[cur_word]) + 1),
				   sizeof(*current_array->words[cur_word]));
			if (!tmp_cur_word) {
				// Case: Out of memory
				for (size_t i = 0; i < current_array->words_len;
				     ++i) {
					free(current_array->words[i]);
				}
				free(tmp_words);
				free(current_array->words);
				free(current_array);
				fprintf(stderr, "Memory allocation error.\n");
				exit(MEMORY_ERROR);
			}
			strncpy(tmp_cur_word, current_array->words[cur_word],
				strlen(current_array->words[cur_word]));
			tmp_words[new_size] = tmp_cur_word;
			++new_size;
		} else {
			free(current_array->words[cur_word]);
		}
	}
	char **tmp = realloc(tmp_words, new_size * sizeof(*tmp_words));
	if (!tmp) {
		// Case: Out of memory
		for (size_t i = 0; i < current_array->words_len; ++i) {
			free(current_array->words[i]);
		}
		free(current_array->words);
		free(tmp_words);
		free(current_array);
		fprintf(stderr, "Memory allocation error.\n");
		exit(MEMORY_ERROR);
	}
	// Free old contents of current_array
	for (size_t i = 0; i < current_array->words_len; ++i) {
		free(current_array->words[i]);
	}
	free(current_array->words);

	current_array->words_len = new_size;
	current_array->words = tmp;
	return;
}

void prune_duplicates(struct words_array *current_array, bool case_insensitive)
// Prunes the duplicate words from the array for the purposes of the -u
// option.
{
	for (size_t anchor_word = 0; anchor_word < current_array->words_len;
	     ++anchor_word) {
		for (size_t word = (anchor_word + 1);
		     word < current_array->words_len; ++word) {
			if (!(current_array->words[word]
			      && current_array->words[anchor_word])) {
				continue;
			}
			if (!case_insensitive) {
				if (!
				    (strncmp
				     (current_array->words[anchor_word],
				      current_array->words[word],
				      strlen(current_array->words[anchor_word])
				      + 1))) {
					free(current_array->words[word]);
					current_array->words[word] = NULL;
					continue;
				}
			} else {
				if (!
				    (strncasecmp
				     (current_array->words[anchor_word],
				      current_array->words[word],
				      strlen(current_array->words[anchor_word])
				      + 1))) {
					free(current_array->words[word]);
					current_array->words[word] = NULL;
					continue;
				}
			}
		}
	}
}

void prune_scrabble_words(struct words_array *current_array)
// Removes Scrabble-invalid words from the current_array argument
// by freeing the allocated string and setting the pointer to NULL.
// No return value, modifies the given struct words_array in-place.
{
	for (size_t word = 0; word < current_array->words_len; ++word) {
		int num_tiles[26] = { 9, 2, 2, 4, 12, 2, 3, 2, 9, 1,
			1, 4, 2, 6, 8, 2, 1, 6, 4, 6,
			4, 2, 2, 1, 2, 1
		};		// Number of Scrabble tiles per letter in the alphabet
		int blank_tiles = 2;
		for (size_t chr = 0; chr < strlen(current_array->words[word]);
		     ++chr) {
			// tmp is the lowercase version of chr in word
			char tmp = tolower(current_array->words[word][chr]);
			if (!isalpha(tmp)) {
				// Case: word contains invalid characters
				free(current_array->words[word]);
				current_array->words[word] = NULL;
				break;
			}
			// alpha_index becomes the index from 0 - 25 in the
			// alphabet for the purposes of indexing into num_tiles
			int alpha_index = tmp - 'a';
			if (num_tiles[alpha_index] > 0) {
				--num_tiles[alpha_index];
			} else if (blank_tiles > 0) {
				--blank_tiles;
			} else {
				// Case: word exceeds valid tile allotment
				free(current_array->words[word]);
				current_array->words[word] = NULL;
				break;
			}
		}
	}
	return;
}