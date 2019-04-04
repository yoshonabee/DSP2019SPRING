#include "hmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{	
	if (argc < 3) {
		printf("usage: ./acc [predict result] [ground truth]\n");
		exit(1);
	}

	FILE *p = open_or_die(argv[1], "r");
	FILE *g = open_or_die(argv[2], "r");

	char buff1[20], buff2[20];
	int sample_num = 0, correct = 0;

	while (fscanf(p, "%s", buff1) != EOF && fscanf(g, "%s", buff2) != EOF) {
		sample_num++;
		if (strcmp(buff1, buff2) == 0)
			correct++;
	}

	printf("Accuracy: %lf\n", (double)correct / sample_num);

	return 0;
}