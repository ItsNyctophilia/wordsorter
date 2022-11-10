#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

char **load_words(char **input_files, size_t count_files);

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
				printf("Succesfully opened %s\n", argv[i]);
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
		load_words(args, argc);
		free(args);
		// TODO: Create struct that holds words
		// plus size of words[]
		
		//for (size_t i = 0; i < words_len; ++i) {
		//	printf("%s\n", words[i]);
		//}
		//for (size_t i = 0; i < words_len; ++i) {
		//	free(words[i]);
		//}

	}
	return (SUCCESS);
}

char **load_words(char **input_files, size_t count_files)
{

	// TODO: iterate through all files once basic
	// functionality established for single file
	char **words;
	size_t words_len = 0;
	size_t current_max = DEFAULT_WORD_COUNT;
	words = malloc(DEFAULT_WORD_COUNT * sizeof(*words));
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
					puts("Fatal allocation error");
					exit(MEMORY_ERROR);
				}
				current_max *= 2;
				printf("\nRealloc'd %zu\n",
				       2 * current_max * sizeof(*words));
				words = tmp;
			}
			if (current_word) {
				current_word_stored =
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
						puts("Fatal allocation error");
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
	return(words);
}
