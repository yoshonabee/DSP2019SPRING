#include "hmm.h"
#include "train.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// argv[1]: iteration num
// argv[2]: initial model path
// argv[3]: traning_data path
// argv[4]: output model path

int main(int argc, char const *argv[]) {
	if (argc < 5) {
		printf("usage: ./train [iteration] [init model path] [training data path] [model save path]\n");
		exit(0);
	}

	int iter = atoi(argv[1]);
	char const *model_init_path = argv[2];
	char const *train_x_path = argv[3];
	char const *model_save_path = argv[4];
	
	HMM hmm;

	init_model(model_init_path, &hmm);
	print_model(&hmm);

	for (int i = 0; i < iter; i++) {
		train_iteration(&hmm, train_x_path);
		printf("\n%d\n", i + 1);
		print_model(&hmm);
	}

	FILE *fp = open_or_die(model_save_path, "w");
	dumpHMM(fp, &hmm);
	fclose(fp);

	return 0;
}