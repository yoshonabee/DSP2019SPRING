#include <iostream>
#include <fstream>
#include "Ngram.h"
#include "Vocab.h"
using namespace std;

char G_zy_table[37][3] = {0};
char G_zy_ch_map[37][5000][3] = {0};
int G_ch_num[100] = {0};
int ch_num[37] = {0};
VocabIndex G_viterbi_map[5000][100] = {0};
double G_viterbi_prob[5000][100] = {0};
int G_viterbi_path[5000][100] = {0};
char G_viterbi_out_ch[100][3] = {0};

double get_prob(const char *word1, char *wor2, Vocab &voc, Ngram &lm)
{
	VocabIndex vi1 = (voc.getIndex(word1) == Vocab_None)
		? ( voc.getIndex(Vocab_Unknown) )
		: ( voc.getIndex(word1) );
	VocabIndex vi2 = (voc.getIndex(wor2) == Vocab_None)
		? ( voc.getIndex(Vocab_Unknown) )
		: ( voc.getIndex(wor2) );
	VocabIndex vi3[] = {vi1, Vocab_None};
	return lm.wordProb(vi2, vi3);
}

int is_zy(char *pch)
{
	int zy_idx = -1;
	for(int i = 0 ; i < 37; i++)
	{
		if(strncmp(pch, G_zy_table[i], 2) == 0)
		{
			zy_idx = i;
			break;
		}
	}
	return zy_idx;
}

void get_G_ch_num(int zy_idx, int cnt)
{
	G_ch_num[cnt] = (zy_idx == -1) ? 1 : ( ch_num[zy_idx] );
}

void get_G_viterbi_map(int zy_idx, int cnt, char *pch, Vocab &voc)
{	
	char w[3];
	if(zy_idx == -1)
	{
		G_viterbi_map[0][cnt] = voc.getIndex(pch);
	}
	else
	{
		for(int i = 0; i < ch_num[zy_idx]; i++)
		{	
			
			strncpy(w, G_zy_ch_map[zy_idx][i], 2);
			w[2] = '\0';
			G_viterbi_map[i][cnt] = voc.getIndex(w);
		}
	}
}

void get_G_viterbi_prob_and_G_viterbi_path(int zy_idx, int cnt, char *pch, char *word, Vocab &voc, Ngram &lm)
{
	double prob;
	if(zy_idx >= 0)
	{
		for(int i = 0; i < ch_num[zy_idx]; i++)
		{
			strncpy(word, G_zy_ch_map[zy_idx][i], 2); // word: 該注音的第i個國字 

			// 若在測資該行的第一個字(cnt == 0):
			if(cnt == 0)
			{
				G_viterbi_prob[i][cnt] = get_prob("<unk>", word, voc, lm);
				G_viterbi_path[i][cnt] = -1;
			}

			// 若不是在測資該行的第一個字(cnt != 0):
			//     G_viterbi_prob[i][cnt] = max_j(  get_prob(voc.getWord(G_viterbi_map[j][cnt-1]), word, voc, lm) + G_viterbi_prob[j][cnt-1]  )
			//     G_viterbi_path[i][cnt] = argmax_j(  get_prob(voc.getWord(G_viterbi_map[j][cnt-1]), word, voc, lm) + G_viterbi_prob[j][cnt-1]  )
			else
			{
				double max_prob = -500000;
				int max_prob_idx = -1;
				for(int j = 0; j < G_ch_num[cnt-1]; j++)
				{
					prob = (G_viterbi_map[j][cnt-1] == -1) 
						? ( get_prob("<unk>", word, voc, lm) - 5000 )
						: ( get_prob(voc.getWord(G_viterbi_map[j][cnt-1]), word, voc, lm) + G_viterbi_prob[j][cnt-1] );
					if(prob > max_prob)
					{
						max_prob = prob;
						max_prob_idx = j;
					}
				}
				G_viterbi_prob[i][cnt] = max_prob;
				G_viterbi_path[i][cnt] = max_prob_idx;
			}
		}
	}
	else
	{
		if(cnt == 0)
		{
			G_viterbi_prob[0][cnt] = get_prob("<unk>", pch, voc, lm);
			G_viterbi_path[0][cnt] = -1;
		}
		else
		{
			double max_prob = -500000;
			int max_prob_idx = -1;
			for(int j = 0 ; j < G_ch_num[cnt-1]; j++)
			{
				if (G_viterbi_map[j][cnt-1] == -1) {
					prob = ( get_prob("<unk>", pch, voc, lm) - 5000 );
				} else {
					prob = ( get_prob(voc.getWord(G_viterbi_map[j][cnt-1]), pch, voc, lm) + G_viterbi_prob[j][cnt-1] );
				}

				// int a = G_viterbi_map[j][cnt-1] == -1;
				// cout << a << endl;

				if(prob > max_prob)
				{
					max_prob = prob;
					max_prob_idx = j;
				}
			}
			G_viterbi_prob[0][cnt] = max_prob;
			G_viterbi_path[0][cnt] = max_prob_idx;
		}
	}
}

