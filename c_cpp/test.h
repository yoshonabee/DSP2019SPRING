#include "hmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COF 6

typedef struct {
	double delta[MAX_SEQ][MAX_STATE];
	int from[MAX_SEQ][MAX_STATE];
} State;

static void init_state(State *state) {
	for (int t = 0; t < MAX_SEQ; t++)
		for (int s = 0; s < MAX_STATE; s++) {
			state->delta[t][s] = 0;
			state->from[t][s] = 0;
		}
}

static void calc_delta(const HMM *hmm, State *state, const char *x) {
	for (int t = 0; t < strlen(x); t++) {
		if (t == 0) {
			for (int s = 0; s < hmm->state_num; s++) {
				state->delta[t][s] = hmm->initial[s] * hmm->observation[x[0] - 65][s];
				state->from[t][s] = -1;
			}
		} else {
			for (int s = 0; s < hmm->state_num; s++) {
				double max_p = 0, from;
				for (int ls = 0; ls < hmm->state_num; ls++) {
					double p = state->delta[t - 1][ls] * hmm->transition[ls][s];
					if (p > max_p) {
						max_p = p;
						from = ls;
					}
				}
				state->delta[t][s] = max_p * hmm->observation[x[t] - 65][s] * COF;
				state->from[t][s] = from;
			}
		}
	}
}

static void predict(const HMM *hmm, const char *modellist_path, const char *test_x_path, const char *result_save_path, const int max_count) {
	char **modellist = malloc(max_count * sizeof(char *));

	FILE *mf = open_or_die(modellist_path, "r");
	for (int i = 0; i < max_count; i++) {
		modellist[i] = malloc(20 * sizeof(char));
		fscanf(mf, "%s", modellist[i]);
	}
	fclose(mf);

	State state;
	FILE *fp = open_or_die(test_x_path, "r");
	FILE *result = open_or_die(result_save_path, "w");
	
	char x[MAX_SEQ];

	while (fscanf(fp, "%s", x) != EOF) {
		int best = -1;
		double score = 0;
		for (int i = 0; i < max_count; i++) {
			init_state(&state);
			calc_delta(&hmm[i], &state, x);
			for (int s = 0; s < hmm[i].state_num; s++)
				if (state.delta[strlen(x) - 1][s] > score) {
					score = state.delta[strlen(x) - 1][s];
					best = i;
				}
		}
		fprintf(result, "%s\n", modellist[best]);
	}
	fclose(fp);
	fclose(result);
}