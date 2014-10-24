#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"

/* GLOBAL STATE */
char **exact_strings;
int n_strings = 0;
char disqualified[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int *counters;
int n_digits, digit_max, n_turns;
long max_time;
int best_exact, best_misplaced;

/* Add string to list of 'exact strings' */
int add_exact_string(char *str)
{
	strncpy(exact_strings[n_strings++], str, n_digits);
}

/* Update the counts of digits and positions in the exact strings */
void update_counters()
{
	int i, j, digit;

	memset(counters, 0x00, 10 * n_digits * sizeof(int));

	for (i = 0; i < n_strings; i++)
		for (j = 0; j < n_digits; j++) {
			digit = exact_strings[i][j] - '0';
			counters[(j * 10) + digit]++;
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
void find_highest(int *highest_idx)
{
	int i, j, cnt;
	int *highest_cnt = malloc(sizeof(int) * best_exact);
	memset(highest_cnt, 0x00, sizeof(int) * best_exact);
	
	for (i = 0; i < (n_digits * 10); i++) {
		cnt = counters[i];
		for (j = 0; j < best_exact; j++) 
			if (cnt > highest_cnt[j]) {
				insert(highest_cnt, best_exact, j, cnt);
				insert(highest_idx, best_exact, j, i);
				break;
			}
	}

	free(highest_cnt);
}

/* Check that no digits repeat or exceed the max_digit */
int valid(char *guess) 
{
	int i, j;

	for (i = 0; i < n_digits; i++) {
		if ((guess[i] - '0') > digit_max)
			return 0;
		for (j = i+1; j < n_digits; j++) 
			if (guess[i] == guess[j])
				return 0;
	}

	return 1;
} 

void new_guess(char *guess_buffer)
{
	int guess_val, mod_val, i, j, index, digit;

	if (best_exact == 0)
		goto again;

	int *set_indices = malloc(sizeof(int) * best_exact);
	int *used_digits = malloc(sizeof(int) * best_exact);
	int *highest_indices = malloc(sizeof(int) * best_exact);

	memset(highest_indices, 0x00, sizeof(int) * best_exact);
	find_highest(highest_indices);	

	/* put in probable parts of solution */
	for (i = 0; i < best_exact; i++) {
		index = highest_indices[i] / 10;
		digit = highest_indices[i] % 10;
	
		write_log("[%d] index=%d, digit=%d", i, index, digit);	

		if (!disqualified[digit]) {
			guess_buffer[index] = digit + '0';
			set_indices[i] = index;
		}
	}

	/* semi-random guesses for the other parts */
	int cont;
again:
	for (i = 0; i < n_digits; i++) {
		cont = 0;
		for (j = 0; j < best_exact; j++)
			if (set_indices[j] == i) {
				cont = 1;
				break;
			}

		if (cont)
			continue;
			
		do {		
			guess_val = rand() % (digit_max + 1);
		} while(disqualified[guess_val]);

		guess_buffer[i] = guess_val + '0';
	}

	if (!valid(guess_buffer))
		goto again;

	if (best_exact > 0) {
		free(set_indices);
		free(used_digits);
		free(highest_indices);
	}
}

/* Generate a random number for the first guess */ 
void random_guess(char* guess_buffer) 
{
	int guess_val, mod_val, i;

	for (i = 0, mod_val = 0; i < n_digits; i++)
		mod_val = (mod_val*10) + 9;

	while (1) {
		guess_val = rand() % mod_val;

		i = 1;
		while (guess_val != 0) {
			guess_buffer[n_digits-i] = ((guess_val % 10) + '0');
			guess_val /= 10;
			i++;
		}
	
		guess_buffer[n_digits] = '\0';

		if (valid(guess_buffer))
			break;
	}
}


int main(int argc, char **argv)
{
	srand(time(NULL));
	set_file("mastermind.log");

	int c_exact, c_misplaced;
	int turn = 1;
	char guess_buffer[10];
	int i;

	scanf("%d %d %d %ld", &digit_max, &n_digits, &n_turns, &max_time);
	
	exact_strings = malloc(sizeof(char *) * n_turns);
	for (i = 0; i < n_turns; i++)
		exact_strings[i] = malloc(n_digits);

	counters = malloc(sizeof(int *) * n_digits * 10);

	best_exact = best_misplaced = 0;

	while (turn <= n_turns) {
		if (turn == 1) 
			random_guess(guess_buffer);
		else
			new_guess(guess_buffer);
		
		printf("Guess: %s\n", guess_buffer);
		scanf("%d %d", &c_exact, &c_misplaced);
		
		if (c_exact == n_digits)
			break;
		
		if ((c_exact == 0) && (c_misplaced == 0))
			for (i = 0; i < n_digits; i++) {
				write_log("main(): setting disqualified[%d] to 1\n", guess_buffer[i] - '0');
				disqualified[guess_buffer[i] - '0'] = 1;
			}

		else if (c_exact + c_misplaced == n_digits) {
			for (i = 0; i <= digit_max; i++)
			       disqualified[i] = 1;	
			for (i = 0; i < n_digits; i++)
				disqualified[guess_buffer[i] - '0'] = 0;
		}

		if (c_exact > 0)
			add_exact_string(guess_buffer);

		if (c_exact >= best_exact || ((c_exact + c_misplaced) >= (best_exact + best_misplaced))) {
			best_exact = c_exact;
			best_misplaced = c_misplaced;
		}

		turn++;
	}

	if (turn <= n_turns)
		printf("Solved in %d turns!\n", turn);
	else
		printf("Failed to solve!\n");
	
	for (i = 0; i < n_turns; i++)
		free(exact_strings[i]);
	
	free(counters);
	free(exact_strings);
	return 0;
}
