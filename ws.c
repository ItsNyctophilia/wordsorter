#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

enum return_codes {
	SUCCESS = 0,
	INVOCATION_ERROR = 1
};

static struct {
	int top_count;
	int bottom_count;
	bool top_to_bottom;	// Applicable only if both -c 
	// and -C are passed indicates the order is to prune 
	//from the top first, then from the bottom if true, 
	// else reversed        
	bool ascii_sort;	// Default sorting algorithm
	bool num_sort;
	bool len_sort;
	bool scrabble_sort;
	bool reversed;
	bool unique;

} options = { 0, 0, true, true, false, false, false, false, false };

int main(int argc, char *argv[])
{
	int opt;
	char *err = '\0';

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
	return (SUCCESS);
}
