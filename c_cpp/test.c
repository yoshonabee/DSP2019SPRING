#include "hmm.h"
#include "test.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char const *argv[]) {
	if (argc < 4) {
		printf("usage: ./test [modellist path] [testing data path] [result save path]\n");
		exit(0);
	}

	char const *modellist_path = argv[1];
	char const *test_x_path = argv[2];
	char const *result_save_path = argv[3];

	int max_count = 5;
	HMM *hmm = malloc(max_count * sizeof(HMM));
	load_models(modellist_path, hmm, max_count);

	predict(hmm, modellist_path, test_x_path, result_save_path, max_count);

}