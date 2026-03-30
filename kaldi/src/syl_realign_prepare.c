#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define INI_PHRASE_LEN 32
#define INI_WORD_CAP 32
#define INI_WORD_LEN 32
#define INI_SYLLABLE_CODE_SIZE 1024
#define DOUBLE_MIN -1.79769e+308

typedef struct _KALDI_LEXICONP_TOKEN_
{
	char *word;
	float prob;
	char *phseq;
} KALDI_LEXICONP_TOKEN;
typedef struct _KALDI_LEXICONP_
{
	int size;
	KALDI_LEXICONP_TOKEN *klt;
} KALDI_LEXICONP;
typedef struct _KALDI_LEXICONP_LIST_
{
	int size;
	KALDI_LEXICONP *kls;
} KALDI_LEXICONP_LIST;
/***************************************/
typedef struct _SINGLEWORD_			 /**/
{									 /**/
	char *word;						 /**/
	double prob_outcome;			 /**/
	int outcome_times;				 /**/
	char *syl_code_seq;				 /**/
} SINGLEWORD;						 /**/
typedef struct _LEXICON_			 /**/
{									 /**/
	SINGLEWORD *sw;					 /**/
	int num_word; /*for word match*/ /**/
	int capacity;					 /**/
} LEXICON;							 /**/
typedef struct _LENDICT_			 /**/
{									 /**/
	int maximum_len;				 /**/
	LEXICON *lexs;					 /**/
	int total_lexicon_outcome;		 /**/
} LENDICT;							 /**/
typedef struct _WMOSYM_				 /**/
{									 /**/
	char *word;						 /**/
	double prob;					 /**/
	int idx, num_next;				 /**/
	struct _WMOSYM_ *prev;			 /**/
} WMOSYM;							 /**/
typedef struct _WMOSYM_LIST_		 /**/
{									 /**/
	int capacity, size;				 /**/
	WMOSYM **list;					 /**/
} WMOSYM_LIST;						 /**/
typedef struct _WORD_LEN_RECORD_	 /**/
{									 /**/
	int size;						 /**/
	int *max_len, *cur_len;			 /**/
	int *link_num;					 /**/
	double *acc_prob;				 /**/
} WORD_LEN_RECORD;					 /**/
/***************************************/
typedef struct _TEXT_
{
	int has_character_flag;
	int num_of_sentence;
	char **sentence;
} TEXT;
typedef struct _TEXT_LIST_
{
	TEXT *txtlst;
	char **tokenized_text;
	char **text_full_name;
	char **text_name;
	int num_of_text;
} TEXT_LIST;
typedef struct _WAV_LIST_
{
	char **wav_full_name;
	char **wav_name;
	int num_of_wav;
} WAV_LIST;
typedef struct _TEXT_AND_WAV_
{
	TEXT_LIST tl;
	WAV_LIST wl;
} TEXT_AND_WAV;
typedef struct _UTT2SPK_pair_
{
	char *utt, *spk;
} UTT2SPK_pair;
typedef struct _UTT2SPK_
{
	int spk_num, utt_num;
	UTT2SPK_pair *usp;
} UTT2SPK;
typedef struct _DATA_SET_
{
	TEXT_AND_WAV all, useful, unpaired;
	UTT2SPK train, dev, test;
	char **diceing_dir;
	int speaker_num;
	char **speaker_list;
} DATA_SET;
typedef struct _STRING_SET_
{
	int size;
	char **as;
} STRING_SET;
typedef struct _STRING_SET_LIST_
{
	int maxinum_len;
	STRING_SET *ss;
} STRING_SET_LIST;
typedef struct _SYLLABLE_CODE_
{
	int size, capacity;
	char **word;
	int *code;
	int *isword;
	int *isexist;
	int region_num;
	int *start_fill_idx;
	int **wmregion;
	int num_lex;
	char **lex;
} SYLLABLE_CODE;
typedef struct _PHONEME_SET_
{
	int size, capacity;
	char **phonemes;
} PHONEME_SET;

