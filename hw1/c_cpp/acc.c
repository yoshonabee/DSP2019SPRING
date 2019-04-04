#include "hmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	FILE *p = open_or_die("result/result1.txt", "r");
	FILE *g = open_or_die("data/test/testing_answer.txt", "r");

	char buff1[20], buff2[20];
	int sample_num = 0, correct = 0;

	while (fscanf(p, "%s", buff1) != EOF && fscanf(g, "%s", buff2) != EOF) {
		sample_num++;
		if (strcmp(buff1, buff2) == 0)
			correct++;
		fscanf(p, "%s", buff1);
	}

	FILE *acc = open_or_die("result/acc.txt", "w");
	printf("%lf\n", (double)correct / sample_num);
	fprintf(acc, "%lf\n", (double)correct / sample_num);
	fclose(p);
	fclose(g);
	fclose(acc);

	return 0;
}