void get_G_viterbi_out_ch(int zy_idx, int cnt, char *word)
{
	strncpy(G_viterbi_out_ch[cnt], word, 2);
}

void print_out(int cnt, char *text, Vocab &voc)
{
	if(cnt == 0)
	{
		cout << "<s>" << text << "</s>" << endl;
	}
	else
	{
		double out_max = -50000;
		int out_max_idx = -1;
		VocabIndex output[100];

		for(int i = 0 ; i < G_ch_num[cnt-1]; i++)
		{
			if(G_viterbi_prob[i][cnt-1] > out_max)
			{
				out_max = G_viterbi_prob[i][cnt-1];
				out_max_idx = i;
			}
		}

		for(int i = 0; i < cnt; i++)
		{
			output[cnt -1 -i] = G_viterbi_map[out_max_idx][cnt -1 -i];
			out_max_idx = G_viterbi_path[out_max_idx][cnt -1 -i];
		}

		cout << "<s> ";
		for(int i = 0; i < cnt; i++)
		{	
			// cout << output[i] << endl;
			cout << ( (output[i] != -1) ? ( voc.getWord(output[i]) ) : ( G_viterbi_out_ch[i] ) ) << " ";
		}
		cout<<"</s>"<<endl;
	}
}

int main( int argc, char * argv[] ){

	fstream test_data, map_data, zy_table_data;
	test_data.open( argv[1] ,ios::in );
	map_data.open( argv[2] ,ios::in );
	zy_table_data.open( "zy_table.txt" ,ios::in );

	Vocab voc;
 	Ngram lm(voc, 2);
	{
		File lmFile(argv[3], "r");
		lm.read(lmFile);
		lmFile.close();
	}

	char text[1000];
	char word[3];
	char *pch;
	int cnt;
	int zy_idx;

	for(int i = 0 ; i < 37; i++)
	{
		zy_table_data >> G_zy_table[i];
	}

	while(map_data >> word)
	{
		int i;
		zy_idx = -1;
		for(i = 0 ; i < 37; i++)
		{
			if(strncmp(word, G_zy_table[i], 2) == 0)
			{
				zy_idx = i; 
				break;
			}
		}
		if (zy_idx == -1)
			map_data.getline(text, 999);
		else {
			char buff[5000 * 3];
			map_data.getline(buff, 5000*3-1);

			int cnt = 0;
			pch = strtok(buff, " ");
			while (pch != NULL) {
				if (strcmp(pch, "\t") != 0) {
					strncpy(G_zy_ch_map[i][cnt], pch, 2);
				}

				cnt++;
				pch = strtok(NULL, " ");
			}
			ch_num[i] = cnt;
		}
	}

	while(test_data.getline(text, 999))
	{
		cnt = 0;

		pch = strtok(text, " ");
		while(pch != NULL)
		{
			zy_idx = is_zy(pch);

			get_G_ch_num(zy_idx, cnt);

			get_G_viterbi_map(zy_idx, cnt, pch, voc);

			get_G_viterbi_prob_and_G_viterbi_path(zy_idx, cnt, pch, word, voc, lm);

			get_G_viterbi_out_ch(zy_idx, cnt, word);

			cnt++;

			pch = strtok(NULL, " ");
		}

		print_out(cnt, text, voc);
	}



	test_data.close();
	map_data.close();
	zy_table_data.close();

	return 0;
}