char English[104][4] = {	{ 'A', '\0' },/**0  Ａ**/	{ 'B', '\0' },/**1  Ｂ**/	{ 'C', '\0' },/**2  Ｃ**/	{ 'D', '\0' },/**3  Ｄ**/	{ 'E', '\0' },/**4  Ｅ**/	{ 'F', '\0' },/**5  Ｆ**/	{ 'G', '\0' },/**6  Ｇ**/	{ 'H', '\0' },/**7  Ｈ**/	{ 'I', '\0' },/**8  Ｉ**/	{ 'J', '\0' },/**9  Ｊ**/	{ 'K', '\0' },/**10  Ｋ**/	{ 'L', '\0' },/**11  Ｌ**/	{ 'M', '\0' },/**12  Ｍ**/	{ 'N', '\0' },/**13  Ｎ**/	{ 'O', '\0' },/**14  Ｏ**/	{ 'P', '\0' },/**15  Ｐ**/	{ 'Q', '\0' },/**16  Ｑ**/	{ 'R', '\0' },/**17  Ｒ**/	{ 'S', '\0' },/**18  Ｓ**/	{ 'T', '\0' },/**19  Ｔ**/	{ 'U', '\0' },/**20  Ｕ**/	{ 'V', '\0' },/**21  Ｖ**/	{ 'W', '\0' },/**22  Ｗ**/	{ 'X', '\0' },/**23  Ｘ**/	{ 'Y', '\0' },/**24  Ｙ**/	{ 'Z', '\0' },/**25  Ｚ**/	{ 'a', '\0' },/**26  ａ**/	{ 'b', '\0' },/**27  ｂ**/	{ 'c', '\0' },/**28  ｃ**/	{ 'd', '\0' },/**29  ｄ**/	{ 'e', '\0' },/**30  ｅ**/	{ 'f', '\0' },/**31  ｆ**/	{ 'g', '\0' },/**32  ｇ**/	{ 'h', '\0' },/**33  ｈ**/	{ 'i', '\0' },/**34  ｉ**/	{ 'j', '\0' },/**35  ｊ**/	{ 'k', '\0' },/**36  ｋ**/	{ 'l', '\0' },/**37  ｌ**/	{ 'm', '\0' },/**38  ｍ**/	{ 'n', '\0' },/**39  ｎ**/	{ 'o', '\0' },/**40  ｏ**/	{ 'p', '\0' },/**41  ｐ**/	{ 'q', '\0' },/**42  ｑ**/	{ 'r', '\0' },/**43  ｒ**/	{ 's', '\0' },/**44  ｓ**/	{ 't', '\0' },/**45  ｔ**/	{ 'u', '\0' },/**46  ｕ**/	{ 'v', '\0' },/**47  ｖ**/	{ 'w', '\0' },/**48  ｗ**/	{ 'x', '\0' },/**49  ｘ**/	{ 'y', '\0' },/**50  ｙ**/	{ 'z', '\0' },/**51  ｚ**/	{ -17, -68, -95, '\0' },/**52  Ａ**/	{ -17, -68, -94, '\0' },/**53  Ｂ**/	{ -17, -68, -93, '\0' },/**54  Ｃ**/	{ -17, -68, -92, '\0' },/**55  Ｄ**/	{ -17, -68, -91, '\0' },/**56  Ｅ**/	{ -17, -68, -90, '\0' },/**57  Ｆ**/	{ -17, -68, -89, '\0' },/**58  Ｇ**/	{ -17, -68, -88, '\0' },/**59  Ｈ**/	{ -17, -68, -87, '\0' },/**60  Ｉ**/	{ -17, -68, -86, '\0' },/**61  Ｊ**/	{ -17, -68, -85, '\0' },/**62  Ｋ**/	{ -17, -68, -84, '\0' },/**63  Ｌ**/	{ -17, -68, -83, '\0' },/**64  Ｍ**/	{ -17, -68, -82, '\0' },/**65  Ｎ**/	{ -17, -68, -81, '\0' },/**66  Ｏ**/	{ -17, -68, -80, '\0' },/**67  Ｐ**/	{ -17, -68, -79, '\0' },/**68  Ｑ**/	{ -17, -68, -78, '\0' },/**69  Ｒ**/	{ -17, -68, -77, '\0' },/**70  Ｓ**/	{ -17, -68, -76, '\0' },/**71  Ｔ**/	{ -17, -68, -75, '\0' },/**72  Ｕ**/	{ -17, -68, -74, '\0' },/**73  Ｖ**/	{ -17, -68, -73, '\0' },/**74  Ｗ**/	{ -17, -68, -72, '\0' },/**75  Ｘ**/	{ -17, -68, -71, '\0' },/**76  Ｙ**/	{ -17, -68, -70, '\0' },/**77  Ｚ**/	{ -17, -67, -127, '\0' },/**78  ａ**/	{ -17, -67, -126, '\0' },/**79  ｂ**/	{ -17, -67, -125, '\0' },/**80  ｃ**/	{ -17, -67, -124, '\0' },/**81  ｄ**/	{ -17, -67, -123, '\0' },/**82  ｅ**/	{ -17, -67, -122, '\0' },/**83  ｆ**/	{ -17, -67, -121, '\0' },/**84  ｇ**/	{ -17, -67, -120, '\0' },/**85  ｈ**/	{ -17, -67, -119, '\0' },/**86  ｉ**/	{ -17, -67, -118, '\0' },/**87  ｊ**/	{ -17, -67, -117, '\0' },/**88  ｋ**/	{ -17, -67, -116, '\0' },/**89  ｌ**/	{ -17, -67, -115, '\0' },/**90  ｍ**/	{ -17, -67, -114, '\0' },/**91  ｎ**/	{ -17, -67, -113, '\0' },/**92  ｏ**/	{ -17, -67, -112, '\0' },/**93  ｐ**/	{ -17, -67, -111, '\0' },/**94  ｑ**/	{ -17, -67, -110, '\0' },/**95  ｒ**/	{ -17, -67, -109, '\0' },/**96  ｓ**/	{ -17, -67, -108, '\0' },/**97  ｔ**/	{ -17, -67, -107, '\0' },/**98  ｕ**/	{ -17, -67, -106, '\0' },/**99  ｖ**/	{ -17, -67, -105, '\0' },/**100  ｗ**/	{ -17, -67, -104, '\0' },/**101  ｘ**/	{ -17, -67, -103, '\0' },/**102  ｙ**/	{ -17, -67, -102, '\0' },/**103  ｚ**/};
char space_and_tab[3][4] = {	{ -29, -128, -128, '\0'},/**473  　**/	{ ' ', '\0' },	{ '\t', '\0' }};
char Number_char[20][4]={
{48, '\0'},/** 0**/
{49, '\0'},/** 1**/
{50, '\0'},/** 2**/
{51, '\0'},/** 3**/
{52, '\0'},/** 4**/
{53, '\0'},/** 5**/
{54, '\0'},/** 6**/
{55, '\0'},/** 7**/
{56, '\0'},/** 8**/
{57, '\0'},/** 9**/
{-17, -68, -112, '\0'},/** ０**/
{-17, -68, -111, '\0'},/** １**/
{-17, -68, -110, '\0'},/** ２**/
{-17, -68, -109, '\0'},/** ３**/
{-17, -68, -108, '\0'},/** ４**/
{-17, -68, -107, '\0'},/** ５**/
{-17, -68, -106, '\0'},/** ６**/
{-17, -68, -105, '\0'},/** ７**/
{-17, -68, -104, '\0'},/** ８**/
{-17, -68, -103, '\0'},/** ９**/
};
char Dot[3][4] = {{-17, -68, -114, '\0'}, /** ．**/{-30, -128, -89, '\0'}, /** ‧**/{'.', '\0'}};
char apostrophe[2][4] = {{-30,-128,-103,'\0'},{"\'"}};
int isDot(char *word);
int stringcomparefun(const void *a, const void *b);
int string_len_comparefun(const void *a, const void *b);
void load_text(FILE *fin, TEXT *text);
void tokenizer(TEXT *text, char **tokenized_text);
int isCJK_words(char *word);
int isspace_and_tab(char *word);
int isapostrophe(char *word);
unsigned long long int utf8word2decimal(char *word);
void normalize_English(char *word);
int isnumber(char *word);
int read_a_utf8_word(char *line, char *word, int *index);
int Retreat_a_utf8_word(char *line, int *index);
int compare( const void *a, const void *b);
int read_a_line(char **line, int *capacity_of_line, FILE *fin);
void output_a_line(FILE *fout, char *line);
void load_text_list(FILE *flst, TEXT_LIST *tl, KALDI_LEXICONP_LIST *kll);
void run(FILE *ftls, FILE *fwls, FILE *fdic, char *output_dir);
void gen_dir(char *output_dir);
void reset_dir_data(char *dir);
void load_data(FILE *ftls, FILE *fwls, DATA_SET *ds, KALDI_LEXICONP_LIST *kll);
void load_wav_list(FILE *fwls, WAV_LIST *wl);
void take_useful_pair(TEXT_AND_WAV *all, TEXT_AND_WAV *useful, TEXT_AND_WAV *unpaired);
int bin_string_search(char *sentence, char **set, int set_size);
void gen_string_set_list(char **raw_set,int raw_size, STRING_SET_LIST *ssl);
void get_speaker_list(DATA_SET *ds);
void gen_utt2spk(UTT2SPK *train, UTT2SPK *dev, UTT2SPK *test, char **diceing_dir, TEXT_AND_WAV *useful);
void output_data(DATA_SET *ds, char *output_dir);
void output_utt2spk(UTT2SPK *u2s, char *output_path);
int utf8_word_length(char *word);
void normalize_word(char *raw, char **modify);
int isEnglish(char *word);
void copy_useful_text(TEXT_LIST *tl, TEXT_LIST *ttl);
void load_kaldi_lexp(FILE *fdic, KALDI_LEXICONP *kl, KALDI_LEXICONP_LIST *kll);
int bin_kaldilexiconp_search(char *word, KALDI_LEXICONP *kl);
int kaldilexiconcomparefun(const void *a, const void *b);
void normalize_continous_space_tab(char *raw, char *mod);
int check_tokon_is_exist_in_dict(char *token, KALDI_LEXICONP_LIST *kll);
int main(int argc, char **argv)
{
	FILE *ftls, *fwls, *fdic;
	if (argc != 5)
	{
		printf("Usage: realign_prepare text_list wav_list lexiconp output_dir\n");
		exit(1);
	}
	if (!(ftls = fopen(argv[1], "r")))
	{
		printf("can not open file: %s\n", argv[1]);
		exit(1);
	}
	if (!(fwls = fopen(argv[2], "r")))
	{
		printf("can not open file: %s\n", argv[2]);
		exit(1);
	}
	if (!(fdic = fopen(argv[3], "r")))
	{
		printf("can not open file: %s\n", argv[3]);
		exit(1);
	}
	run(ftls, fwls, fdic, argv[4]);

	fclose(ftls);
	fclose(fwls);
	fclose(fdic);

	return 0;
}
void run(FILE *ftls, FILE *fwls, FILE *fdic, char *output_dir)
{
	DATA_SET ds;
	KALDI_LEXICONP kl;
	KALDI_LEXICONP_LIST kll;
	char *checked_output_dir = (char *)malloc((strlen(output_dir) + 2) * sizeof(char));
	int i;

	strcpy(checked_output_dir, output_dir);
	if (checked_output_dir[strlen(checked_output_dir) - 1] != '/')
	{
		checked_output_dir[strlen(checked_output_dir) + 1] = '\0';
		checked_output_dir[strlen(checked_output_dir)] = '/';
	}

	gen_dir(checked_output_dir);
	load_kaldi_lexp(fdic, &kl, &kll);
	load_data(ftls, fwls, &ds, &kll);
	printf("total_wav_size=%d, total_text_size=%d\n", ds.all.wl.num_of_wav, ds.all.tl.num_of_text);
	printf("useful_wav_size=%d, useful_text_size=%d\n", ds.useful.wl.num_of_wav, ds.useful.tl.num_of_text);
	printf("unpaird_text_number: %d\n", ds.unpaired.tl.num_of_text);
	for (i = 0; i < ds.unpaired.tl.num_of_text; i++)
	{
		printf("%d\t%s\n",i+1,ds.unpaired.tl.text_full_name[i]);
	}
	printf("unpaird_wave_number: %d\n", ds.unpaired.wl.num_of_wav);
	for (i = 0; i < ds.unpaired.wl.num_of_wav; i++)
	{
		printf("%d\t%s\n", i + 1, ds.unpaired.wl.wav_full_name[i]);
	}
	output_data(&ds, checked_output_dir);

	free(checked_output_dir);
}
void output_data(DATA_SET *ds, char *output_dir)
{
	FILE *fout;
	int i, j, temp;
	char *file_name = (char *)malloc((strlen(output_dir) + 32) * sizeof(char));

	strcpy(file_name, output_dir);
	strcat(file_name, "wav.scp");
	if (!(fout = fopen(file_name, "w")))
	{
		printf("can not open file: %s\n", file_name);
		exit(1);
	}
	for (i = 0; i < ds->useful.wl.num_of_wav; i++)
	{
		fprintf(fout, "%s %s\n", ds->useful.wl.wav_name[i], ds->useful.wl.wav_full_name[i]);
	}
	fclose(fout);

	strcpy(file_name, output_dir);
	strcat(file_name, "text");
	if (!(fout = fopen(file_name, "w")))
	{
		printf("can not open file: %s\n", file_name);
		exit(1);
	}
	for (i = 0; i < ds->useful.tl.num_of_text; i++)
	{
		fprintf(fout, "%s %s\n", ds->useful.tl.text_name[i], ds->useful.tl.tokenized_text[i]);
	}
	fclose(fout);

	strcpy(file_name, output_dir);
	strcat(file_name, "utt2spk");
	if (!(fout = fopen(file_name, "w")))
	{
		printf("can not open file: %s\n", file_name);
		exit(1);
	}
	for (i = 0; i < ds->useful.wl.num_of_wav; i++)
	{
		fprintf(fout, "%s ", ds->useful.wl.wav_name[i]);
		for (j = 0, temp = strlen(ds->useful.wl.wav_name[i]); j < temp; j++)
		{
			if (ds->useful.wl.wav_name[i][j] == '-')
			{
				fprintf(fout, "\n");
				break;
			}
			fprintf(fout, "%c", ds->useful.wl.wav_name[i][j]);
		}
		if (j == temp)
		{
			fprintf(fout, "\n");
		}
	}
	fclose(fout);

	free(file_name);
}
void output_utt2spk(UTT2SPK *u2s, char *output_path)
{
	FILE *fout;
	int i;
	if (!(fout = fopen(output_path, "w")))
	{
		printf("can not open file: %s\n", output_path);
		exit(1);
	}
	else
	{
		for (i = 0; i < u2s->utt_num; i++)
		{
			fprintf(fout, "%s %s\n", u2s->usp[i].utt, u2s->usp[i].spk);
		}
		fclose(fout);
	}
}
void get_speaker_list(DATA_SET *ds)
{
	int i, j, k;

	ds->speaker_num = 0;
	ds->speaker_list = (char **)malloc(ds->useful.tl.num_of_text * sizeof(char *));

	for (i = 0; i < ds->useful.tl.num_of_text; i++)
	{
		for (j = 0; ds->useful.tl.text_name[i][j] != '-' && ds->useful.tl.text_name[i][j] != '\0'; j++)
		{
			continue;
		}
		for (k = 0; k < ds->speaker_num; k++)
		{
			if (strncmp(ds->speaker_list[k], ds->useful.tl.text_name[i], j) == 0 && strlen(ds->speaker_list[k]) == j)
			{
				break;
			}
		}
		if (k == ds->speaker_num)
		{
			ds->speaker_list[ds->speaker_num] = (char *)malloc((j + 1) * sizeof(char));
			strncpy(ds->speaker_list[ds->speaker_num], ds->useful.tl.text_name[i], j);
			ds->speaker_list[ds->speaker_num][j]='\0';
			ds->speaker_num++;
		}
	}
}
void gen_utt2spk(UTT2SPK *train, UTT2SPK *dev, UTT2SPK *test, char **diceing_dir, TEXT_AND_WAV *useful)
{
	int i, j, len, idx1, idx2, idx3;

	train->spk_num = dev->spk_num = test->spk_num = 0;
	if (train->utt_num > 0)
	{
		train->usp = (UTT2SPK_pair *)malloc(train->utt_num * sizeof(UTT2SPK_pair));
	}
	if (dev->utt_num > 0)
	{
		dev->usp = (UTT2SPK_pair *)malloc(dev->utt_num * sizeof(UTT2SPK_pair));
	}
	if (test->utt_num > 0)
	{
		test->usp = (UTT2SPK_pair *)malloc(test->utt_num * sizeof(UTT2SPK_pair));
	}

	for (i = idx1 = idx2 = idx3 = 0; i < useful->wl.num_of_wav; i++)
	{
		len = strlen(useful->wl.wav_name[i]);
		if (strcmp(diceing_dir[i], "data/train/") == 0)
		{
			train->usp[idx1].utt = (char *)malloc((len + 1) * sizeof(char));
			strcpy(train->usp[idx1].utt, useful->wl.wav_name[i]);
			train->usp[idx1].spk = (char *)malloc((len + 1) * sizeof(char));
			strcpy(train->usp[idx1].spk, useful->wl.wav_name[i]);
			for (j = 0; j < len; j++)
			{
				if (train->usp[idx1].spk[j] == '-')
				{
					break;
				}
			}
			if (j < len)
			{
				train->usp[idx1].spk[j] = '\0';
			}
			if (train->spk_num == 0)
			{
				train->spk_num++;
			}
			else
			{
				if (strcmp(train->usp[idx1].spk, train->usp[idx1 - 1].spk) != 0)
				{
					train->spk_num++;
				}
			}
			idx1++;
		}
		else if (strcmp(diceing_dir[i], "data/dev/") == 0)
		{
			dev->usp[idx2].utt = (char *)malloc((len + 1) * sizeof(char));
			strcpy(dev->usp[idx2].utt, useful->wl.wav_name[i]);
			dev->usp[idx2].spk = (char *)malloc((len + 1) * sizeof(char));
			strcpy(dev->usp[idx2].spk, useful->wl.wav_name[i]);
			for (j = 0; j < len; j++)
			{
				if (dev->usp[idx2].spk[j] == '-')
				{
					break;
				}
			}
			if (j < len)
			{
				dev->usp[idx2].spk[j] = '\0';
			}
			if (dev->spk_num == 0)
			{
				dev->spk_num++;
			}
			else
			{
				if (strcmp(dev->usp[idx2].spk, dev->usp[idx2 - 1].spk) != 0)
				{
					dev->spk_num++;
				}
			}
			idx2++;
		}
		else
		{
			test->usp[idx3].utt = (char *)malloc((len + 1) * sizeof(char));
			strcpy(test->usp[idx3].utt, useful->wl.wav_name[i]);
			test->usp[idx3].spk = (char *)malloc((len + 1) * sizeof(char));
			strcpy(test->usp[idx3].spk, useful->wl.wav_name[i]);
			for (j = 0; j < len; j++)
			{
				if (test->usp[idx3].spk[j] == '-')
				{
					break;
				}
			}
			if (j < len)
			{
				test->usp[idx3].spk[j] = '\0';
			}
			if (test->spk_num == 0)
			{
				test->spk_num++;
			}
			else
			{
				if (strcmp(test->usp[idx3].spk, test->usp[idx3 - 1].spk) != 0)
				{
					test->spk_num++;
				}
			}
			idx3++;
		}
	}
}
void load_data(FILE *ftls, FILE *fwls, DATA_SET *ds, KALDI_LEXICONP_LIST *kll)
{
	int i, j;
	TEXT_LIST ttl;
	load_wav_list(fwls, &ds->all.wl);
	//for (i = 0; i < ds->all.wl.num_of_wav; i++)
	//{
	//	printf("%d %s %s\n", i+1, ds->all.wl.wav_full_name[i], ds->all.wl.wav_name[i]);
	//}
	load_text_list(ftls, &ttl, kll);
	copy_useful_text(&ds->all.tl, &ttl);
	// for (i = 0; i < ds->all.tl.num_of_text; i++)
	//{
	//	printf("%d %s %s\n", i + 1, ds->all.tl.text_full_name[i], ds->all.tl.text_name[i]);
	//	for (j = 0; j < ds->all.tl.txtlst[i].num_of_sentence; j++)
	//	{
	//		printf("\t%s\n", ds->all.tl.txtlst[i].sentence[j]);
	//	}
	// }
	take_useful_pair(&ds->all, &ds->useful, &ds->unpaired);
	if (ds->useful.tl.num_of_text != ds->useful.wl.num_of_wav)
	{
		printf("error: num_of_text(%d) != num_of_wav(%d)\n", ds->useful.tl.num_of_text, ds->useful.wl.num_of_wav);
		exit(1);
	}
	//for (i = 0; i < ds->useful.tl.num_of_text; i++)
	//{
	//	printf("%d %s %s %s %s\n", i + 1, ds->useful.tl.text_full_name[i], ds->useful.tl.text_name[i], ds->useful.wl.wav_full_name[i], ds->useful.wl.wav_name[i]);
	//	for (j = 0; j < ds->useful.tl.txtlst[i].num_of_sentence; j++)
	//	{
	//		printf("\t%s\n", ds->useful.tl.txtlst[i].sentence[j]);
	//	}
	//}
}
void copy_useful_text(TEXT_LIST *tl, TEXT_LIST *ttl)
{
	int i, j, tl_idx;
	tl->num_of_text = ttl->num_of_text;

	for (i = 0; i < ttl->num_of_text; i++)
	{
		if (ttl->txtlst[i].has_character_flag == 0)
		{
			tl->num_of_text--;
		}
	}
	tl->text_full_name = (char **)malloc(tl->num_of_text * sizeof(char *));
	tl->text_name = (char **)malloc(tl->num_of_text * sizeof(char *));
	tl->tokenized_text = (char **)malloc(tl->num_of_text * sizeof(char *));
	tl->txtlst = (TEXT *)malloc(tl->num_of_text * sizeof(TEXT));

	for (i = tl_idx = 0; i < ttl->num_of_text; i++)
	{
		if (ttl->txtlst[i].has_character_flag == 1)
		{
			tl->text_full_name[tl_idx] = (char *)malloc((strlen(ttl->text_full_name[i]) + 1) * sizeof(char));
			strcpy(tl->text_full_name[tl_idx], ttl->text_full_name[i]);
			tl->text_name[tl_idx] = (char *)malloc((strlen(ttl->text_name[i]) + 1) * sizeof(char));
			strcpy(tl->text_name[tl_idx], ttl->text_name[i]);
			tl->tokenized_text[tl_idx] = (char *)malloc((strlen(ttl->tokenized_text[i]) + 1) * sizeof(char));
			strcpy(tl->tokenized_text[tl_idx], ttl->tokenized_text[i]);

			tl->txtlst[tl_idx].has_character_flag = ttl->txtlst[i].has_character_flag;
			tl->txtlst[tl_idx].num_of_sentence = ttl->txtlst[i].num_of_sentence;
			tl->txtlst[tl_idx].sentence = (char **)malloc(ttl->txtlst[i].num_of_sentence * sizeof(char *));
			for (j = 0; j < ttl->txtlst[i].num_of_sentence; j++)
			{
				tl->txtlst[tl_idx].sentence[j] = (char *)malloc((strlen(ttl->txtlst[i].sentence[j]) + 1) * sizeof(char));
				strcpy(tl->txtlst[tl_idx].sentence[j], ttl->txtlst[i].sentence[j]);
			}

			tl_idx++;
		}
	}
}
void take_useful_pair(TEXT_AND_WAV *all, TEXT_AND_WAV *useful, TEXT_AND_WAV *unpaired)
{
	int i, j, ret, flag;
	STRING_SET_LIST atssl, awssl;

	gen_string_set_list(all->tl.text_name, all->tl.num_of_text, &atssl);
	gen_string_set_list(all->wl.wav_name, all->wl.num_of_wav, &awssl);

	useful->tl.text_full_name = (char **)malloc(all->tl.num_of_text * sizeof(char *));
	useful->tl.text_name = (char **)malloc(all->tl.num_of_text * sizeof(char *));
	useful->tl.tokenized_text = (char **)malloc(all->tl.num_of_text * sizeof(char *));
	useful->tl.txtlst = (TEXT *)malloc(all->tl.num_of_text * sizeof(TEXT));
	useful->wl.wav_full_name = (char **)malloc(all->wl.num_of_wav * sizeof(char *));
	useful->wl.wav_name = (char **)malloc(all->wl.num_of_wav * sizeof(char *));
	unpaired->tl.text_full_name = (char **)malloc(all->tl.num_of_text * sizeof(char *));
	unpaired->tl.text_name = (char **)malloc(all->tl.num_of_text * sizeof(char *));
	unpaired->tl.tokenized_text = (char **)malloc(all->tl.num_of_text * sizeof(char *));
	unpaired->tl.txtlst = (TEXT *)malloc(all->tl.num_of_text * sizeof(TEXT));
	unpaired->wl.wav_full_name = (char **)malloc(all->wl.num_of_wav * sizeof(char *));
	unpaired->wl.wav_name = (char **)malloc(all->wl.num_of_wav * sizeof(char *));

	for (i = 0, useful->tl.num_of_text = unpaired->tl.num_of_text = 0, flag = 0; i < all->tl.num_of_text; i++, flag = 0)
	{
		if (strlen(all->tl.text_name[i]) <= awssl.maxinum_len && strlen(all->tl.text_name[i]) > 0)
		{
			if (bin_string_search(all->tl.text_name[i], awssl.ss[strlen(all->tl.text_name[i]) - 1].as, awssl.ss[strlen(all->tl.text_name[i]) - 1].size) != -1)
			{
				useful->tl.text_full_name[useful->tl.num_of_text] = (char *)malloc((strlen(all->tl.text_full_name[i]) + 1) * sizeof(char));
				strcpy(useful->tl.text_full_name[useful->tl.num_of_text], all->tl.text_full_name[i]);
				useful->tl.text_name[useful->tl.num_of_text] = (char *)malloc((strlen(all->tl.text_name[i]) + 1) * sizeof(char));
				strcpy(useful->tl.text_name[useful->tl.num_of_text], all->tl.text_name[i]);
				useful->tl.tokenized_text[useful->tl.num_of_text] = (char *)malloc((strlen(all->tl.tokenized_text[i]) + 1) * sizeof(char));
				strcpy(useful->tl.tokenized_text[useful->tl.num_of_text], all->tl.tokenized_text[i]);
				useful->tl.txtlst[useful->tl.num_of_text].sentence = (char **)malloc(all->tl.txtlst[i].num_of_sentence * sizeof(char *));
				useful->tl.txtlst[useful->tl.num_of_text].num_of_sentence = all->tl.txtlst[i].num_of_sentence;
				for (j = 0; j < all->tl.txtlst[i].num_of_sentence; j++)
				{
					useful->tl.txtlst[useful->tl.num_of_text].sentence[j] = (char *)malloc((strlen(all->tl.txtlst[i].sentence[j]) + 1) * sizeof(char));
					strcpy(useful->tl.txtlst[useful->tl.num_of_text].sentence[j], all->tl.txtlst[i].sentence[j]);
				}

				useful->tl.num_of_text++;
				flag = 1;
			}
		}
		if (flag == 0)
		{
			unpaired->tl.text_full_name[unpaired->tl.num_of_text] = (char *)malloc((strlen(all->tl.text_full_name[i]) + 1) * sizeof(char));
			strcpy(unpaired->tl.text_full_name[unpaired->tl.num_of_text], all->tl.text_full_name[i]);
			unpaired->tl.text_name[unpaired->tl.num_of_text] = (char *)malloc((strlen(all->tl.text_name[i]) + 1) * sizeof(char));
			strcpy(unpaired->tl.text_name[unpaired->tl.num_of_text], all->tl.text_name[i]);
			unpaired->tl.tokenized_text[unpaired->tl.num_of_text] = (char *)malloc((strlen(all->tl.tokenized_text[i]) + 1) * sizeof(char));
			strcpy(unpaired->tl.tokenized_text[unpaired->tl.num_of_text], all->tl.tokenized_text[i]);
			unpaired->tl.txtlst[unpaired->tl.num_of_text].sentence = (char **)malloc(all->tl.txtlst[i].num_of_sentence * sizeof(char *));
			unpaired->tl.txtlst[unpaired->tl.num_of_text].num_of_sentence = all->tl.txtlst[i].num_of_sentence;
			for (j = 0; j < all->tl.txtlst[i].num_of_sentence; j++)
			{
				unpaired->tl.txtlst[unpaired->tl.num_of_text].sentence[j] = (char *)malloc((strlen(all->tl.txtlst[i].sentence[j]) + 1) * sizeof(char));
				strcpy(unpaired->tl.txtlst[unpaired->tl.num_of_text].sentence[j], all->tl.txtlst[i].sentence[j]);
			}

			unpaired->tl.num_of_text++;
		}
	}
	//printf("useful_text_size=%d\n", useful->tl.num_of_text);

	for (i = 0, useful->wl.num_of_wav = unpaired->wl.num_of_wav = 0, flag = 0; i < all->wl.num_of_wav; i++, flag = 0)
	{
		if (strlen(all->wl.wav_name[i]) <= atssl.maxinum_len && strlen(all->wl.wav_name[i]) > 0)
		{
			if (bin_string_search(all->wl.wav_name[i], atssl.ss[strlen(all->wl.wav_name[i]) - 1].as, atssl.ss[strlen(all->wl.wav_name[i]) - 1].size) != -1)
			{
				useful->wl.wav_full_name[useful->wl.num_of_wav] = (char *)malloc((strlen(all->wl.wav_full_name[i]) + 1) * sizeof(char));
				strcpy(useful->wl.wav_full_name[useful->wl.num_of_wav], all->wl.wav_full_name[i]);
				useful->wl.wav_name[useful->wl.num_of_wav] = (char *)malloc((strlen(all->wl.wav_name[i]) + 1) * sizeof(char));
				strcpy(useful->wl.wav_name[useful->wl.num_of_wav], all->wl.wav_name[i]);
				useful->wl.num_of_wav++;
				flag = 1;
			}
		}
		if (flag == 0)
		{
			unpaired->wl.wav_full_name[unpaired->wl.num_of_wav] = (char *)malloc((strlen(all->wl.wav_full_name[i]) + 1) * sizeof(char));
			strcpy(unpaired->wl.wav_full_name[unpaired->wl.num_of_wav], all->wl.wav_full_name[i]);
			unpaired->wl.wav_name[unpaired->wl.num_of_wav] = (char *)malloc((strlen(all->wl.wav_name[i]) + 1) * sizeof(char));
			strcpy(unpaired->wl.wav_name[unpaired->wl.num_of_wav], all->wl.wav_name[i]);
			unpaired->wl.num_of_wav++;
		}
	}
	//printf("useful_wave_size=%d\n", useful->wl.num_of_wav);
}
void gen_string_set_list(char **raw_set, int raw_size, STRING_SET_LIST *ssl)
{
	int i, idx;
	char **new_set = (char **)malloc(raw_size * sizeof(char *));

	for (i = 0, ssl->maxinum_len = 0; i < raw_size; i++)
	{
		new_set[i] = (char *)malloc((strlen(raw_set[i]) + 1) * sizeof(char));
		strcpy(new_set[i], raw_set[i]);
		if (ssl->maxinum_len < strlen(raw_set[i]))
		{
			ssl->maxinum_len = strlen(raw_set[i]);
		}
	}
	//printf("ssl->maxinum_len=%d\n", ssl->maxinum_len);
	ssl->ss = (STRING_SET *)malloc(ssl->maxinum_len * sizeof(STRING_SET));
	for (i = 0; i < ssl->maxinum_len; i++)
	{
		ssl->ss[i].size = 0;
	}
	qsort(new_set, raw_size, sizeof(char *), string_len_comparefun);

	//for (i = 0; i < raw_size; i++)
	//{
	//	printf("%s\n", new_set[i]);
	//}
	for (i = 0; i < raw_size; i++)
	{
		if (strlen(new_set[i]) > 0)
		{
			idx = strlen(new_set[i]) - 1;
			if (ssl->ss[idx].size == 0)
			{
				ssl->ss[idx].as = &new_set[i];
			}
			ssl->ss[idx].size++;
		}
	}
	for (i = 0; i < ssl->maxinum_len; i++)
	{
		qsort(ssl->ss[i].as, ssl->ss[i].size, sizeof(char *), stringcomparefun);
	}
}
void load_wav_list(FILE *fwls, WAV_LIST *wl)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int i, j;

	for (wl->num_of_wav = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fwls);
		if (line[0] != '\0')
		{
			wl->num_of_wav++;
		}
	}
	fseek(fwls, 0, SEEK_SET);
	wl->wav_full_name = (char **)malloc(wl->num_of_wav * sizeof(char *));
	wl->wav_name = (char **)malloc(wl->num_of_wav * sizeof(char *));

	for (i = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fwls);
		if (line[0] != '\0')
		{
			wl->wav_full_name[i] = (char *)malloc((strlen(line) + 1) * sizeof(char));
			wl->wav_name[i] = (char *)malloc((strlen(line) + 1) * sizeof(char));
			strcpy(wl->wav_full_name[i], line);
			i++;
		}
	}
	qsort(wl->wav_full_name, wl->num_of_wav, sizeof(char *), stringcomparefun);
	for (i = 0; i < wl->num_of_wav; i++)
	{
		for (j = strlen(wl->wav_full_name[i]); j >= 0; j--)
		{
			if (wl->wav_full_name[i][j] == '/')
			{
				break;
			}
		}
		if (j >= 0)
		{
			strcpy(wl->wav_name[i], &wl->wav_full_name[i][j + 1]);
		}
		else
		{
			strcpy(wl->wav_name[i], wl->wav_full_name[i]);
		}
		for (j = strlen(wl->wav_name[i]); j >= 0; j--)
		{
			if (wl->wav_name[i][j] == '.')
			{
				break;
			}
		}
		if (j >= 0)
		{
			wl->wav_name[i][j] = '\0';
		}
	}

	free(line);
}
void gen_dir(char *output_dir)
{
	struct stat st = {0};

	if (stat(output_dir, &st) == -1)
	{
		mkdir(output_dir, 0755);
	}
}
void reset_dir_data(char *dir)
{
	FILE *ftmp;
	char *output_name = (char *)malloc((strlen(dir) + 16) * sizeof(char));

	strcpy(output_name, dir);
	strcat(output_name, "/text");
	if (!(ftmp = fopen(output_name, "w")))
	{
		printf("can not open file: %s\n", output_name);
		exit(1);
	}
	else
	{
		fclose(ftmp);
	}
	strcpy(output_name, dir);
	strcat(output_name, "/wav.scp");
	if (!(ftmp = fopen(output_name, "w")))
	{
		printf("can not open file: %s\n", output_name);
		exit(1);
	}
	else
	{
		fclose(ftmp);
	}
	strcpy(output_name, dir);
	strcat(output_name, "/utt2spk");
	if (!(ftmp = fopen(output_name, "w")))
	{
		printf("can not open file: %s\n", output_name);
		exit(1);
	}
	else
	{
		fclose(ftmp);
	}
	free(output_name);
}
void output_a_line(FILE *fout, char *line)
{
	int idx;
	char word[7], last[7] = {'\0'};
	int eng_flag;

	for(idx=0, eng_flag=0;!read_a_utf8_word(line,word,&idx);){
		if (isEnglish(word) == 1)
		{
			normalize_English(word);
			if(eng_flag==0)
			{
				fprintf(fout,"%s",word);
			}
			else{
				fprintf(fout,"%s",word);
			}
			eng_flag=1;
		}
		else{
			if(eng_flag==1){
				if (isapostrophe(word) == 1)
				{
					fprintf(fout,"\'");
				}
				else{
					fprintf(fout," ");
				}
			}
			eng_flag=0;
			if(isCJK_words(word)==1||isnumber(word)==1){
				if (isnumber(word) == 1 && strlen(word) == 1)
				{
					fprintf(fout, "%s ", Number_char[word[0] - 48 + 10]);
				}
				else
				{
					fprintf(fout, "%s ", word);
				}
			}
		}
		if (isspace_and_tab(word) == 1 && isapostrophe(last) == 1)
		{
			fprintf(fout, " ");
		}
		strcpy(last, word);
	}
	if(eng_flag==1){
		fprintf(fout," ");
	}
	//fprintf(fout," ");
}
void load_text_list(FILE *flst, TEXT_LIST *tl, KALDI_LEXICONP_LIST *kll)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int i, j;
	FILE *fin;

	for (tl->num_of_text = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, flst);
		if (line[0] != '\0')
		{
			tl->num_of_text++;
		}
	}
	fseek(flst, 0, SEEK_SET);
	tl->text_full_name = (char **)malloc(tl->num_of_text * sizeof(char *));
	tl->text_name = (char **)malloc(tl->num_of_text * sizeof(char *));
	tl->txtlst = (TEXT *)malloc(tl->num_of_text * sizeof(TEXT));
	tl->tokenized_text = (char **)malloc(tl->num_of_text * sizeof(char *));

	for (i = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, flst);
		if (line[0] != '\0')
		{
			if (!(fin = fopen(line, "r")))
			{
				tl->num_of_text--;
			}
			else
			{
				//load_text(fin, &tl->txtlst[i]);
				fclose(fin);
				tl->text_full_name[i] = (char *)malloc((strlen(line) + 1) * sizeof(char));
				tl->text_name[i] = (char *)malloc((strlen(line) + 1) * sizeof(char));
				strcpy(tl->text_full_name[i++], line);
			}
		}
	}
	qsort(tl->text_full_name, tl->num_of_text, sizeof(char *), stringcomparefun);
	for (i = 0; i < tl->num_of_text; i++)
	{
		fin = fopen(tl->text_full_name[i], "r");
		load_text(fin, &tl->txtlst[i]);
		tokenizer(&tl->txtlst[i], &tl->tokenized_text[i]);
		if (strlen(tl->tokenized_text[i]) == 0)
		{
			printf("ignoring \'%s\': there are not any characters in\n", tl->text_full_name[i]);
			tl->txtlst[i].has_character_flag = 0;
		}
		else
		{
			if (check_tokon_is_exist_in_dict(tl->tokenized_text[i], kll) == 1)
			{
				tl->txtlst[i].has_character_flag = 1;
			}
			else
			{
				printf("ignoring \'%s\': OOV\n", tl->text_full_name[i]);
				tl->txtlst[i].has_character_flag = 0;
			}
		}
		// printf("%s\n", tl->tokenized_text[i]);
		fclose(fin);
		for (j = strlen(tl->text_full_name[i]); j >= 0; j--)
		{
			if (tl->text_full_name[i][j] == '/')
			{
				break;
			}
		}
		if (j >= 0)
		{
			strcpy(tl->text_name[i], &tl->text_full_name[i][j + 1]);
		}
		else
		{
			strcpy(tl->text_name[i], tl->text_full_name[i]);
		}
		for (j = strlen(tl->text_name[i]); j >= 0; j--)
		{
			if (tl->text_name[i][j] == '.')
			{
				break;
			}
		}
		if (j >= 0)
		{
			tl->text_name[i][j] = '\0';
		}
	}

	free(line);
}
int check_tokon_is_exist_in_dict(char *token, KALDI_LEXICONP_LIST *kll)
{
	int idx, step, ret, flag = 1;
	char *tmp = (char *)malloc((strlen(token) + 1) * sizeof(char));

	for (idx = step = 0; token[idx] != '\0'; step++)
	{
		if (token[idx + step] == ' ')
		{
			ret = 1;
			strncpy(tmp, &token[idx], step);
			tmp[step] = '\0';
			if (strlen(tmp) < kll->size)
			{
				ret = bin_kaldilexiconp_search(tmp, &kll->kls[strlen(tmp)-1]);
			}
			else
			{
				ret = -1;
			}
			if (ret == -1)
			{
				printf("%s does not exist in lexiconp\n", tmp);
				flag = 0;
			}
			idx += step + 1;
			step = 0;
		}
	}

	free(tmp);

	return flag;
}
void tokenizer(TEXT *text, char **tokenized_text)
{
	int i, length, idx, eng_flag;
	char word[7], *temp_eng, *temp_token;

	for (i = length = 0; i < text->num_of_sentence; i++)
	{
		length += (int)strlen(text->sentence[i]);
	}
	*tokenized_text = (char *)malloc((length * 2 + 1) * sizeof(char));
	*tokenized_text[0] = '\0';
	temp_eng = (char *)malloc((length + 1) * sizeof(char));
	temp_eng[0] = '\0';

	for (i = length = eng_flag = 0; i < text->num_of_sentence; i++)
	{
		for (idx = 0;;)
		{
			read_a_utf8_word(text->sentence[i], word, &idx);
			if (isCJK_words(word) == 1 || isnumber(word) == 1)
			{
				if (eng_flag == 1)
				{
					strcat(*tokenized_text, temp_eng);
					strcat(*tokenized_text, " ");
					temp_eng[0] = '\0';
					eng_flag = 0;
				}
				strcat(*tokenized_text, word);
				strcat(*tokenized_text, " ");
			}
			else if (isEnglish(word) == 1)
			{
				strcat(temp_eng, word);
				eng_flag = 1;
			}
			else if (isapostrophe(word) == 1)
			{
				if (eng_flag == 1)
				{
					read_a_utf8_word(text->sentence[i], word, &idx);
					if (isEnglish(word) == 1)
					{
						strcat(temp_eng, "\'");
						strcat(temp_eng, word);
					}
					else
					{
						if (word[0] != '\0')
						{
							Retreat_a_utf8_word(text->sentence[i], &idx);
						}
						strcat(*tokenized_text, temp_eng);
						strcat(*tokenized_text, " ");
						temp_eng[0] = '\0';
						eng_flag = 0;
					}
				}
			}
			else
			{
				if (eng_flag == 1)
				{
					strcat(*tokenized_text, temp_eng);
					strcat(*tokenized_text, " ");
					temp_eng[0] = '\0';
					eng_flag = 0;
				}
			}
			if (word[0] == '\0')
			{
				break;
			}
		}
	}
	temp_token = (char *)malloc((strlen(*tokenized_text) + 1) * sizeof(char));
	strcpy(temp_token, *tokenized_text);

	//normalize_word(temp_token, tokenized_text);

	free(temp_eng);
	free(temp_token);
}
void load_text(FILE *fin, TEXT *text_in)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int i, num_line;

	for (num_line = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fin);
		num_line++;
	}
	text_in->num_of_sentence = num_line;
	fseek(fin, 0, SEEK_SET);
	text_in->sentence = (char **)malloc(text_in->num_of_sentence * sizeof(char *));
	for (i = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fin);
		text_in->sentence[i] = (char *)malloc((strlen(line) + 1) * sizeof(char));
		strcpy(text_in->sentence[i++], line);
	}

	free(line);
}
int read_a_line(char **line, int *capacity_of_line, FILE *fin)
{
    char temp[1024];
    int cnt, times, ENDOFLINE = 0, FILEEND = 0;

    for (*line[0] = '\0', times = 1; ENDOFLINE == 0; times++)
    {
        for (cnt = 0; 1; cnt++)
        {
            temp[cnt] = fgetc(fin);
            if (temp[cnt] == '\n' || temp[cnt] == EOF || cnt == 1022)
            {
                if (temp[cnt] == '\n' || temp[cnt] == EOF)
                {
                    ENDOFLINE = 1;
                    if (temp[cnt] == EOF)
                    {
                        FILEEND = 1;
                    }
                    temp[cnt] = '\0';
                }
                break;
            }
        }
        if (*capacity_of_line < times * 1024)
        {
            *line = (char *)realloc(*line, times * 1024 * sizeof(char));
            *capacity_of_line = times * 1024;
            //printf("%d %d %d\n", times, (times - 1) * 1024 - times + 1, *capacity_of_line);
            (*line)[(times - 1) * 1024 - times + 1] = '\0';
        }
        if (cnt == 1022)
        {
            temp[cnt + 1] = '\0';
        }
        strcat(*line, temp);
    }
    //printf("%s\n", *line);

    return FILEEND;
}
int compare( const void *a, const void *b)
{
	char **a1=(char**)a,**b1=(char**)b;
	return( strcmp(a1[0],b1[0]) );
}
int read_a_utf8_word(char *line, char *word, int *index){
	unsigned char trick = 64;//0x01000000
	unsigned char trick1 = 128;//0x01000000
	int i = 1;

	
	word[0] = line[(*index)++];
	if (word[0] == '\0'){
		word[i] = '\0';
		(*index)--;
		return 1;
	}
	while (word[0] & trick&&word[0] & trick1){
		word[i++] = line[(*index)++];
		trick = trick >> 1;
	}
	word[i] = '\0';

	return 0;
}
int Retreat_a_utf8_word(char *line, int *index){
	if ((*index) == 0){
		return 0;
	}

	if (line[(*index) - 1] > 0){
		(*index) -= 1;
		return 1;
	}
	else if (line[(*index) - 2] >= -64 && line[(*index) - 2] <= -33){
		(*index) -= 2;
		return 2;
	}
	else if (line[(*index) - 3] >= -32 && line[(*index) - 3] <= -17){
		(*index) -= 3;
		return 3;
	}
	else if (line[(*index) - 4] >= -16 && line[(*index) - 4] <= -9){
		(*index) -= 4;
		return 4;
	}
	else if (line[(*index) - 5] >= -8 && line[(*index) - 5] <= -5){
		(*index) -= 5;
		return 5;
	}
	else if (line[(*index) - 6] >= -4 && line[(*index) - 6] <= -3){
		(*index) -= 6;
		return 6;
	}
	else{
		fprintf(stdout, "Retreat error : not utf-8 format.\n");
		exit(1);
	}
	return 1;
}
int isnumber(char *word){
	int i;
	for (i = 0; i < 20; i++){
		if (strcmp(word, Number_char[i]) == 0){
			return 1;
		}
	}
	return 0;
}
void normalize_English(char *word){
	if(strlen(word)==1){
		if(word[0]>=97&&word[0]<=122){
			word[0]-=32;
		}
	}
	else{
		if(word[2]>=-95&&word[2]<=-70){
			word[0]=word[2]+160;
			word[1]='\0';
		}
		else{
			word[0]=word[2]+192;
			word[1]='\0';
		}
	}
}
int isspace_and_tab(char *word){
	int i;
	for (i = 0; i < 3; i++){
		if (strcmp(word, space_and_tab[i]) == 0){
			return 1;
		}
	}
	return 0;
}
int isapostrophe(char *word){
	int i;
	for (i = 0; i < 2; i++){
		if (strcmp(word, apostrophe[i]) == 0){
			return 1;
		}
	}
	return 0;
}
int isCJK_words(char *word)
{
	//class										unicode		UTF-8				UTF-8_deciaml
	//CJK_Kangxi_Radicals						2F00-2FDF	e2bc80-e2bf9f		14859392-14860191
	//CJK_Radicals_Supplement					2E80-2EFF	e2ba80-e2bbbf		14858880-14859199
	//CJK_Unified_Ideographs_Extension_A		3400-4DBF	e39080-e4b6bf		14913664-14988991
	//CJK-Unified_Ideographs					4E00-9FFF	e4b880-e9bfbf		14989440-15318975
	//CJK-Compatibility_Ideographs				F900-FAFF	efa480-efabbf		15705216-15707071
	//CJK_Unified_Ideographs_Extension_B		20000-2A6DF	f0a08080-f0aa9b9f	4037050496-4037712799
	//CJK_Unified_Ideographs_Extension_C		2A700-2B73F	f0aa9c80-f0ab9cbf	4037713024-4037778623
	//CJK_Unified_Ideographs_Extension_D		2B740-2B81F	f0ab9d80-f0aba09f	4037778816-4037779615
	//CJK_Unified_Ideographs_Extension_E		2B820-2CEAF	f0aba0a0-f0acbaaf	4037779616-4037851823
	//CJK_Unified_Ideographs_Extension_F		2CEB0-2EBEF	f0acbab0-f0aeafaf	4037851824-4037980079
	//CJK-Compatibility_Ideographs_Supplement	2F800-2FA1F	f0afa080-f0afa89f	4038041728-4038043807
	//CJK_Unified_Ideographs_Extension_G		30000-3134A	f0b08080-f0b18d8a	4038099072-4038167946

	unsigned long long int utf8_value = 0;
	if (strlen(word) == 3)
	{
		utf8_value = utf8word2decimal(word);
		if ((utf8_value >= 14859392 && utf8_value <= 14860191) ||
			(utf8_value >= 14858880 && utf8_value <= 14859199) ||
			(utf8_value >= 14913664 && utf8_value <= 14988991) ||
			(utf8_value >= 14989440 && utf8_value <= 15318975) ||
			(utf8_value >= 15705216 && utf8_value <= 15707071))
		{
			return 1;
		}
	}
	else if (strlen(word) == 4)
	{
		utf8_value = utf8word2decimal(word);
		if ((utf8_value >= 4037050496 && utf8_value <= 4037712799) ||
			(utf8_value >= 4037713024 && utf8_value <= 4037778623) ||
			(utf8_value >= 4037778816 && utf8_value <= 4037779615) ||
			(utf8_value >= 4037779616 && utf8_value <= 4037851823) ||
			(utf8_value >= 4037851824 && utf8_value <= 4037980079) ||
			(utf8_value >= 4038041728 && utf8_value <= 4038043807) ||
			(utf8_value >= 4038099072 && utf8_value <= 4038167946))
		{
			return 1;
		}
	}
	// printf("%s %lu\n",word, utf8_value);

	return 0;
}
unsigned long long int utf8word2decimal(char *word){
	unsigned long long int ret = 0;
	int num_byte = strlen(word);
	int i;
	
	for(i=0; i<num_byte; i++){
		ret = ret<<8;
		ret += ((unsigned long long int)word[i])&255;
	}
	
	
	return ret;
}
int stringcomparefun(const void *a, const void *b)
{
	const char *pa = (*(char **)a);
	const char *pb = (*(char **)b);
	return strcmp(pa, pb);
}
int string_len_comparefun(const void *a, const void *b)
{
	const char *pa = (*(char **)a);
	const char *pb = (*(char **)b);
	if (strlen(pa) > strlen(pb))
	{
		return 1;
	}
	else if (strlen(pa) == strlen(pb))
	{
		return 0;
	}
	else
	{
		return -1;
	}
	return 0;
}
int bin_string_search(char *sentence, char **set, int set_size)
{
	int i, low = 0, up = set_size - 1, middle;

	if (set_size <= 0)
	{
		return -1;
	}

	while (1)
	{
		middle = (low + up) * 0.5;
		//printf("%s %d %d %d\n", set[middle], low, up, middle);
		if (strcmp(sentence, set[middle]) > 0)
		{
			low = middle + 1;
		}
		else if (strcmp(sentence, set[middle]) == 0)
		{
			return middle;
		}
		else if (strcmp(sentence, set[middle]) < 0)
		{
			up = middle - 1;
		}
		if ((up == low && up == middle) || up < low)
		{
			return -1;
		}
	}
}
int utf8_word_length(char *word)
{
	int length = 0;
	unsigned int index = 0, end_idx = strlen(word);
	for (; index < end_idx;)
	{
		if (word[index] > 0)
			index += 1;
		else if (word[index] > -65 && word[index] < -32)
			index += 2;
		else if (word[index] > -33 && word[index] < -16)
			index += 3;
		else if (word[index] > -17 && word[index] < -8)
			index += 4;
		else if (word[index] > -9 && word[index] < -4)
			index += 5;
		else if (word[index] > -5 && word[index] < -2)
			index += 6;

		length++;
	}
	return length;
}
void normalize_word(char *raw, char **modify)
{
	int idx;
	char word[7];
	char Dash[5][4] = {
		{"-"},
		{-17, -68, -115, '\0'},	 /**1  －**/
		{-30, -128, -109, '\0'}, /**2  –**/
		{-30, -128, -108, '\0'}, /**3  —**/
		{-30, -108, -128, '\0'}	 /**4  ─**/
	};
	char full_apostrophe[4] = {-30, -128, -103, '\0'};
	char spaces[3][4] = {
		{" "},
		{"\t"},
		{-29, -128, -128, '\0'}, /*full shape space*/
	};

	(*modify) = (char *)realloc((*modify), (strlen(raw) * 3 + 1) * sizeof(char));
	(*modify)[0] = '\0';
	for (idx = 0; read_a_utf8_word(raw, word, &idx) == 0;)
	{
		if (strlen(word) == 1)
		{
			if (word[0] >= 97 && word[0] <= 122)
			{
				strcat((*modify), English[(int)word[0] - 97]);
			}
			else
			{
				strcat((*modify), word);
			}
		}
		else if (strlen(word) == 3)
		{
			if (word[0] == -17 && word[1] == -68 && (word[2] >= -95 && word[2] <= -70))
			{
				strcat((*modify), English[(int)word[2] + 95]);
			}
			else if (word[0] == -17 && word[1] == -67 && (word[2] >= -127 && word[2] <= -102))
			{
				strcat((*modify), English[(int)word[2] + 127]);
			}
			else
			{
				strcat((*modify), word);
			}
		}
		else if (strcmp(word, Dash[1]) == 0 || strcmp(word, Dash[2]) == 0 || strcmp(word, Dash[3]) == 0 || strcmp(word, Dash[4]) == 0)
		{
			strcat((*modify), "-");
		}
		else if (strcmp(word, full_apostrophe) == 0)
		{
			strcat((*modify), "\'");
		}
		else if (strcmp(word, spaces[1]) == 0 || strcmp(word, spaces[2]) == 0)
		{
			strcat((*modify), " ");
		}
		else
		{
			strcat((*modify), word);
		}
	}
}
int isEnglish(char *word)
{
	/*int i;
	for (i = 0; i < 104; i++)
	{
		if (strcmp(word, English[i]) == 0)
		{
			return 1;
		}
	}*/
	if (strlen(word) == 1)
	{
		if ((word[0] >= 65 && word[0] <= 90) || (word[0] >= 97 && word[0] <= 122))
		{
			return 1;
		}
	}
	else if (strlen(word) == 3)
	{
		if (word[0] == -17)
		{
			if (word[1] == -67)
			{
				if (word[2] >= -127 && word[2] <= -102)
				{
					return 1;
				}
			}
			else if (word[1] == -68)
			{
				if (word[2] >= -95 && word[2] <= -70)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}
int isDot(char *word)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		if (strcmp(word, Dot[i]) == 0)
		{
			return 1;
		}
	}
	return 0;
}
void load_kaldi_lexp(FILE *fdic, KALDI_LEXICONP *kl, KALDI_LEXICONP_LIST *kll)
{
	int FILEEND, capacity_of_line = 0;
	char *temp;
	char *line = (char *)malloc(sizeof(char));
	int i, j, idx, len, sp_num;

	for (kl->size = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fdic);
		if (line[0] != '\0')
		{
			kl->size++;
		}
	}
	fseek(fdic, 0, SEEK_SET);

	temp = (char *)malloc(capacity_of_line * sizeof(char));
	kl->klt = (KALDI_LEXICONP_TOKEN *)malloc(kl->size * sizeof(KALDI_LEXICONP_TOKEN));

	for (i = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fdic);
		if (line[0] != '\0')
		{
			temp[0] = '\0';
			normalize_continous_space_tab(line, temp);
			for (j = 0, sp_num = 0; temp[j] != '\0'; j++)
			{
				if (temp[j] == ' ')
				{
					sp_num++;
				}
			}
			if (sp_num < 2)
			{
				printf("dictionary format error: %s\n", line);
				exit(1);
			}
			for (idx = len = 0; temp[idx + len] != ' '; len++)
			{
				continue;
			}
			kl->klt[i].word = (char *)malloc((len + 1) * sizeof(char));
			strncpy(kl->klt[i].word, &temp[idx], len);
			kl->klt[i].word[len] = '\0';

			idx += len + 1;
			for (len = 0; temp[idx + len] != ' '; len++)
			{
				continue;
			}
			temp[idx + len] == '\0';
			kl->klt[i].prob = atof(&temp[idx]);

			idx += len + 1;
			for (len = 0; temp[idx + len] != '\0'; len++)
			{
				continue;
			}
			kl->klt[i].phseq = (char *)malloc((len + 1) * sizeof(char));
			strncpy(kl->klt[i].phseq, &temp[idx], len);
			kl->klt[i].phseq[len] = '\0';
			// printf("%s %f %s\n", kl->klt[i].word, kl->klt[i].prob, kl->klt[i].phseq);
			i++;
		}
	}
	qsort(kl->klt, kl->size, sizeof(KALDI_LEXICONP_TOKEN), kaldilexiconcomparefun);
	//for (i = 0; i < kl->size; i++)
	//{
	//	printf("%s %f %s\n", kl->klt[i].word, kl->klt[i].prob, kl->klt[i].phseq);
	//}
	//printf("%d\n", kl->size);
	//exit(1);
	kll->size = strlen(kl->klt[kl->size - 1].word);
	kll->kls = (KALDI_LEXICONP *)malloc(kll->size * sizeof(KALDI_LEXICONP));
	for (i = 0; i < kll->size; i++)
	{
		kll->kls[i].size = 0;
	}
	for (i = idx = 0, len = strlen(kl->klt[0].word); idx + i <= kl->size; i++)
	{
		if (idx + i == kl->size)
		{
			kll->kls[len - 1].size = i;
			kll->kls[len - 1].klt = &kl->klt[idx];
			break;
		}
		else if (strlen(kl->klt[idx + i].word) != len)
		{
			kll->kls[len - 1].size = i;
			kll->kls[len - 1].klt = &kl->klt[idx];
			len = strlen(kl->klt[idx + i].word);
			idx += i;
			i = 0;
		}
	}
	// for (i = 0; i < kll->size; i++)
	//{
	//	printf("%d %d\n", i + 1, kll->kls[i].size);
	// }

	free(line);
	free(temp);
}
int kaldilexiconcomparefun(const void *a, const void *b)
{
	const char *pa = ((KALDI_LEXICONP_TOKEN *)a)->word;
	const char *pb = ((KALDI_LEXICONP_TOKEN *)b)->word;
	if (strlen(pa) > strlen(pb))
	{
		if (strcmp(pa, pb) > 0)
		{
			return 3;
		}
		else if (strcmp(pa, pb) == 0)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
	else if (strlen(pa) == strlen(pb))
	{
		if (strcmp(pa, pb) > 0)
		{
			return 1;
		}
		else if (strcmp(pa, pb) == 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		if (strcmp(pa, pb) > 0)
		{
			return -1;
		}
		else if (strcmp(pa, pb) == 0)
		{
			return -2;
		}
		else
		{
			return -3;
		}
	}
	return 0;
}
void normalize_continous_space_tab(char *raw, char *mod)
{
	int idx;
	char word[7], last[7] = {'\0'};

	for (idx = 0; !read_a_utf8_word(raw, word, &idx);)
	{
		if (word[0] == ' ' || word[0] == '\t')
		{
			if (last[0] != ' ' && last[0] != '\t')
			{
				strcat(mod, " ");
			}
		}
		else
		{
			strcat(mod, word);
		}
		strcpy(last, word);
	}
	if (mod[strlen(mod) - 1] == ' ')
	{
		mod[strlen(mod) - 1] = '\0';
	}
	if (mod[0] == ' ')
	{
		strcpy(mod, &mod[1]);
	}
}
int bin_kaldilexiconp_search(char *word, KALDI_LEXICONP *kl)
{
	int i, low = 0, up = kl->size - 1, middle;

	if (up < 0)
	{
		return -1;
	}

	while (1)
	{
		middle = (low + up) * 0.5;
		if (strcmp(word, kl->klt[middle].word) > 0)
		{
			low = middle + 1;
		}
		else if (strcmp(word, kl->klt[middle].word) == 0)
		{
			return middle;
		}
		else if (strcmp(word, kl->klt[middle].word) < 0)
		{
			up = middle - 1;
		}
		if ((up == low && up == middle) || up < low)
		{
			return -1;
		}
	}
}