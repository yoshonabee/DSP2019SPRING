#include "hmm.h"
#include <stdio.h>
#include <string.h>

#define COF 6

typedef struct {
	double alpha[MAX_SEQ][MAX_STATE];
	double beta[MAX_SEQ][MAX_STATE];
	double gamma[MAX_SEQ][MAX_STATE];
	double epsilon[MAX_STATE][MAX_STATE];
} State;

static void init_state(State *state) {
	for (int l = 0; l < MAX_SEQ; l++)
		for (int s = 0; s < MAX_STATE; s++) {
			state->alpha[l][s] = 0;
			state->beta[l][s] = 0;
			state->gamma[l][s] = 0;
		}

	for (int i = 0; i < MAX_STATE; i++)
		for (int j = 0; j < MAX_STATE; j++)
			state->epsilon[i][j] = 0;
}

static void calc_alpha(const HMM *hmm, State *state, const char *x) {
	for (int t = 0; t < strlen(x); t++) {
		if (t == 0) {
			for (int s = 0; s < hmm->state_num; s++)
				state->alpha[0][s] = hmm->initial[s] * hmm->observation[x[0] - 65][s];
		} else {
			for (int s = 0; s < hmm->state_num; s++) {
				double transition = 0;
				for (int ls = 0; ls < hmm->state_num; ls++)
					transition += state->alpha[t - 1][ls] * hmm->transition[ls][s];
				state->alpha[t][s] = transition * hmm->observation[x[t] - 65][s] * COF;
			}
		}
	}
}

static void calc_beta(const HMM *hmm, State *state, const char *x) {
	for (int s = 0; s < hmm->state_num; s++)
		state->beta[strlen(x) - 1][s] = 1;

	for (int t = strlen(x) - 2; t >= 0; t--) {
		for (int s = 0; s < hmm->state_num; s++) {
			for (int ls = 0; ls < hmm->state_num; ls++)
				state->beta[t][s] += hmm->transition[s][ls] * hmm->observation[x[t + 1] - 65][ls] * state->beta[t + 1][ls] * COF;
		}
	}
}

static void calc_gamma(State *state, const int seq_len, const int state_num) {
	for (int t = 0; t < seq_len; t++) {
		double total = 0;
		for (int s = 0; s < state_num; s++)
			total += state->alpha[t][s] * state->beta[t][s];

		for (int s = 0; s < state_num; s++)
			state->gamma[t][s] += state->alpha[t][s] * state->beta[t][s] / total;
	}
}

static void calc_epsilon(const HMM *hmm, State *state, const char *x) {
	for (int t = 0; t < strlen(x) - 1; t++) {
		double total = 0;

		for (int i = 0; i < hmm->state_num; i++)
			for (int j = 0; j < hmm->state_num; j++)
				total += state->alpha[t][i] * hmm->transition[i][j] * hmm->observation[x[t + 1] - 65][j] * state->beta[t + 1][j];

		for (int i = 0; i < hmm->state_num; i++)
			for (int j = 0; j < hmm->state_num; j++)
				state->epsilon[i][j] += state->alpha[t][i] * hmm->transition[i][j] * hmm->observation[x[t + 1] - 65][j] * state->beta[t + 1][j] / total;
	}
}

static void summation_gamma(double *total_gamma, const State *state) {
	for (int s = 0; s < MAX_STATE; s++) {
		double total = 0;
		for (int t = 0; t < MAX_SEQ; t++) {
			total += state->gamma[t][s];

		}
		total_gamma[s] = total;
	}
}

static void calc_new_initial(double *initial, const State *state, const int state_num) {
	for (int i = 0; i < state_num; i++)
		initial[i] += state->gamma[0][i];
}

static void calc_new_transition(double **transition, const State *state, const int seq_len, const int state_num) {
	for (int i = 0; i < state_num; i++)
		for (int j = 0; j < state_num; j++) {
			double total_gamma = 0;
			for (int t = 0; t < seq_len; t++)
				total_gamma += state->gamma[t][i];
			transition[i][j] += state->epsilon[i][j] / total_gamma;
		}
}

static void calc_new_observation(double **observation, const State *state, const int seq_len, const int state_num, const int observ_num, const char *x) {
	for (int s = 0; s < state_num; s++) {
		double total_gamma = 0;
		for (int t = 0; t < seq_len; t++)
			total_gamma += state->gamma[t][s];

		for (int o = 0; o < observ_num; o++) {
			double numerator = 0;
			for (int t = 0; t < seq_len; t++)
				if (x[t] - 65 == o)
					numerator += state->gamma[t][s];
			observation[o][s] += numerator / total_gamma;
		}
	}
}

static void update_hmm(HMM *hmm, double *initial, double **transition, double **observation, int sample_num) {
	for (int i = 0; i < hmm->state_num; i++)
		hmm->initial[i] = initial[i] / sample_num;

	for (int i = 0; i < hmm->state_num; i++)
		for (int j = 0; j < hmm->state_num; j++)
			hmm->transition[i][j] = transition[i][j] / sample_num;

	for (int i = 0; i < hmm->observ_num; i++)
		for (int j = 0; j < hmm->state_num; j++)
			hmm->observation[i][j] = observation[i][j] / sample_num;
}

static void train_iteration(HMM *hmm, const char *train_x_path) {
	State state;
	FILE *fp = open_or_die(train_x_path, "r");
	
	int sample_num = 0;
	char x[MAX_SEQ];

	double *total_gamma = malloc(MAX_STATE * sizeof(double));
	double *new_initial = malloc(MAX_STATE * sizeof(double));

	double **new_transition = malloc(MAX_SEQ * sizeof(double *));
	for (int i = 0; i < MAX_SEQ; i++)
		new_transition[i] = malloc(MAX_STATE * sizeof(double));

	double **new_observation = malloc(MAX_OBSERV * sizeof(double *));
	for (int i = 0; i < MAX_OBSERV; i++)
		new_observation[i] = malloc(MAX_STATE * sizeof(double));

	init_state(&state);

	while (fscanf(fp, "%s", x) != EOF) {
		sample_num++;
		calc_alpha(hmm, &state, x);
		calc_beta(hmm, &state, x);
		calc_gamma(&state, strlen(x), hmm->state_num);
		calc_epsilon(hmm, &state, x);

		summation_gamma(total_gamma, &state);
		calc_new_initial(new_initial, &state, hmm->state_num);
		calc_new_transition(new_transition, &state, strlen(x), hmm->state_num);
		calc_new_observation(new_observation, &state, strlen(x), hmm->state_num, hmm->observ_num, x);
		init_state(&state);
	}

	update_hmm(hmm, new_initial, new_transition, new_observation, sample_num);
	// fclose(fp);
}