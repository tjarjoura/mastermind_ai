#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "logging.h"

/* GLOBAL STATE */
char **exact_strings;
int n_exact_strings = 0;
char **misplaced_strings;
int n_misplaced_strings = 0;

int *exact_counts;
int misplaced_counts[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int set_indices[10]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int disqualified[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int used[10]         = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int n_digits, digit_max, n_turns;
long max_time;

int best_exact, best_misplaced;

/* Add string to list of 'exact strings' */
void add_exact_string(char *str)
{
	strncpy(exact_strings[n_exact_strings++], str, n_digits);
}

void add_misplaced_string(char *str)
{
	strncpy(misplaced_strings[n_misplaced_strings++], str, n_digits);
}

void update_misplaced_counts()
{
	int i, j, digit;

	for (i = 0; i < n_misplaced_strings; i++)
		for (j = 0; j < n_digits; j++) {
			digit = misplaced_strings[i][j] - '0';
			misplaced_counts[digit]++;			
		}
}

/* Update the counts of digits and positions in the exact strings */
void update_exact_counts()
{
	int i, j, digit;

	memset(exact_counts, 0x00, 10 * n_digits * sizeof(int));

	for (i = 0; i < n_exact_strings; i++)
		for (j = 0; j < n_digits; j++) {
			digit = exact_strings[i][j] - '0';
			exact_counts[(j * 10) + digit]++;
		}
}

/* Insert a value into an array, knocking the end one off */
void insert(int *array, ssize_t sz, int pos, int val)
{
	int i;

	for (i = (sz - 1); i > pos; i--)
		array[i] = array[i - 1];

	array[pos] = val;
}

/* Find the best_exact highest counts */
void find_highest_exact(int *highest_idx)
{
	int i, j, cnt;
	int *highest_cnt = malloc(sizeof(int) * best_exact);
	memset(highest_cnt, 0x00, sizeof(int) * best_exact);
	
	for (i = 0; i < (n_digits * 10); i++) {
		cnt = exact_counts[i];
		for (j = 0; j < best_exact; j++) 
			if (cnt > highest_cnt[j]) {
				insert(highest_cnt, best_exact, j, cnt);
				insert(highest_idx, best_exact, j, i);
				break;
			}
	}

	free(highest_cnt);
}

/* Find the best_misplaced highest counts */
void find_highest_misplaced(int *highest_digits)
{
	int i, j, cnt;
	int *highest_cnt = malloc(sizeof(int) * best_exact);
	memset(highest_cnt, 0x00, sizeof(int) * best_exact);

	for (i = 0; i < 10; i++) {
		cnt = misplaced_counts[i];
		for (j = 0; j < best_misplaced; j++)
			if (cnt > highest_cnt[j]) {
				insert(highest_cnt, best_misplaced, j, cnt);
				insert(highest_digits, best_misplaced, j, i);
				break;
			}
	}

	free(highest_cnt);
}

void exact_logic(char *guess_buffer)
{
	int i, index, digit;
	int *highest_indices = malloc(sizeof(int) * best_exact);
	memset(highest_indices, 0x00, sizeof(int) * best_exact);
	find_highest_exact(highest_indices);

	for (i = 0; i < best_exact; i++) {
		index = highest_indices[i] / 10;
		digit = highest_indices[i] % 10;
	
		if (!disqualified[digit] && !used[digit]) {
			guess_buffer[index] = digit + '0';
			set_indices[index] = 1;
			used[digit] = 1;
		}
	}

	free(highest_indices);
}

void misplaced_logic(char *guess_buffer)
{
	int i, guess_val;
	int *highest_digits = malloc(sizeof(int) * best_misplaced);
	memset(highest_digits, 0x00, sizeof(int) * best_misplaced);
	find_highest_misplaced(highest_digits);

	for (i = 0; i < n_digits; i++) {
		if (set_indices[i])
		       continue;

		guess_val = highest_digits[rand() % best_misplaced];
		if (!disqualified[guess_val] && !used[guess_val]) {
			guess_buffer[i] = guess_val + '0';
			used[guess_val] = 1;
			set_indices[i] = 1;
		}
	}
}

void new_guess(char *guess_buffer)
{
	int guess_val, i;
	
	if (best_exact != 0)
		exact_logic(guess_buffer); /* use knowledge of strings with exactly placed digits */

	if (best_misplaced != 0)
		misplaced_logic(guess_buffer); /* use knowledge of strings with misplaced but correct digits */
	
	/* semi-random guesses for the other parts */
	for (i = 0; i < n_digits; i++) {
		if (set_indices[i])
			continue;
			
		do {		
			guess_val = rand() % (digit_max + 1);
		} while(disqualified[guess_val] || used[guess_val]);

		guess_buffer[i] = guess_val + '0';
		used[guess_val] = 1;
	}
}

/* Generate a random number for the first guess */ 
void random_guess(char* guess_buffer) 
{
	int guess_val,  i;

	for (i = 0; i < n_digits; i++) {
		do {
			guess_val = rand() % (digit_max + 1);
		} while (used[guess_val]);
		
		guess_buffer[i] = guess_val + '0';
		used[guess_val] = 1;
	}
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	set_log_file("mastermind.log");

	int c_exact, c_misplaced;
	int turn = 1;
	char guess_buffer[10];
	int i;

	scanf("%d %d %d %ld", &digit_max, &n_digits, &n_turns, &max_time);

	for (i = digit_max; i < 10; i++)
		disqualified[i] = 1;
	
	exact_strings = malloc(sizeof(char *) * n_turns);
	for (i = 0; i < n_turns; i++)
		exact_strings[i] = malloc(n_digits);

	misplaced_strings = malloc(sizeof(char *) * n_turns);
	for (i = 0; i < n_turns; i++)
		misplaced_strings[i] = malloc(n_digits);

	exact_counts = malloc(sizeof(int *) * n_digits * 10);

	best_exact = best_misplaced = 0;

	/* MAIN LOOP */

	while (turn <= n_turns) {
		memset(used, 0x00, sizeof(int) * 10);
		memset(set_indices, 0x00, sizeof(int) * 10);

		if (turn == 1) 
			random_guess(guess_buffer);
		else
			new_guess(guess_buffer);
		
		printf("Guess: %s\n", guess_buffer);
		scanf("%d %d", &c_exact, &c_misplaced);
		
		if (c_exact == n_digits)
			break;
		
		if ((c_exact == 0) && (c_misplaced == 0))
			for (i = 0; i < n_digits; i++) 
				disqualified[guess_buffer[i] - '0'] = 1;

		else if (c_exact + c_misplaced == n_digits) {
			for (i = 0; i <= digit_max; i++)
			       disqualified[i] = 1;	
			for (i = 0; i < n_digits; i++)
				disqualified[guess_buffer[i] - '0'] = 0;
		}

		if (c_exact > 0)
			add_exact_string(guess_buffer);
		if (c_misplaced > 0)
			add_misplaced_string(guess_buffer);
		
		if (c_exact >= best_exact || ((c_exact + c_misplaced) >= (best_exact + best_misplaced))) {
			best_exact = c_exact;
			best_misplaced = c_misplaced;
		}

		update_exact_counts();
		turn++;
	}

	if (turn <= n_turns)
		printf("Solved in %d turns!\n", turn);
	else
		printf("Failed to solve!\n");
	
	for (i = 0; i < n_turns; i++) {
		free(exact_strings[i]);
		free(misplaced_strings[i]);	
	}

	free(exact_counts);
	free(exact_strings);
	free(misplaced_strings);
	return 0;
}
