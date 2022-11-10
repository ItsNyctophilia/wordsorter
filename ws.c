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
	DEFAULT_WORD_COUNT = 32
};

static struct {
	int top_count;
	int bottom_count;
	bool top_to_bottom;	// Applicable only if both -c 
	// and -C are passed indicates the order is to prune 
	// from the top first, then from the bottom if true, 
	// else reversed        
	bool ascii_sort;	// Default sorting algorithm
	bool num_sort;
	bool len_sort;
	bool scrabble_sort;
	bool reversed;
	bool unique;

} options = { 0, 0, true, true, false, false, false, false, false };

struct words_array {
	char **words;
	size_t words_len;
};

struct words_array *load_words(char **input_files, size_t count_files);

int main(int argc, char *argv[])
{
	int opt;
	char *err = '\0';

	// Option-handling syntax borrowed from Liam Echlin in
	// getopt-demo.c
	while ((opt = getopt(argc, argv, "ac:C:hlnrs")) != -1) {

		switch (opt) {
			// a[scii sort]
		case 'a':
			options.ascii_sort = true;
			options.len_sort = false;
			options.scrabble_sort = false;
			options.num_sort = false;
			break;
			// l[ength sort]
		case 'l':
			options.ascii_sort = false;
			options.len_sort = true;
			options.scrabble_sort = false;
			options.num_sort = false;
			break;
			// s[crabble sort]
		case 's':
			options.ascii_sort = false;
			options.len_sort = false;
			options.scrabble_sort = true;
			options.num_sort = false;
			break;
			// n[umerical sort]
		case 'n':
			options.ascii_sort = false;
			options.len_sort = false;
			options.scrabble_sort = false;
			options.num_sort = true;
			break;
			// c[ount from top]
		case 'c':
			// TODO: Validate count < len(words)
			err = '\0';
			options.top_count = strtol(optarg, &err, 10);
			if (*err) {
				fprintf(stderr, "%s is not a number.\n",
					optarg);
				exit(INVOCATION_ERROR);
			}
			options.top_to_bottom = false;
			break;
			// C[ount from bottom]
		case 'C':
			// TODO: Validate count < len(words)
			err = '\0';
			options.bottom_count = strtol(optarg, &err, 10);
			if (*err) {
				fprintf(stderr, "%s is not a number.\n",
					optarg);
				exit(INVOCATION_ERROR);
			}
			options.top_to_bottom = true;
			break;
		case 'h':
			// TODO: create generic /? message
			printf("Usage: %s [OPTION]... [FILE]...\n", argv[0]);
			exit(SUCCESS);
		}

	}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		bool close_flag = false;
		for (int i = 0; i < argc; ++i) {
			FILE *fo = fopen(argv[i], "r");
			if (!fo) {
				close_flag = true;	// Close after all files checked
				fprintf(stderr, "%s could not be opened",
					argv[i]);
				perror(" \b");	// Backspace to format perror string
			} else {
				fclose(fo);
			}
		}
		if (close_flag == true) {
			exit(INVOCATION_ERROR);
		}
		char **args;
		// TODO: Error-handle malloc call
		args = malloc(argc * sizeof(*args));

		for (int i = 0; i < argc; ++i) {
			args[i] = argv[i];
		}
		struct words_array *current_array = load_words(args, argc);
		free(args);
		for (size_t i = 0; i < current_array->words_len; ++i) {
			printf("%s\n", current_array->words[i]);
		}		// DEVPRINT
		printf("words_len: %zu\n", current_array->words_len);	// DEVPRINT

		if (current_array->words_len) {
			// ASCII SORT TEST
			qsort(current_array->words, current_array->words_len,
			      sizeof(*(current_array->words)), len_sort);
		}
		putchar('\n');
		for (size_t i = 0; i < current_array->words_len; ++i) {
			printf("%s\n", current_array->words[i]);
		}
		// TODO: Logical sorting based on options
		// Free all allocated memory to current_array
		for (size_t i = 0; i < current_array->words_len; ++i) {
			free(current_array->words[i]);
		}
		free(current_array->words);
		free(current_array);
	}
	return (SUCCESS);
}

struct words_array *load_words(char **input_files, size_t count_files)
// Iterates through each file passed to it, tokenizing individual
// words on any whitespace character and returning a pointer to a
// struct words_array with pointers to strings and a count of 
// total words populated.
{
	// TODO: Error handle malloc call
	char **words = malloc(DEFAULT_WORD_COUNT * sizeof(*words));
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
				printf("\nRealloc'd %zu\n",
				       2 * current_max * sizeof(*words));
				words = tmp;
			}
			if (current_word) {
				current_word_stored =
				    // TODO: Error-handle malloc call
				    malloc((strlen(current_word) +
					    1) * sizeof(*current_word));
				strcpy(current_word_stored, current_word);
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
					// TODO: error-handle calloc call
					current_word_stored =
					    calloc(strlen(current_word)
						   + 1, sizeof(char));

					strcpy(current_word_stored,
					       current_word);
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
	//TODO: Error handle malloc call
	struct words_array *current_array = malloc(sizeof(*current_array));
	current_array->words_len = words_len;
	current_array->words = words;
	return (current_array);
}
