#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

typedef struct _SINGLEWORD_
{
	char *word;
	double prob_outcome;
	int outcome_times;
	char *syl_code_seq;
	char *phoneme_seq;
} SINGLEWORD;
typedef struct _LEXICON_
{
	SINGLEWORD *sw;
	int num_word;
	int capacity;
} LEXICON;
typedef struct _LENDICT_
{
	int maximum_len;
	LEXICON *lexs;
	int total_lexicon_outcome;
} LENDICT;
typedef struct _PRONUNCIATION_TABLE_
{
	int size;
	char **phcode;
	char **phonetic;
	char **pinyin;
	char **initial;
	char **final;
	char **medials;
	char **rime;
	char **nucleus;
	char **coda;
} PRONUNCIATION_TABLE;
typedef struct _TEXT_PAIR_
{
	int size, exist_flag;
	char **raw, **mod;
} TEXT_PAIR;
typedef struct _MODIFY_TEXT_
{
	TEXT_PAIR text[3];//test,dev,train
} MODIFY_TEXT;
typedef struct _NUCLUES_
{
	char *name;
	int tone_size, *possible_tone;
} NUCLUES;
typedef struct _NUCLUES_SET_
{
	int size;
	NUCLUES *nuc;
} NUCLUES_SET;
typedef struct _NOT_NUCLUES_SET_
{
	int size;
	char **name;
} NOT_NUCLUES_SET;
typedef struct _PHONEME_SET_
{
	NUCLUES_SET ns;
	NOT_NUCLUES_SET nns;
} PHONEME_SET;
typedef struct _SYLLABLE_TOKEN_
{
	char *name;
	char tone;
} SYLLABLE_TOKEN;
typedef struct _SYLLABLE_SET_
{
	int size, capacity;
	SYLLABLE_TOKEN *st;
} SYLLABLE_SET;

void nonsilence_phones_output(PHONEME_SET *ch, PHONEME_SET *eng, char *output_data_path);
void extra_questions_output(PHONEME_SET *ch, PHONEME_SET *eng, char *output_data_path);
void get_phone_set(LEXICON *lex, PHONEME_SET *ch, PHONEME_SET *eng, int tone_flag);
void output_extra_questions_and_nonsilence_phones(LEXICON *lex, char *output_data_path, int tone_flag);
void copy_dict(LEXICON *raw_lex, LEXICON *mod_lex, int tone_flag);
void phoneme2inifinorsyllable(char *input, char *output, PRONUNCIATION_TABLE *pt, int iors); //1 fot inifin, 2 for syllable
void split_word(char *word, char *phs, char **w0, char **w1);
void change_lex_src_and_target(LEXICON *raw_lex, LEXICON *mod_lex, PRONUNCIATION_TABLE *pt, char *src_type, char *target_type);
void output_dict(char *s5_dir, char *type, LEXICON *lex, int tone_flag);
void copy_data(char *src_dir, char *des_dir, char *name);
void output_text(char *s5_dir, char *type, MODIFY_TEXT *mt);
void gen_dir(char *s5_dir, char *type, MODIFY_TEXT *mt);
void output_data(char *s5_dir, char *type, MODIFY_TEXT *mt, LEXICON *lex, int tone_flag);
int lexiconWPcomparefun(const void *a, const void *b);
int syllablecomparefun(const void *a, const void *b);
void gen_dict_from_phonemetable(PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, LEXICON *lex, PHONEME_SET *ps, char *type, int tone_flag);
void load_phoneme_set(FILE *fnsp, PHONEME_SET *ps);
void convet_dictionary(PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, LEXICON *raw_lex, LEXICON *new_lex, char *src_type, char *target_type, int src_tone_flag, int target_tone_flag, char *s5_dir);
unsigned long long int utf8word2decimal(char *word);
int bin_lexicon_search_len_match(char *word, LEXICON *lex);
void word2sylorph(char *word, char *output, LENDICT *ld, PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, char *type, int tone_flag);
int isCJK_words(char *word);
int read_a_utf8_word(char *line, char *word, int *index);
void transform_raw_text(TEXT_PAIR *tp, LENDICT *ld, PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, char *type, int tone_flag);
int load_raw_text(char ***lines, FILE *ftxt);
void convert_text(MODIFY_TEXT *mt, LENDICT *ld, PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, char *type, int tone_flag, char *s5_dir);
void split_dictionary(LENDICT *ld, LEXICON *lex);
int lexilenconcomparefun(const void *a, const void *b);
int lexiconcomparefun(const void *a, const void *b);
void load_lexicon(FILE *fdict, LEXICON *lex);
void build_lexicon(char *s5_dir, LEXICON *lex, LENDICT *ld);
int Retreat_a_utf8_word(char *line, int *index);
int utf8_word_length(char *word);
int read_a_line(char **line, int *capacity_of_line, FILE *fin);
void load_pronunciation_table(PRONUNCIATION_TABLE *pt, FILE *fpt);
void run(FILE *fpt, char *src_type, char *traget_type, int src_tone_flag, int target_tone_flag, char *s5_dir);
int main(int argc, char **argv)
{
	FILE *fpt;
	if (argc != 7)
	{
		printf("Usage: excute.exe phone_tab src-type(phoneme/inifin/syllable) target-type(word/phoneme/inifin/syllable) src-tone-flag(0/1) target-tone-flag s5_dir\n");
		exit(1);
	}
	if (!(fpt = fopen(argv[1], "r")))
	{
		printf("can not open file: %s\n", argv[1]);
		exit(1);
	}
	if (strcmp(argv[2], "phoneme") != 0 && strcmp(argv[2], "inifin") != 0 && strcmp(argv[2], "syllable") != 0)
	{
		printf("kind of src must one of below\nphoneme inifin syllable\n");
		exit(1);
	}
	if (strcmp(argv[3], "phoneme") != 0 && strcmp(argv[3], "inifin") != 0 && strcmp(argv[3], "syllable") != 0 && strcmp(argv[3], "word") != 0)
	{
		printf("kind of target must one of below\nphoneme inifin syllable\n");
		exit(1);
	}

	if ((strcmp(argv[3], "phoneme") == 0 && strcmp(argv[2], "phoneme") != 0) ||
		(strcmp(argv[3], "inifin") == 0 && (strcmp(argv[2], "phoneme") != 0 && strcmp(argv[2], "inifin") != 0)) ||
		(strcmp(argv[3], "syllable") == 0 && (strcmp(argv[2], "phoneme") != 0 && strcmp(argv[2], "inifin") != 0 && strcmp(argv[2], "syllable") != 0)))
	{
		printf("illegal format: src-type:%s, target-type:%s\n", argv[2], argv[3]);
		exit(1);
	}
	if (strcmp(argv[3], "word") == 0 && strcmp(argv[2], "phoneme") == 0)
	{
		printf("It's equal to raw data, nothing be done.\n");
		exit(1);
	}

	run(fpt, argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), argv[6]);

	fclose(fpt);
	return 0;
}
void run(FILE *fpt, char *src_type, char *traget_type, int src_tone_flag, int target_tone_flag, char *s5_dir)
{
	LEXICON lex, n_lex;
	LENDICT ld;
	PRONUNCIATION_TABLE pt;
	MODIFY_TEXT mt;
	SYLLABLE_SET ss;
	char *src_target_type = (char *)malloc((strlen(src_type) + strlen(traget_type) + 2) * sizeof(char));

	int i;
	char *checked_s5_dir = (char *)malloc((strlen(s5_dir) + 2) * sizeof(char));

	strcpy(checked_s5_dir, s5_dir);
	if (checked_s5_dir[strlen(checked_s5_dir) - 1] != '/')
	{
		checked_s5_dir[strlen(checked_s5_dir) + 1] = '\0';
		checked_s5_dir[strlen(checked_s5_dir)] = '/';
	}

	strcpy(src_target_type, src_type);
	strcat(src_target_type, "2");
	strcat(src_target_type, traget_type);

	load_pronunciation_table(&pt, fpt);
	//for (i = 0; i < pt.size; i++)
	//{
	//	printf("%d %s %s %s %s\n", i, pt.initial[i], pt.medials[i], pt.nucleus[i], pt.coda[i]);
	//}
	//exit(1);
	build_lexicon(checked_s5_dir, &lex, &ld);
	convert_text(&mt, &ld, &pt, &ss, traget_type, target_tone_flag, checked_s5_dir);
	convet_dictionary(&pt, &ss, &lex, &n_lex, src_type, traget_type, src_tone_flag, target_tone_flag, checked_s5_dir);
	printf("dictionary_size=%d\n", n_lex.num_word);
	output_data(checked_s5_dir, src_target_type, &mt, &n_lex, src_tone_flag);

	free(checked_s5_dir);
}
void output_data(char *s5_dir, char *type, MODIFY_TEXT *mt, LEXICON *lex, int tone_flag)
{
	gen_dir(s5_dir, type, mt);
	output_text(s5_dir, type, mt);
	output_dict(s5_dir, type, lex, tone_flag);
}
void output_dict(char *s5_dir, char *type, LEXICON *lex, int tone_flag)
{
	FILE *fin, *fout;
	int capacity_of_line = 0, FILEEND;
	char *line = (char *)malloc(sizeof(char));
	char *output_data_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	char *output_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	char *data_src_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	char *copy_src_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	int i;

	strcpy(output_data_path, s5_dir);
	strcat(output_data_path, "data_");
	strcat(output_data_path, type);
	strcat(output_data_path, "/local/dict/");

	strcpy(data_src_path, s5_dir);
	strcat(data_src_path, "data/local/dict/");

	strcpy(output_path, output_data_path);
	strcat(output_path, "lexicon.txt");
	if (!(fout = fopen(output_path, "w")))
	{
		printf("can not open file: %s\n", output_path);
		exit(1);
	}
	for (i = 0; i < lex->num_word; i++)
	{
		fprintf(fout, "%s %s\n", lex->sw[i].word, lex->sw[i].phoneme_seq);
	}
	fclose(fout);

	if (strncmp(type, "phoneme", 7) == 0)
	{
		copy_data(data_src_path, output_data_path, "extra_questions.txt");
		copy_data(data_src_path, output_data_path, "nonsilence_phones.txt");
	}
	else
	{
		output_extra_questions_and_nonsilence_phones(lex, output_data_path, tone_flag);
		
	}
	copy_data(data_src_path, output_data_path, "optional_silence.txt");
	copy_data(data_src_path, output_data_path, "silence_phones.txt");

	free(line);
	free(output_data_path);
	free(output_path);
	free(copy_src_path);
	free(data_src_path);
}
void output_extra_questions_and_nonsilence_phones(LEXICON *lex, char *output_data_path, int tone_flag)
{
	int i, j;
	PHONEME_SET ch, eng;
	
	get_phone_set(lex, &ch, &eng, tone_flag);

	//printf("mandarin-phones:\n");
	//printf("\tnon-vowels:\n");
	//for (i = 0; i < ch.nns.size; i++)
	//{
	//	printf("\t\t%s\n", ch.nns.name[i]);
	//}
	//printf("\tvowels:\n");
	//for (i = 0; i < ch.ns.size; i++)
	//{
	//	printf("\t\t%s", ch.ns.nuc[i].name);
	//	for (j = 0; j < ch.ns.nuc[i].tone_size; j++)
	//	{
	//		printf(" %d", ch.ns.nuc[i].possible_tone[j]);
	//	}
	//	printf("\n");
	//}
	//printf("english-phones:\n");
	//printf("\tnon-vowels:\n");
	//for (i = 0; i < eng.nns.size; i++)
	//{
	//	printf("\t\t%s\n", eng.nns.name[i]);
	//}
	//printf("\tvowels:\n");
	//for (i = 0; i < eng.ns.size; i++)
	//{
	//	printf("\t\t%s", eng.ns.nuc[i].name);
	//	for (j = 0; j < eng.ns.nuc[i].tone_size; j++)
	//	{
	//		printf(" %d", eng.ns.nuc[i].possible_tone[j]);
	//	}
	//	printf("\n");
	//}

	extra_questions_output(&ch, &eng, output_data_path);
	nonsilence_phones_output(&ch, &eng, output_data_path);
}
void nonsilence_phones_output(PHONEME_SET *ch, PHONEME_SET *eng, char *output_data_path)
{
	FILE *fnonsil;

	int i, j;

	char *output_name = (char *)malloc((strlen(output_data_path) + 64) * sizeof(char));

	strcpy(output_name, output_data_path);
	strcat(output_name, "nonsilence_phones.txt");
	if (!(fnonsil = fopen(output_name, "w")))
	{
		printf("can not open file: %s\n", output_name);
		exit(1);
	}

	for (i = 0; i < eng->nns.size; i++)
	{
		fprintf(fnonsil, "%s\n", eng->nns.name[i]);
	}
	for (i = 0; i < eng->ns.size; i++)
	{
		fprintf(fnonsil, "%s", eng->ns.nuc[i].name);
		for (j = 0; j < eng->ns.nuc[i].tone_size; j++)
		{
			fprintf(fnonsil, " %s%d", eng->ns.nuc[i].name, eng->ns.nuc[i].possible_tone[j]);
		}
		fprintf(fnonsil, "\n");
	}
	for (i = 0; i < ch->nns.size; i++)
	{
		fprintf(fnonsil, "%s\n", ch->nns.name[i]);
	}
	for (i = 0; i < ch->ns.size; i++)
	{
		fprintf(fnonsil, "%s", ch->ns.nuc[i].name);
		for (j = 0; j < ch->ns.nuc[i].tone_size; j++)
		{
			fprintf(fnonsil, " %s%d", ch->ns.nuc[i].name, ch->ns.nuc[i].possible_tone[j]);
		}
		fprintf(fnonsil, "\n");
	}

	free(output_name);
	fclose(fnonsil);
}
void extra_questions_output(PHONEME_SET *ch, PHONEME_SET *eng, char *output_data_path)
{
	FILE *fextra;
	int poss_eng_tones[3] = {0, 1, 2};
	int poss_ch_tones[5] = {1, 2, 3, 4, 5};
	int i, j, k, flag;
	char *output_name = (char *)malloc((strlen(output_data_path) + 64) * sizeof(char));
	

	strcpy(output_name, output_data_path);
	strcat(output_name, "extra_questions.txt");
	if (!(fextra = fopen(output_name, "w")))
	{
		printf("can not open file: %s\n", output_name);
		exit(1);
	}
	fprintf(fextra, "SIL SPN NSN\n");
	for (i = 0; i < eng->ns.size; i++)
	{
		fprintf(fextra, "%s", eng->ns.nuc[i].name);
		if (i < eng->ns.size - 1)
		{
			fprintf(fextra, " ");
		}
	}
	if (eng->ns.size > 0)
	{
		fprintf(fextra, " ");
	}
	for (i = 0; i < eng->nns.size; i++)
	{
		fprintf(fextra, "%s", eng->nns.name[i]);
		if (i < eng->nns.size - 1)
		{
			fprintf(fextra, " ");
		}
	}
	fprintf(fextra, "\n");
	for (i = 0; i < ch->ns.size; i++)
	{
		fprintf(fextra, "%s", ch->ns.nuc[i].name);
		if (i < ch->ns.size - 1)
		{
			fprintf(fextra, " ");
		}
	}
	if (ch->ns.size > 0)
	{
		fprintf(fextra, " ");
	}
	for (i = 0; i < ch->nns.size; i++)
	{
		fprintf(fextra, "%s", ch->nns.name[i]);
		if (i < ch->nns.size - 1)
		{
			fprintf(fextra, " ");
		}
	}
	fprintf(fextra, "\n");

	for (i = 0; i < 3; i++)
	{
		for (flag = 0, j = 0; j < eng->ns.size; j++)
		{
			for (k = 0; k < eng->ns.nuc[j].tone_size; k++)
			{
				if (eng->ns.nuc[j].possible_tone[k] == poss_eng_tones[i])
				{
					if (flag == 1)
					{
						fprintf(fextra, " ");
					}
					fprintf(fextra, "%s%d", eng->ns.nuc[j].name, eng->ns.nuc[j].possible_tone[k]);
					flag = 1;
				}
			}
		}
		if (flag == 1)
		{
			fprintf(fextra, "\n");
		}
	}

	for (i = 0; i < 5; i++)
	{
		for (flag = 0, j = 0; j < ch->ns.size; j++)
		{
			for (k = 0; k < ch->ns.nuc[j].tone_size; k++)
			{
				if (ch->ns.nuc[j].possible_tone[k] == poss_ch_tones[i])
				{
					if (flag == 1)
					{
						fprintf(fextra, " ");
					}
					fprintf(fextra, "%s%d", ch->ns.nuc[j].name, ch->ns.nuc[j].possible_tone[k]);
					flag = 1;
				}
			}
		}
		if (flag == 1)
		{
			fprintf(fextra, "\n");
		}
	}

	free(output_name);
	fclose(fextra);
}
void get_phone_set(LEXICON *lex, PHONEME_SET *ch, PHONEME_SET *eng, int tone_flag)
{
	int i, j, idx, step, len;
	char **all_phs, tmp[1024];
	int all_size = 0, all_capacity = 1024;
	char **chv, **engv;
	int chv_size, engv_size;
	int *chv_possible_tone_size, *engv_possible_tone_size;

	all_phs = (char **)malloc(all_capacity*sizeof(char *));

	for (i = 0; i < lex->num_word; i++)
	{
		len = strlen(lex->sw[i].phoneme_seq);
		for (idx = step = 0; idx < len; step++)
		{
			if (lex->sw[i].phoneme_seq[idx + step] == ' ' || lex->sw[i].phoneme_seq[idx + step] == '\0')
			{
				strncpy(tmp, &lex->sw[i].phoneme_seq[idx], step);
				tmp[step] = '\0';
				//printf("%s\n", tmp);
				for (j = 0; j < all_size; j++)
				{
					if (strcmp(all_phs[j], tmp) == 0)
					{
						break;
					}
				}
				if (j == all_size)
				{
					all_phs[all_size] = (char *)malloc((strlen(tmp) + 1) * sizeof(char));
					strcpy(all_phs[all_size++], tmp);
					if (all_size == all_capacity)
					{
						all_capacity += 1024;
						all_phs = (char **)realloc(all_phs, all_capacity * sizeof(char *));
					}
				}
				idx += step + 1;
				step = 0;
			}
		}
	}
	//for (i = 0; i < all_size; i++)
	//{
	//	printf("%d %s\n", i, all_phs[i]);
	//}

	chv_size = engv_size = 0;
	chv = (char **)malloc(all_capacity * sizeof(char *));
	engv = (char **)malloc(all_capacity * sizeof(char *));
	chv_possible_tone_size = (int *)malloc(all_capacity * sizeof(int));
	engv_possible_tone_size = (int *)malloc(all_capacity * sizeof(int));

	ch->ns.size = ch->nns.size = eng->ns.size = eng->nns.size = 0;
	for (i = 0; i < all_size; i++)
	{
		if (strcmp(all_phs[i], "SIL") == 0 || strcmp(all_phs[i], "SPN") == 0 || strcmp(all_phs[i], "NSN") == 0)
		{
			continue;
		}
		if (all_phs[i][0] >= 65 && all_phs[i][0] <= 90)
		{
			if (all_phs[i][strlen(all_phs[i])-1] >= 48 && all_phs[i][strlen(all_phs[i])-1] <= 57 && tone_flag == 1)
			{
				strcpy(tmp, all_phs[i]);
				tmp[strlen(tmp) - 1] = '\0';
				for (idx = 0; idx < engv_size; idx++)
				{
					if (strcmp(tmp, engv[idx]) == 0)
					{
						engv_possible_tone_size[idx]++;
						break;
					}
				}
				if (idx == engv_size)
				{
					engv_possible_tone_size[engv_size] = 1;
					engv[engv_size] = (char *)malloc((strlen(tmp) + 1) * sizeof(char));
					strcpy(engv[engv_size++], tmp);
				}
			}
			else
			{
				eng->nns.size++;
			}
		}
		else
		{
			if (all_phs[i][strlen(all_phs[i])-1] >= 48 && all_phs[i][strlen(all_phs[i])-1] <= 57 && tone_flag == 1)
			{
				strcpy(tmp, all_phs[i]);
				tmp[strlen(tmp) - 1] = '\0';
				for (idx = 0; idx < chv_size; idx++)
				{
					if (strcmp(tmp, chv[idx]) == 0)
					{
						chv_possible_tone_size[idx]++;
						break;
					}
				}
				if (idx == chv_size)
				{
					chv_possible_tone_size[chv_size] = 1;
					chv[chv_size] = (char *)malloc((strlen(tmp) + 1) * sizeof(char));
					strcpy(chv[chv_size++], tmp);
				}
			}
			else
			{
				ch->nns.size++;
			}
		}
	}

	//for (i = 0; i < engv_size; i++)
	//{
	//	printf("%s %d\n", engv[i], engv_possible_tone_size[i]);
	//}
	//for (i = 0; i < chv_size; i++)
	//{
	//	printf("%s %d\n", chv[i], chv_possible_tone_size[i]);
	//}

	ch->ns.size = chv_size;
	eng->ns.size = engv_size;
	//printf("%d %d %d %d\n", ch->ns.size, ch->nns.size, eng->ns.size, eng->nns.size);
	ch->ns.nuc = (NUCLUES *)malloc(ch->ns.size * sizeof(NUCLUES));
	ch->nns.name = (char **)malloc(ch->nns.size * sizeof(char*));
	eng->ns.nuc = (NUCLUES *)malloc(eng->ns.size * sizeof(NUCLUES));
	eng->nns.name = (char **)malloc(eng->nns.size * sizeof(char *));

	ch->nns.size = eng->nns.size = 0;

	for (i = 0; i < engv_size; i++)
	{
		eng->ns.nuc[i].tone_size = 0;
		eng->ns.nuc[i].possible_tone = (int *)malloc(engv_possible_tone_size[i] * sizeof(int));
		eng->ns.nuc[i].name = (char *)malloc((strlen(engv[i]) + 1) * sizeof(char));
		strcpy(eng->ns.nuc[i].name, engv[i]);
	}
	for (i = 0; i < chv_size; i++)
	{
		ch->ns.nuc[i].tone_size = 0;
		ch->ns.nuc[i].possible_tone = (int *)malloc(chv_possible_tone_size[i] * sizeof(int));
		ch->ns.nuc[i].name = (char *)malloc((strlen(chv[i]) + 1) * sizeof(char));
		strcpy(ch->ns.nuc[i].name, chv[i]);
	}

	for (i = 0; i < all_size; i++)
	{
		if (strcmp(all_phs[i], "SIL") == 0 || strcmp(all_phs[i], "SPN") == 0 || strcmp(all_phs[i], "NSN") == 0)
		{
			continue;
		}
		if (all_phs[i][0] >= 65 && all_phs[i][0] <= 90)
		{
			if (all_phs[i][strlen(all_phs[i]) - 1] >= 48 && all_phs[i][strlen(all_phs[i]) - 1] <= 57 && tone_flag == 1)
			{
				strcpy(tmp, all_phs[i]);
				tmp[strlen(tmp) - 1] = '\0';
				for (idx = 0; idx < engv_size; idx++)
				{
					if (strcmp(tmp, engv[idx]) == 0)
					{
						eng->ns.nuc[idx].possible_tone[eng->ns.nuc[idx].tone_size++] = (int)all_phs[i][strlen(all_phs[i]) - 1] - 48;
						break;
					}
				}
				if (idx == engv_size)
				{
					printf("error: unknow english phoneme... \"%s\"\n", tmp);
					exit(1);
				}
			}
			else
			{
				eng->nns.name[eng->nns.size] = (char *)malloc((strlen(all_phs[i]) + 1) * sizeof(char));
				strcpy(eng->nns.name[eng->nns.size++], all_phs[i]);
			}
		}
		else
		{
			if (all_phs[i][strlen(all_phs[i]) - 1] >= 48 && all_phs[i][strlen(all_phs[i]) - 1] <= 57 && tone_flag == 1)
			{
				strcpy(tmp, all_phs[i]);
				tmp[strlen(tmp) - 1] = '\0';
				for (idx = 0; idx < chv_size; idx++)
				{
					if (strcmp(tmp, chv[idx]) == 0)
					{
						ch->ns.nuc[idx].possible_tone[ch->ns.nuc[idx].tone_size++] = (int)all_phs[i][strlen(all_phs[i]) - 1] - 48;
						break;
					}
				}
				if (idx == chv_size)
				{
					printf("error: unknow chinese phoneme... \"%s\"\n", tmp);
					exit(1);
				}
			}
			else
			{
				ch->nns.name[ch->nns.size] = (char *)malloc((strlen(all_phs[i]) + 1) * sizeof(char));
				strcpy(ch->nns.name[ch->nns.size++], all_phs[i]);
			}
		}
	}
}
void output_text(char *s5_dir, char *type, MODIFY_TEXT *mt)
{
	FILE *fin, *fout;
	int capacity_of_line=0, FILEEND;
	char *line = (char *)malloc(sizeof(char));
	char *output_data_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	char *output_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	char *data_src_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	char *copy_src_path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	int i;

	strcpy(output_data_path, s5_dir);
	strcat(output_data_path, "data_");
	strcat(output_data_path, type);
	strcat(output_data_path, "/");

	strcpy(data_src_path, s5_dir);
	strcat(data_src_path, "data/");

	if (mt->text[0].exist_flag == 1)
	{
		strcpy(output_path, output_data_path);
		strcat(output_path, "test/text");
		if (!(fout = fopen(output_path, "w")))
		{
			printf("can not open file: %s\n", output_path);
			exit(1);
		}
		for (i = 0; i < mt->text[0].size; i++)
		{
			fprintf(fout, "%s\n", mt->text[0].mod[i]);
		}
		fclose(fout);
		strcpy(output_path, output_data_path);
		strcat(output_path, "test/");
		strcpy(copy_src_path, data_src_path);
		strcat(copy_src_path, "test/");
		copy_data(copy_src_path, output_path, "utt2spk");
		copy_data(copy_src_path, output_path, "wav.scp");
	}
	if (mt->text[1].exist_flag == 1)
	{
		strcpy(output_path, output_data_path);
		strcat(output_path, "dev/text");
		if (!(fout = fopen(output_path, "w")))
		{
			printf("can not open file: %s\n", output_path);
			exit(1);
		}
		for (i = 0; i < mt->text[1].size; i++)
		{
			fprintf(fout, "%s\n", mt->text[1].mod[i]);
		}
		fclose(fout);
		strcpy(output_path, output_data_path);
		strcat(output_path, "dev/");
		strcpy(copy_src_path, data_src_path);
		strcat(copy_src_path, "dev/");
		copy_data(copy_src_path, output_path, "utt2spk");
		copy_data(copy_src_path, output_path, "wav.scp");
	}
	if (mt->text[2].exist_flag == 1)
	{
		strcpy(output_path, output_data_path);
		strcat(output_path, "train/text");
		if (!(fout = fopen(output_path, "w")))
		{
			printf("can not open file: %s\n", output_path);
			exit(1);
		}
		for (i = 0; i < mt->text[2].size; i++)
		{
			fprintf(fout, "%s\n", mt->text[2].mod[i]);
		}
		fclose(fout);
		strcpy(output_path, output_data_path);
		strcat(output_path, "train/");
		strcpy(copy_src_path, data_src_path);
		strcat(copy_src_path, "train/");
		copy_data(copy_src_path, output_path, "utt2spk");
		copy_data(copy_src_path, output_path, "wav.scp");
	}
	

	free(line);
	free(output_data_path);
	free(output_path);
	free(copy_src_path);
	free(data_src_path);
}
void copy_data(char *src_dir, char *des_dir, char *name)
{
	char *input_path = (char *)malloc((strlen(src_dir) + strlen(name) + 1) * sizeof(char));
	char *output_path = (char *)malloc((strlen(des_dir) + strlen(name) + 1) * sizeof(char));
	FILE *fin, *fout;
	int capacity_of_line = 0, FILEEND;
	char *line = (char *)malloc(sizeof(char));

	strcpy(input_path, src_dir);
	strcat(input_path, name);
	strcpy(output_path, des_dir);
	strcat(output_path, name);

	if (!(fin = fopen(input_path, "r")))
	{
		printf("can not open file: %s\n", input_path);
		exit(1);
	}
	if (!(fout = fopen(output_path, "w")))
	{
		printf("can not open file: %s\n", output_path);
		exit(1);
	}
	for (FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fin);
		if (line[0] != '\0')
		{
			fprintf(fout, "%s\n", line);
		}
	}

	fclose(fin);
	fclose(fout);
	free(input_path);
	free(output_path);
	free(line);
}
void gen_dir(char *s5_dir, char *type, MODIFY_TEXT *mt)
{
	char *dir_name = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));
	char *data_name = (char *)malloc((strlen(s5_dir) + 32) * sizeof(char));
	char *check_dir = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));
	struct stat st = {0};

	strcpy(data_name, s5_dir);
	strcat(data_name, "data_");
	strcat(data_name, type);
	strcat(data_name, "/");
	strcpy(dir_name, data_name);
	if (stat(dir_name, &st) == -1)
	{
		if (stat(dir_name, &st) == -1)
		{
			mkdir(dir_name, 0755);
		}
	}
	strcpy(dir_name, data_name);
	strcat(dir_name, "train");
	if (stat(dir_name, &st) == -1 && mt->text[2].exist_flag == 1)
	{
		mkdir(dir_name, 0755);
	}
	strcpy(dir_name, data_name);
	strcat(dir_name, "dev");
	if (stat(dir_name, &st) == -1 && mt->text[1].exist_flag == 1)
	{
		if (stat(dir_name, &st) == -1)
		{
			mkdir(dir_name, 0755);
		}
	}
	strcpy(dir_name, data_name);
	strcat(dir_name, "test");
	if (stat(dir_name, &st) == -1 && mt->text[0].exist_flag == 1)
	{
		if (stat(dir_name, &st) == -1)
		{
			mkdir(dir_name, 0755);
		}
	}
	strcpy(dir_name, data_name);
	strcat(dir_name, "local/");
	if (stat(dir_name, &st) == -1)
	{
		if (stat(dir_name, &st) == -1)
		{
			mkdir(dir_name, 0755);
		}
	}
	strcpy(dir_name, data_name);
	strcat(dir_name, "local/dict");
	if (stat(dir_name, &st) == -1)
	{
		if (stat(dir_name, &st) == -1)
		{
			mkdir(dir_name, 0755);
		}
	}
	free(check_dir);
	free(dir_name);
	free(data_name);
}
void convet_dictionary(PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, LEXICON *raw_lex, LEXICON *new_lex, char *src_type, char *target_type, int src_tone_flag, int target_tone_flag, char *s5_dir)
{
	FILE *fnsp;
	NUCLUES_SET ns;
	PHONEME_SET ps;
	LEXICON temp_lex;
	LEXICON changed_lex;
	char *path = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));
	char word[7];
	int i, j, k, idx, flag;

	strcpy(path, s5_dir);
	strcat(path, "data/local/dict/nonsilence_phones.txt");

	if (!(fnsp = fopen(path, "r")))
	{
		printf("warning: can not open file: %s\n", path);
		exit(1);
	}
	else
	{
		change_lex_src_and_target(raw_lex, &changed_lex, pt, src_type, target_type);
		qsort(changed_lex.sw, changed_lex.num_word, sizeof(SINGLEWORD), lexiconWPcomparefun);
		//for (i = 0; i < changed_lex.num_word; i++)
		//{
		//	printf("%d\t%s %s\n", i, changed_lex.sw[i].word, changed_lex.sw[i].phoneme_seq);
		//}
		copy_dict(&changed_lex, new_lex, src_tone_flag);
		//for (i = 0; i < new_lex->num_word; i++)
		//{
		//	printf("%d\t%s %s\n", i, new_lex->sw[i].word, new_lex->sw[i].phoneme_seq);
		//}
		if (target_tone_flag == 0 && src_tone_flag == 1)
		{
			for (i = 0; i < new_lex->num_word; i++)
			{
				if (new_lex->sw[i].word[strlen(new_lex->sw[i].word) - 1] >= 48 && new_lex->sw[i].word[strlen(new_lex->sw[i].word) - 1] <= 57)
				{
					new_lex->sw[i].word[strlen(new_lex->sw[i].word) - 1] = '\0';
				}
			}
		}
//		load_phoneme_set(fnsp, &ps);
//
//		//for (i = 0; i < ps.ns.size; i++)
//		//{
//		//	printf("%s %d\n",ps.ns.nuc[i].name,ps.ns.nuc[i].tone_size);
//		//}
//		//for (i = 0; i < ps.nns.size; i++)
//		//{
//		//	printf("%s\n", ps.nns.name[i]);
//		//}
//		temp_lex.num_word = 0;
//		temp_lex.capacity = pt->size * 4 * 5;
//		temp_lex.sw = (SINGLEWORD *)malloc(temp_lex.capacity * sizeof(SINGLEWORD));
//		for (i = 0, idx = 0; i < raw_lex->num_word; i++, idx = 0)
//		{
//			read_a_utf8_word(raw_lex->sw[i].word, word, &idx);
//			if (isCJK_words(word) == 0)
//			{
//				temp_lex.sw[temp_lex.num_word].word = (char *)malloc((strlen(raw_lex->sw[i].word) + 1) * sizeof(char));
//				strcpy(temp_lex.sw[temp_lex.num_word].word, raw_lex->sw[i].word);
//				temp_lex.sw[temp_lex.num_word].phoneme_seq = (char *)malloc((strlen(raw_lex->sw[i].phoneme_seq) + 1) * sizeof(char));
//				strcpy(temp_lex.sw[temp_lex.num_word++].phoneme_seq, raw_lex->sw[i].phoneme_seq);
//			}
//		}
//
//		gen_dict_from_phonemetable(pt, ss, &temp_lex, &ps, target_type, tone_flag);
//		qsort(temp_lex.sw, temp_lex.num_word, sizeof(SINGLEWORD), lexiconWPcomparefun);
//		//for (i = 0; i < temp_lex.num_word; i++)
//		//{
//		//	printf("%s %s\n", temp_lex.sw[i].word, temp_lex.sw[i].phoneme_seq);
//		//}
//
//		new_lex->num_word = 0;
//		new_lex->capacity = temp_lex.capacity;
//		new_lex->sw = (SINGLEWORD *)malloc(new_lex->capacity * sizeof(SINGLEWORD));
//		for (i = flag = 0; i < temp_lex.num_word; i++, flag = 0)
//		{
//			if (i == 0)
//			{
//				flag = 1;
//			}
//			else if (strcmp(temp_lex.sw[i].word, temp_lex.sw[i - 1].word) != 0 || strcmp(temp_lex.sw[i].phoneme_seq, temp_lex.sw[i - 1].phoneme_seq) != 0)
//			{
//				flag = 1;
//			}
//			if (flag == 1)
//			{
//				new_lex->sw[new_lex->num_word].word = (char *)malloc((strlen(temp_lex.sw[i].word) + 1) * sizeof(char));
//				new_lex->sw[new_lex->num_word].phoneme_seq = (char *)malloc((strlen(temp_lex.sw[i].phoneme_seq) + 1) * sizeof(char));
//				strcpy(new_lex->sw[new_lex->num_word].word, temp_lex.sw[i].word);
//				for (j = k = 0; temp_lex.sw[i].phoneme_seq[j] != '\0'; j++)
//				{
//					if (temp_lex.sw[i].phoneme_seq[j] != '/' && temp_lex.sw[i].phoneme_seq[j + 1] != '/')
//					{
//						new_lex->sw[new_lex->num_word].phoneme_seq[k++] = temp_lex.sw[i].phoneme_seq[j];
//					}
//				}
//				new_lex->sw[new_lex->num_word].phoneme_seq[k] = '\0';
//				new_lex->num_word++;
//			}
//		}
//		//for (i = 0; i < new_lex->num_word; i++)
//		//{
//		//	printf("%s %s\n", new_lex->sw[i].word, new_lex->sw[i].phoneme_seq);
//		//}
//		fclose(fnsp);
	}

	free(path);
}
void copy_dict(LEXICON * raw_lex, LEXICON * mod_lex, int tone_flag)
{
	int i, idx, tmp;
	char *rm_tone_word = (char *)malloc(sizeof(char));
	char *rm_tone_phone = (char *)malloc(sizeof(char));
	char *temp = (char *)malloc(sizeof(char));

	mod_lex->capacity = raw_lex->num_word;
	mod_lex->num_word = 0;
	mod_lex->sw = (SINGLEWORD *)malloc(mod_lex->capacity * sizeof(SINGLEWORD));

	for (i = 0; i < raw_lex->num_word; i++)
	{
		if (tone_flag == 1)
		{
			mod_lex->sw[i].word = (char *)malloc((strlen(raw_lex->sw[i].word) + 1) * sizeof(char));
			strcpy(mod_lex->sw[i].word, raw_lex->sw[i].word);
			mod_lex->sw[i].phoneme_seq = (char *)malloc((strlen(raw_lex->sw[i].phoneme_seq) + 1) * sizeof(char));
			strcpy(mod_lex->sw[i].phoneme_seq, raw_lex->sw[i].phoneme_seq);
			mod_lex->num_word++;
		}
		else
		{
			rm_tone_word = (char *)realloc(rm_tone_word, (strlen(raw_lex->sw[i].word) + 1) * sizeof(char));
			strcpy(rm_tone_word, raw_lex->sw[i].word);
			if (rm_tone_word[strlen(rm_tone_word) - 1] >= 48 && rm_tone_word[strlen(rm_tone_word) - 1] <= 57)
			{
				rm_tone_word[strlen(rm_tone_word) - 1] = '\0';
			}
			for (idx = 0; idx < mod_lex->num_word; idx++)
			{
				if (strcmp(rm_tone_word, mod_lex->sw[idx].word) == 0)
				{
					break;
				}
			}
			if (idx == mod_lex->num_word)
			{
				mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(rm_tone_word) + 1) * sizeof(char));
				strcpy(mod_lex->sw[mod_lex->num_word].word, rm_tone_word);
				rm_tone_phone = (char *)realloc(rm_tone_phone, (strlen(raw_lex->sw[i].phoneme_seq) + 1) * sizeof(char));
				strcpy(rm_tone_phone, raw_lex->sw[i].phoneme_seq);
				for (tmp = strlen(rm_tone_phone) - 1; tmp > 0; tmp--)
				{
					if (rm_tone_phone[tmp] >= 48 && rm_tone_phone[tmp] <= 57)
					{
						temp = (char *)realloc(temp, (strlen(rm_tone_phone) + 1) * sizeof(char));
						strcpy(temp, &rm_tone_phone[tmp + 1]);
						strcpy(&rm_tone_phone[tmp], temp);
						for (tmp--; rm_tone_phone[tmp] != ' ' && tmp > 0; tmp--)
						{
							continue;
						}
					}
				}
				mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(rm_tone_phone) + 1) * sizeof(char));
				strcpy(mod_lex->sw[mod_lex->num_word].phoneme_seq, rm_tone_phone);
				mod_lex->num_word++;
			}
		}
	}

	free(rm_tone_word);
	free(rm_tone_phone);
	free(temp);
}
void change_lex_src_and_target(LEXICON *raw_lex, LEXICON *mod_lex, PRONUNCIATION_TABLE *pt, char *src_type, char *target_type)
{
	char word[7];
	int i, idx, temp, step;
	int mandarin_num;
	int maximum_len;
	char ***wp = (char ***)malloc(2 * sizeof(char **));
	char phmap[1024];
	char **appeared_syllable = (char **)malloc(pt->size * 6 * sizeof(char *));
	char tone[2] = {'\0', '\0'};
	int appeared_num = 0;
	LEXICON tmp_lex;

	tmp_lex.capacity = raw_lex->capacity;
	tmp_lex.num_word = raw_lex->num_word;
	tmp_lex.sw = (SINGLEWORD *)malloc(tmp_lex.num_word * sizeof(SINGLEWORD));

	for (i = maximum_len = 0; i < raw_lex->num_word; i++)
	{
		for (idx = temp = 0; read_a_utf8_word(raw_lex->sw[i].word, word, &idx) == 0;)
		{
			temp++;
		}
		if (temp > maximum_len)
		{
			maximum_len = temp;
		}
	}

	wp[0] = (char **)malloc(maximum_len * sizeof(char *));
	wp[1] = (char **)malloc(maximum_len * sizeof(char *));

	for (i = 0; i < maximum_len; i++)
	{
		wp[0][i] = (char *)malloc(sizeof(char));
		wp[1][i] = (char *)malloc(sizeof(char));
	}

	for (i = idx = 0; i < raw_lex->num_word; i++, idx = 0)
	{
		tmp_lex.sw[i].word = (char *)malloc((strlen(raw_lex->sw[i].word) + 1) * sizeof(char));
		strcpy(tmp_lex.sw[i].word, raw_lex->sw[i].word);
		read_a_utf8_word(tmp_lex.sw[i].word, word, &idx);
		if (isCJK_words(word) == 0)
		{
			tmp_lex.sw[i].phoneme_seq = (char *)malloc((strlen(raw_lex->sw[i].phoneme_seq) + 1) * sizeof(char));
			strcpy(tmp_lex.sw[i].phoneme_seq, raw_lex->sw[i].phoneme_seq);
			//printf("%s %s\n", tmp_lex.sw[i].word, tmp_lex.sw[i].phoneme_seq);
		}
		else
		{
			for (idx = mandarin_num = 0; read_a_utf8_word(tmp_lex.sw[i].word, word, &idx) == 0;)
			{
				mandarin_num++;
			}

			split_word(raw_lex->sw[i].word, raw_lex->sw[i].phoneme_seq, wp[0], wp[1]);
			//printf("%s %s\n", raw_lex->sw[i].word, raw_lex->sw[i].phoneme_seq);
			for (idx = 0; idx < mandarin_num; idx++)
			{
				//printf("\t%s %s %d\n", wp[0][idx], wp[1][idx], idx);
				phoneme2inifinorsyllable(wp[1][idx], phmap, pt, 2);
				for (temp = 0; temp < appeared_num; temp++)
				{
					if (strcmp(phmap, appeared_syllable[temp]) == 0)
					{
						break;
					}
				}
				if (temp == appeared_num)
				{
					appeared_syllable[appeared_num] = (char*)malloc((strlen(phmap) + 1) * sizeof(char));
					strcpy(appeared_syllable[appeared_num++], phmap);
				}
			}
			tmp_lex.sw[i].phoneme_seq = (char *)malloc((strlen(raw_lex->sw[i].phoneme_seq) + mandarin_num + 32) * sizeof(char));
			if (strcmp(src_type, "phoneme") == 0)
			{
				for (idx = 0, tmp_lex.sw[i].phoneme_seq[0] = '\0'; idx < mandarin_num; idx++)
				{
					strcat(tmp_lex.sw[i].phoneme_seq, wp[1][idx]);
					if (idx < mandarin_num - 1)
					{
						strcat(tmp_lex.sw[i].phoneme_seq, " ");
					}
				}
			}
			else
			{
				for (idx = 0, tmp_lex.sw[i].phoneme_seq[0] = '\0'; idx < mandarin_num; idx++)
				{
					if (strcmp(src_type, "inifin") == 0)
					{
						phoneme2inifinorsyllable(wp[1][idx], phmap, pt, 1);
					}
					else if (strcmp(src_type, "syllable") == 0)
					{
						phoneme2inifinorsyllable(wp[1][idx], phmap, pt, 2);
					}
					//printf("%s %s\n", wp[1][idx], phmap);
					strcat(tmp_lex.sw[i].phoneme_seq, phmap);
					if (idx < mandarin_num - 1)
					{
						strcat(tmp_lex.sw[i].phoneme_seq, " ");
					}
				}
			}
		}
		//printf("%s %s\n", tmp_lex.sw[i].word, tmp_lex.sw[i].phoneme_seq);
	}
	//for (i = 0; i < tmp_lex.num_word; i++)
	//{
	//	printf("%d %s %s\n", i + 1, tmp_lex.sw[i].word, tmp_lex.sw[i].phoneme_seq);
	//}

	if (strcmp(target_type, "word") == 0)
	{
		mod_lex->capacity = tmp_lex.capacity;
		mod_lex->num_word = tmp_lex.num_word;
		mod_lex->sw = (SINGLEWORD *)malloc(mod_lex->num_word * sizeof(SINGLEWORD));
		for (i = 0; i < tmp_lex.num_word; i++)
		{
			mod_lex->sw[i].word = (char *)malloc((strlen(tmp_lex.sw[i].word) + 1) * sizeof(char));
			strcpy(mod_lex->sw[i].word, tmp_lex.sw[i].word);
			mod_lex->sw[i].phoneme_seq = (char *)malloc((strlen(tmp_lex.sw[i].phoneme_seq) + 1) * sizeof(char));
			strcpy(mod_lex->sw[i].phoneme_seq, tmp_lex.sw[i].phoneme_seq);
		}
	}
	else
	{
		mod_lex->capacity = tmp_lex.capacity + appeared_num;
		mod_lex->num_word = 0;
		mod_lex->sw = (SINGLEWORD *)malloc(mod_lex->capacity * sizeof(SINGLEWORD));
		for (i = 0; i < tmp_lex.num_word; i++)
		{
			idx = 0;
			read_a_utf8_word(tmp_lex.sw[i].word, word, &idx);
			if (isCJK_words(word) == 0)
			{
				mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(tmp_lex.sw[i].word) + 1) * sizeof(char));
				strcpy(mod_lex->sw[mod_lex->num_word].word, tmp_lex.sw[i].word);
				mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(tmp_lex.sw[i].phoneme_seq) + 1) * sizeof(char));
				strcpy(mod_lex->sw[mod_lex->num_word++].phoneme_seq, tmp_lex.sw[i].phoneme_seq);
			}
		}
		for (i = 0; i < appeared_num; i++)
		{
			//printf("%d %s\n", i + 1, appeared_syllable[i]);
			tone[0] = appeared_syllable[i][strlen(appeared_syllable[i]) - 1];
			strcpy(phmap, appeared_syllable[i]);
			phmap[strlen(phmap) - 1] = '\0';
			for (idx = 0; idx < pt->size; idx++)
			{
				if (strcmp(phmap, pt->pinyin[idx]) == 0)
				{
					break;
				}
			}
			if (idx == pt->size)
			{
				printf("unknow syllable... \"%s\"\n",phmap);
				exit(1);
			}
			//printf("%d %s %d\n", i + 1, appeared_syllable[i], idx);
			if (strcmp(target_type, "syllable") == 0)
			{
				mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(appeared_syllable[i]) + 1) * sizeof(char));
				strcpy(mod_lex->sw[mod_lex->num_word].word, appeared_syllable[i]);
				if (strcmp(src_type, "syllable") == 0)
				{
					mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(appeared_syllable[i]) + 1) * sizeof(char));
					strcpy(mod_lex->sw[mod_lex->num_word++].phoneme_seq, appeared_syllable[i]);
				}
				else if (strcmp(src_type, "inifin") == 0)
				{
					mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->initial[idx]) + strlen(pt->final[idx]) + 3) * sizeof(char));
					if (pt->initial[idx][0] != '/')
					{
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->initial[idx]);
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, " ");
					}
					strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->final[idx]);
					strcat(mod_lex->sw[mod_lex->num_word++].phoneme_seq, tone);
				}
				else
				{
					mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->initial[idx]) + strlen(pt->medials[idx]) + strlen(pt->nucleus[idx]) + strlen(pt->coda[idx]) + 5) * sizeof(char));
					mod_lex->sw[mod_lex->num_word].phoneme_seq[0] = '\0';
					if (pt->initial[idx][0] != '/')
					{
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->initial[idx]);
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, " ");
					}
					if (pt->medials[idx][0] != '/')
					{
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->medials[idx]);
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, " ");
					}
					strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->nucleus[idx]);
					strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, tone);
					if (pt->coda[idx][0] != '/')
					{
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, " ");
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->coda[idx]);
					}
					mod_lex->num_word++;
				}
			}
			else if (strcmp(target_type, "inifin") == 0)
			{
				if (pt->initial[idx][0] != '/')
				{
					for (temp = 0; temp < mod_lex->num_word; temp++)
					{
						if (strcmp(mod_lex->sw[temp].word, pt->initial[idx]) == 0)
						{
							break;
						}
					}
					if (temp == mod_lex->num_word)
					{
						mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(pt->initial[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word].word, pt->initial[idx]);
						mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->initial[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word++].phoneme_seq, pt->initial[idx]);
					}
				}
				for (temp = 0; temp < mod_lex->num_word; temp++)
				{
					if (strcmp(mod_lex->sw[temp].word, pt->final[idx]) == 0)
					{
						break;
					}
				}
				if (temp == mod_lex->num_word)
				{
					mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(pt->final[idx]) + 2) * sizeof(char));
					strcpy(mod_lex->sw[mod_lex->num_word].word, pt->final[idx]);
					strcat(mod_lex->sw[mod_lex->num_word].word, tone);
					if (strcmp(src_type, "inifin") == 0)
					{
						mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->final[idx]) + 2) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->final[idx]);
						strcat(mod_lex->sw[mod_lex->num_word++].phoneme_seq, tone);
					}
					else
					{
						mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->medials[idx]) + strlen(pt->nucleus[idx]) + strlen(pt->coda[idx]) + 4) * sizeof(char));
						mod_lex->sw[mod_lex->num_word].phoneme_seq[0] = '\0';
						if (pt->medials[idx][0] != '/')
						{
							strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->medials[idx]);
							strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, " ");
						}
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->nucleus[idx]);
						strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, tone);
						if (pt->coda[idx][0] != '/')
						{
							strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, " ");
							strcat(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->coda[idx]);
						}
						mod_lex->num_word++;
					}
				}
			}
			else
			{
				if (pt->initial[idx][0] != '/')
				{
					for (temp = 0; temp < mod_lex->num_word; temp++)
					{
						if (strcmp(mod_lex->sw[temp].word, pt->initial[idx]) == 0)
						{
							break;
						}
					}
					if (temp == mod_lex->num_word)
					{
						mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(pt->initial[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word].word, pt->initial[idx]);
						mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->initial[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word++].phoneme_seq, pt->initial[idx]);
					}
				}
				if (pt->medials[idx][0] != '/')
				{
					for (temp = 0; temp < mod_lex->num_word; temp++)
					{
						if (strcmp(mod_lex->sw[temp].word, pt->medials[idx]) == 0)
						{
							break;
						}
					}
					if (temp == mod_lex->num_word)
					{
						mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(pt->medials[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word].word, pt->medials[idx]);
						mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->medials[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word++].phoneme_seq, pt->medials[idx]);
					}
				}
				for (temp = 0; temp < mod_lex->num_word; temp++)
				{
					if (strcmp(mod_lex->sw[temp].word, pt->nucleus[idx]) == 0)
					{
						break;
					}
				}
				if (temp == mod_lex->num_word)
				{
					mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(pt->nucleus[idx]) + 2) * sizeof(char));
					strcpy(mod_lex->sw[mod_lex->num_word].word, pt->nucleus[idx]);
					strcat(mod_lex->sw[mod_lex->num_word].word, tone);
					mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->nucleus[idx]) + 2) * sizeof(char));
					strcpy(mod_lex->sw[mod_lex->num_word].phoneme_seq, pt->nucleus[idx]);
					strcat(mod_lex->sw[mod_lex->num_word++].phoneme_seq, tone);
				}
				if (pt->coda[idx][0] != '/')
				{
					for (temp = 0; temp < mod_lex->num_word; temp++)
					{
						if (strcmp(mod_lex->sw[temp].word, pt->coda[idx]) == 0)
						{
							break;
						}
					}
					if (temp == mod_lex->num_word)
					{
						mod_lex->sw[mod_lex->num_word].word = (char *)malloc((strlen(pt->coda[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word].word, pt->coda[idx]);
						mod_lex->sw[mod_lex->num_word].phoneme_seq = (char *)malloc((strlen(pt->coda[idx]) + 1) * sizeof(char));
						strcpy(mod_lex->sw[mod_lex->num_word++].phoneme_seq, pt->coda[idx]);
					}
				}
			}
			//printf("%d %s %s\n", mod_lex->num_word, mod_lex->sw[mod_lex->num_word - 1].word, mod_lex->sw[mod_lex->num_word - 1].phoneme_seq);
			
		}
	}
}
void phoneme2inifinorsyllable(char *input, char *output, PRONUNCIATION_TABLE *pt, int iors)
{
	char phmap[4][64] = {"/", "/", "/", "/"};
	char tone;
	int i, idx, step;

	for (idx = 0; input[idx] != '\0';)
	{
		if (strncmp(&input[idx], "ini_", 4) == 0 || strncmp(&input[idx], "m_", 2) == 0 || strncmp(&input[idx], "n_", 2) == 0 || strncmp(&input[idx], "c_", 2) == 0)
		{
			for (step = 0; input[idx + step] != ' ' && input[idx + step] != '\0'; step++)
			{
				//printf("%d %c %d\n", step, input[idx + step], (int)input[idx + step]);
				continue;
			}

			if (strncmp(&input[idx], "ini_", 4) == 0)
			{
				strncpy(phmap[0], &input[idx], step);
				phmap[0][step] = '\0';
			}
			else if (strncmp(&input[idx], "m_", 2) == 0)
			{
				strncpy(phmap[1], &input[idx], step);
				phmap[1][step] = '\0';
			}
			else if (strncmp(&input[idx], "n_", 2) == 0)
			{
				strncpy(phmap[2], &input[idx], step);
				phmap[2][step] = '\0';
			}
			else if (strncmp(&input[idx], "c_", 2) == 0)
			{
				strncpy(phmap[3], &input[idx], step);
				phmap[3][step] = '\0';
			}
			

			idx += step;
		}
		else
		{
			idx++;
		}
	}
	tone = phmap[2][strlen(phmap[2]) - 1];
	phmap[2][strlen(phmap[2]) - 1] = '\0';
	for (i = 0; i < pt->size; i++)
	{
		if (strcmp(pt->initial[i], phmap[0]) == 0 && 
			strcmp(pt->medials[i], phmap[1]) == 0 && 
			strcmp(pt->nucleus[i], phmap[2]) == 0 && 
			strcmp(pt->coda[i], phmap[3]) == 0)
		{
			break;
		}
	}
	//printf("%d\n", i);
	if (i == pt->size)
	{
		printf("error: unknow phoneme sequence...\"%s\"\n%s %s %s %s %c\n", input, phmap[0], phmap[1], phmap[2], phmap[3], tone);

		exit(1);
	}
	output[0] = '\0';
	if (iors == 1)
	{
		if (pt->initial[i][0] != '/')
		{
			strcat(output, pt->initial[i]);
			strcat(output, " ");
		}
		strcat(output, pt->final[i]);
	}
	else
	{
		strcat(output, pt->pinyin[i]);
	}
	output[strlen(output) + 1] = '\0';
	output[strlen(output)] = tone;
}
void split_word(char *word, char *phs, char **w0, char **w1)
{
	char temp[7];
	int i, idx, ph_idx, ph_step;

	for (i = idx = ph_idx = 0; read_a_utf8_word(word, temp, &idx) == 0; i++)
	{
		w0[i] = (char *)malloc((strlen(temp) + 1) * sizeof(char));
		strcpy(w0[i], temp);

		for (ph_step = 0; phs[ph_idx + ph_step] != '\0'; ph_step++)
		{
			if (strncmp(&phs[ph_idx + ph_step], "n_", 2) == 0)
			{
				for (; phs[ph_idx + ph_step] != '\0' && phs[ph_idx + ph_step] != ' '; ph_step++)
				{
					continue;
				}
				if (phs[ph_idx + ph_step] == ' ')
				{
					if (strncmp(&phs[ph_idx + ph_step + 1], "c_", 2) == 0)
					{
						for (ph_step++; phs[ph_idx + ph_step] != '\0' && phs[ph_idx + ph_step] != ' '; ph_step++)
						{
							continue;
						}
					}
				}
				w1[i] = (char *)malloc((ph_step + 1) * sizeof(char));
				strncpy(w1[i], &phs[ph_idx], ph_step);
				w1[i][ph_step] = '\0';
				ph_idx += ph_step + 1;
				break;
			}
		}
	}
}
void gen_dict_from_phonemetable(PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, LEXICON *lex, PHONEME_SET *ps, char *type, int tone_flag)
{
	int i, j, idx;

	if (strcmp(type, "syllable") == 0)
	{
		for (i = 0; i < ss->size; i++)
		{
			for (idx = 0; idx < pt->size; idx++)
			{
				if (strcmp(ss->st[i].name, pt->pinyin[idx]) == 0)
				{
					break;
				}
			}
			if (idx == ss->size)
			{
				printf("warning: can't find %s in phoneme_table.\n", ss->st[i].name);
			}
			else
			{
				lex->sw[lex->num_word].word = (char *)malloc((strlen(pt->pinyin[idx]) + 2) * sizeof(char));
				lex->sw[lex->num_word].phoneme_seq = (char *)malloc(128 * sizeof(char));
				strcpy(lex->sw[lex->num_word].word, pt->pinyin[idx]);
				if (tone_flag != 0)
				{
					lex->sw[lex->num_word].word[strlen(lex->sw[lex->num_word].word) + 1] = '\0';
					lex->sw[lex->num_word].word[strlen(lex->sw[lex->num_word].word)] = ss->st[i].tone;
				}
				sprintf(lex->sw[lex->num_word].phoneme_seq, "%s %s %s%c %s", pt->initial[idx], pt->medials[idx], pt->nucleus[idx], ss->st[i].tone, pt->coda[idx]);
				lex->num_word++;
			}
		}
	}
	if (strcmp(type, "inifin") == 0)
	{
		for (i = 0; i < ps->nns.size; i++)
		{
			if (strncmp(ps->nns.name[i], "ini_", 4) == 0)
			{
				lex->sw[lex->num_word].word = (char *)malloc((strlen(ps->nns.name[i]) + 1) * sizeof(char));
				lex->sw[lex->num_word].phoneme_seq = (char *)malloc((strlen(ps->nns.name[i]) + 1) * sizeof(char));
				strcpy(lex->sw[lex->num_word].word, ps->nns.name[i]);
				strcpy(lex->sw[lex->num_word].phoneme_seq, ps->nns.name[i]);
				lex->num_word++;
			}
		}
		for (i = 0; i < ss->size; i++)
		{
			for (idx = 0; idx < pt->size; idx++)
			{
				if (strcmp(ss->st[i].name, pt->pinyin[idx]) == 0)
				{
					break;
				}
			}
			if (idx == ss->size)
			{
				printf("warning: can't find %s in phoneme_table.\n", ss->st[i].name);
			}
			else
			{
				lex->sw[lex->num_word].word = (char *)malloc((strlen(pt->final[idx]) + 2) * sizeof(char));
				lex->sw[lex->num_word].phoneme_seq = (char *)malloc(128 * sizeof(char));
				strcpy(lex->sw[lex->num_word].word, pt->final[idx]);
				if (tone_flag != 0)
				{
					lex->sw[lex->num_word].word[strlen(lex->sw[lex->num_word].word) + 1] = '\0';
					lex->sw[lex->num_word].word[strlen(lex->sw[lex->num_word].word)] = ss->st[i].tone;
				}
				sprintf(lex->sw[lex->num_word].phoneme_seq, "%s %s%c %s", pt->medials[idx], pt->nucleus[idx], ss->st[i].tone, pt->coda[idx]);
				lex->num_word++;
			}
		}
	}
	if (strcmp(type, "phoneme") == 0)
	{
		for (i = 0; i < ps->nns.size; i++)
		{
			lex->sw[lex->num_word].word = (char *)malloc((strlen(ps->nns.name[i]) + 1) * sizeof(char));
			lex->sw[lex->num_word].phoneme_seq = (char *)malloc((strlen(ps->nns.name[i]) + 1) * sizeof(char));
			strcpy(lex->sw[lex->num_word].word, ps->nns.name[i]);
			strcpy(lex->sw[lex->num_word].phoneme_seq, ps->nns.name[i]);
			lex->num_word++;
		}
		for (i = 0; i < ps->ns.size; i++)
		{
			for (j = 0; j < ps->ns.nuc[i].tone_size; j++)
			{
				lex->sw[lex->num_word].word = (char *)malloc((strlen(ps->ns.nuc[i].name) + 2) * sizeof(char));
				lex->sw[lex->num_word].phoneme_seq = (char *)malloc((strlen(ps->ns.nuc[i].name) + 2) * sizeof(char));
				if (tone_flag != 0)
				{
					sprintf(lex->sw[lex->num_word].word, "%s%d", ps->ns.nuc[i].name, ps->ns.nuc[i].possible_tone[j]);
				}
				else
				{
					strcpy(lex->sw[lex->num_word].word, ps->ns.nuc[i].name);
				}
				sprintf(lex->sw[lex->num_word].phoneme_seq, "%s%d", ps->ns.nuc[i].name, ps->ns.nuc[i].possible_tone[j]);
				lex->num_word++;
			}
		}
	}
}
void load_phoneme_set(FILE *fnsp, PHONEME_SET *ps)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int nuc_i, nns_i, j, idx, step;

	for (FILEEND = 0, ps->ns.size = ps->nns.size = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fnsp);
		if (strncmp(line, "n_", 2) == 0)
		{
			ps->ns.size++;
		}
		if (strncmp(line, "ini_", 4) == 0 || strncmp(line, "m_", 2) == 0 || strncmp(line, "c_", 2) == 0)
		{
			ps->nns.size++;
		}
	}
	fseek(fnsp, 0, SEEK_SET);
	ps->ns.nuc = (NUCLUES *)malloc(ps->ns.size * sizeof(NUCLUES));
	ps->nns.name = (char **)malloc(ps->nns.size * sizeof(char *));
	//printf("ps->ns.size = %d\n", ps->ns.size);
	for (FILEEND = 0, nuc_i = nns_i = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fnsp);
		if (strncmp(line, "n_", 2) == 0)
		{
			for (idx = 0, ps->ns.nuc[nuc_i].tone_size = 0; line[idx] != '\0'; idx++)
			{
				if (line[idx] == ' ')
				{
					ps->ns.nuc[nuc_i].tone_size++;
				}
			}
			ps->ns.nuc[nuc_i].possible_tone = (int *)malloc(ps->ns.nuc[nuc_i].tone_size * sizeof(int));
			for (idx = step = 0;; step++)
			{
				if (line[idx + step] == ' ')
				{
					ps->ns.nuc[nuc_i].name = (char *)malloc((step + 1) * sizeof(char));
					strncpy(ps->ns.nuc[nuc_i].name, line, step);
					ps->ns.nuc[nuc_i].name[step] = '\0';
					idx += step + 1;
					break;
				}
			}
			for (step = 0, j = 0;; step++)
			{
				if (line[idx + step] == ' ' || line[idx + step] == '\0')
				{
					ps->ns.nuc[nuc_i].possible_tone[j++] = (int)line[idx + step - 1] - 48;
					if (line[idx + step] == '\0')
					{
						break;
					}
				}
			}
			//printf("%s\t", ps->ns.nuc[i].name);
			//for (j = 0; j < ps->ns.nuc[i].tone_size; j++)
			//{
			//	printf("%d ", ps->ns.nuc[i].possible_tone[j]);
			//}
			//printf("\n");
			nuc_i++;
		}
		if (strncmp(line, "ini_", 4) == 0 || strncmp(line, "m_", 2) == 0 || strncmp(line, "c_", 2) == 0)
		{
			ps->nns.name[nns_i] = (char *)malloc((strlen(line) + 1) * sizeof(char));
			strcpy(ps->nns.name[nns_i], line);
			nns_i++;
		}
	}
	free(line);
}
void convert_text(MODIFY_TEXT *mt, LENDICT *ld, PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, char *type, int tone_flag, char *s5_dir)
{
	FILE *ftxt;
	char *path = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));
	char set_path[3][32] = {"data/test/text", "data/dev/text", "data/train/text"};
	int i;

	ss->capacity = pt->size * 5;
	ss->st = (SYLLABLE_TOKEN *)malloc(ss->capacity * sizeof(SYLLABLE_TOKEN));
	ss->size = 0;
	for (i = 0; i < 3; i++)
	{
		strcpy(path, s5_dir);
		strcat(path, set_path[i]);
		if (!(ftxt = fopen(path, "r")))
		{
			printf("warning: can not open file: %s\n", path);
			mt->text[i].exist_flag = 0;
		}
		else
		{
			mt->text[i].exist_flag = 1;
			mt->text[i].size = load_raw_text(&(mt->text[i].raw), ftxt);
			transform_raw_text(&(mt->text[i]), ld, pt, ss, type, tone_flag);
			fclose(ftxt);
		}
	}
	qsort(ss->st, ss->size, sizeof(SYLLABLE_TOKEN), syllablecomparefun);
	//for (i = 0; i < ss->size; i++)
	//{
	//	printf("%s%c\n", ss->st[i].name, ss->st[i].tone);
	//}

	free(path);
}
void transform_raw_text(TEXT_PAIR *tp, LENDICT *ld, PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, char *type, int tone_flag)
{
	int i, idx, w_idx, len;
	char word[7], *token = (char *)malloc(sizeof(char));

	tp->mod = (char **)malloc(tp->size * sizeof(char *));
	for (i = 0; i < tp->size; i++)
	{
		tp->mod[i] = (char *)malloc(strlen(tp->raw[i]) * 8 * sizeof(char));
		token = (char *)realloc(token, (strlen(tp->raw[i]) + 1) * sizeof(char));
		for (idx = 0, tp->mod[i][0] = '\0'; read_a_utf8_word(tp->raw[i], word, &idx) == 0;)
		{
			if (word[0] == ' ')
			{
				break;
			}
			strcat(tp->mod[i], word);
		}
		if (word[0] != '\0')
		{
			for (;;)
			{
				for (token[0] = '\0'; read_a_utf8_word(tp->raw[i], word, &idx) == 0;)
				{
					if (word[0] == ' ')
					{
						break;
					}
					strcat(token, word);
				}
				//printf("%s\t", token);
				w_idx = 0;
				read_a_utf8_word(token, word, &w_idx);
				if (isCJK_words(word) == 1)
				{
					//strcat(tp->mod[i], " ");
					if (strcmp(type, "word") == 0)
					{
						strcat(tp->mod[i], " ");
						strcat(tp->mod[i], token);
					}
					else
					{
						word2sylorph(token, tp->mod[i], ld, pt, ss, type, tone_flag);
					}
				}
				else
				{
					strcat(tp->mod[i], " ");
					strcat(tp->mod[i], token);
				}
				if (word[0] == '\0')
				{
					break;
				}
			}
		}
		//printf("%s\n%s\n", tp->raw[i], tp->mod[i]);
	}

	free(token);
}
void word2sylorph(char *word, char *output, LENDICT *ld, PRONUNCIATION_TABLE *pt, SYLLABLE_SET *ss, char *type, int tone_flag)
{
	int i, j, idx, p_idx, p_step;
	char *phseq, *phw, s_idx, step;
	char ph[4][16], tone;

	idx = bin_lexicon_search_len_match(word, &(ld->lexs[strlen(word) - 1]));
	if (idx == -1)
	{
		printf("error: can't find %s in dictionary\n", word);
		exit(1);
	}
	else
	{
		//printf("%sraw------------------->%s\n", word, ld->lexs[strlen(word) - 1].sw[idx].phoneme_seq);
		fflush(stdout);
		phseq = (char *)malloc((strlen(ld->lexs[strlen(word) - 1].sw[idx].phoneme_seq) + 1) * sizeof(char));
		phw = (char *)malloc((strlen(ld->lexs[strlen(word) - 1].sw[idx].phoneme_seq) + 1) * sizeof(char));
		strcpy(phseq, ld->lexs[strlen(word) - 1].sw[idx].phoneme_seq);
		for (s_idx = step = 0, phw[0] = '\0'; phseq[s_idx] != '\0'; step++)
		{
			if (strncmp(&phseq[s_idx + step], "n_", 2) == 0)
			{
				for (step += 1;; step++)
				{
					if (phseq[s_idx + step] == '\0')
					{
						break;
					}
					else if (strncmp(&phseq[s_idx + step], "ini_", 4) == 0 || strncmp(&phseq[s_idx + step], "m_", 2) == 0 || strncmp(&phseq[s_idx + step], "n_", 2) == 0)
					{
						step -= 1;
						break;
					}
				}
				strncpy(phw, &phseq[s_idx], step);
				phw[step] = '\0';
				//printf(">>>>>>>>>>>>%s\n", phw);
				ph[0][0] = ph[1][0] = ph[2][0] = ph[3][0] = '/';
				ph[0][1] = ph[1][1] = ph[2][1] = ph[3][1] = '\0';
				for (p_idx = 0; phw[p_idx] != '\0';)
				{
					for (p_step = 1; phw[p_idx + p_step] != '\0' && phw[p_idx + p_step] != ' '; p_step++)
					{
						continue;
					}
					//printf("%s\n", &phw[p_idx]);
					if (strncmp(&phw[p_idx], "ini_", 4) == 0)
					{
						strncpy(ph[0], &phw[p_idx], p_step);
						ph[0][p_step] = '\0';
					}
					else if (strncmp(&phw[p_idx], "m_", 2) == 0)
					{
						strncpy(ph[1], &phw[p_idx], p_step);
						ph[1][p_step] = '\0';
					}
					else if (strncmp(&phw[p_idx], "n_", 2) == 0)
					{
						strncpy(ph[2], &phw[p_idx], p_step);
						ph[2][p_step] = '\0';
					}
					else if (strncmp(&phw[p_idx], "c_", 2) == 0)
					{
						strncpy(ph[3], &phw[p_idx], p_step);
						ph[3][p_step] = '\0';
					}
					if (phw[p_idx + p_step] == ' ')
					{
						p_idx += p_step + 1;
					}
					else
					{
						p_idx += p_step;
					}
				}
				//printf("%s %s %s %s\n", ph[0], ph[1], ph[2], ph[3]);
				tone = ph[2][strlen(ph[2]) - 1];
				for (i = 0, ph[2][strlen(ph[2]) - 1] = '\0'; i < pt->size; i++)
				{
					if (strcmp(ph[0], pt->initial[i]) == 0 && strcmp(ph[1], pt->medials[i]) == 0 && strcmp(ph[2], pt->nucleus[i]) == 0 && strcmp(ph[3], pt->coda[i]) == 0)
					{
						//printf("%s\n", pt->pinyin[i]);
						break;
					}
				}
				if (i == pt->size)
				{
					printf("error: can't find %s %s %s %s in phoneme table\n", ph[0], ph[1], ph[2], ph[3]);
					exit(1);
				}
				else
				{
					if (strcmp(type, "syllable") == 0)
					{
						strcat(output, " ");
						strcat(output, pt->pinyin[i]);
						if (tone_flag != 0)
						{
							output[strlen(output) + 1] = '\0';
							output[strlen(output)] = tone;
						}
					}
					else if (strcmp(type, "inifin") == 0)
					{
						if (pt->initial[i][0] != '/')
						{
							strcat(output, " ");
							strcat(output, pt->initial[i]);
						}
						strcat(output, " ");
						strcat(output, pt->final[i]);
						if (tone_flag != 0)
						{
							output[strlen(output) + 1] = '\0';
							output[strlen(output)] = tone;
						}
					}
					else if (strcmp(type, "phoneme") == 0)
					{
						if (pt->initial[i][0] != '/')
						{
							strcat(output, " ");
							strcat(output, pt->initial[i]);
						}
						if (pt->medials[i][0] != '/')
						{
							strcat(output, " ");
							strcat(output, pt->medials[i]);
						}
						strcat(output, " ");
						strcat(output, pt->nucleus[i]);
						if (tone_flag != 0)
						{
							output[strlen(output) + 1] = '\0';
							output[strlen(output)] = tone;
						}
						if (pt->coda[i][0] != '/')
						{
							strcat(output, " ");
							strcat(output, pt->coda[i]);
						}
					}
					for (j = 0; j < ss->size; j++)
					{
						if (strcmp(ss->st[j].name, pt->pinyin[i]) == 0 && ss->st[j].tone == tone)
						{
							break;
						}
					}
					if (j == ss->size)
					{
						ss->st[ss->size].name = (char *)malloc((strlen(pt->pinyin[i]) + 1) * sizeof(char));
						strcpy(ss->st[ss->size].name, pt->pinyin[i]);
						ss->st[ss->size].tone = tone;
						ss->size++;
					}
				}
				if (phseq[s_idx + step] == '\0')
				{
					break;
				}
				else
				{
					s_idx += step + 1;
				}
				step = -1;
			}
		}

		free(phseq);
		free(phw);
	}
}
int load_raw_text(char ***lines, FILE *ftxt)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int idx, i, row;

	for (FILEEND = 0, row = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, ftxt);
		if (line[0] != '\0')
		{
			row++;
		}
	}

	(*lines) = (char **)malloc(row * sizeof(char *));
	fseek(ftxt, 0, SEEK_SET);
	for (FILEEND = 0, idx = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, ftxt);
		if (line[0] != '\0')
		{
			(*lines)[idx] = (char *)malloc((strlen(line) + 1) * sizeof(char));
			strcpy((*lines)[idx++], line);
		}
	}

	free(line);
	return row;
}
void build_lexicon(char *s5_dir, LEXICON *lex, LENDICT *ld)
{
	int i;
	char *dict_dir = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));
	FILE *fdic;

	strcpy(dict_dir, s5_dir);
	strcat(dict_dir, "data/local/dict/lexicon.txt");
	//printf("%s\n", dict_dir);
	if (!(fdic = fopen(dict_dir, "r")))
	{
		printf("can not open file: %s\n", dict_dir);
		exit(1);
	}

	load_lexicon(fdic, lex);
	qsort(lex->sw, lex->num_word, sizeof(SINGLEWORD), lexilenconcomparefun);
	split_dictionary(ld, lex);
	for (i = 0; i < ld->maximum_len; i++)
	{
		qsort(ld->lexs[i].sw, ld->lexs[i].num_word, sizeof(SINGLEWORD), lexiconcomparefun);
	}
	//for (i = 0; i < lex->num_word; i++)
	//{
	//	printf("%s %s %.31lf\n", lex->sw[i].word, lex->sw[i].phoneme_seq, lex->sw[i].prob_outcome);
	//}

	free(dict_dir);
}
void split_dictionary(LENDICT *ld, LEXICON *lex)
{
	int i, j, len, idx;

	ld->maximum_len = 0;
	ld->total_lexicon_outcome = 0;
	for (i = 0; i < lex->num_word; i++)
	{
		if (strlen(lex->sw[i].word) > ld->maximum_len)
		{
			ld->maximum_len = strlen(lex->sw[i].word);
		}
		ld->total_lexicon_outcome += lex->sw[i].outcome_times;
	}
	//printf("maximum_len=%d\n", ld->maximum_len);
	ld->lexs = (LEXICON *)malloc(ld->maximum_len * sizeof(LEXICON));

	for (i = 0; i < ld->maximum_len; i++)
	{
		ld->lexs[i].num_word = 0;
		ld->lexs[i].capacity = 0;
	}
	for (i = 0; i < lex->num_word; i++)
	{
		ld->lexs[strlen(lex->sw[i].word) - 1].capacity++;
	}
	for (i = idx = 0; i < ld->maximum_len; i++)
	{
		//printf("len=%d,num=%d\n", i + 1, ld->lexs[i].capacity);
		//ld->lexs[i].sw = (SINGLEWORD *)malloc(ld->lexs[i].capacity * sizeof(SINGLEWORD));
		ld->lexs[i].sw = &(lex->sw[idx]);
		ld->lexs[i].num_word = ld->lexs[i].capacity;
		idx += ld->lexs[i].capacity;
	}

	for (i = 0; i < ld->maximum_len; i++)
	{
		for (j = 0; j < ld->lexs[i].num_word; j++)
		{
			if (ld->lexs[i].sw[j].outcome_times > 0)
			{
				ld->lexs[i].sw[j].prob_outcome = log((double)ld->lexs[i].sw[j].outcome_times / (double)ld->total_lexicon_outcome);
			}
			else
			{
				ld->lexs[i].sw[j].prob_outcome = -99;
			}
		}
	}
}
void load_lexicon(FILE *fdict, LEXICON *lex)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int idx, i, row;

	for (FILEEND = 0, lex->num_word = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fdict);
		if (line[0] != '\0')
		{
			lex->num_word++;
		}
	}
	//printf("lex->num_word=%d\n", lex->num_word);
	lex->capacity = lex->num_word;
	lex->sw = (SINGLEWORD *)malloc(lex->num_word * sizeof(SINGLEWORD));
	fseek(fdict, 0, SEEK_SET);
	for (FILEEND = 0, row = 0; FILEEND == 0; row++)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fdict);
		if (line[0] != '\0')
		{
			for (idx = 0; line[idx] != '\t' && line[idx] != ' '; idx++)
				;
			lex->sw[row].word = (char *)malloc((idx + 1) * sizeof(char));
			for (i = idx = 0; line[idx] != '\t' && line[idx] != ' '; idx++)
			{
				lex->sw[row].word[i++] = line[idx];
			}
			lex->sw[row].word[i] = '\0';
			lex->sw[row].phoneme_seq = (char *)malloc((strlen(&line[idx + 1]) + 1) * sizeof(char));
			strcpy(lex->sw[row].phoneme_seq, &line[idx + 1]);
			lex->sw[row].outcome_times = 0;
		}
	}
	qsort(lex->sw, lex->num_word, sizeof(SINGLEWORD), lexiconcomparefun);
	//for (i = 0; i < lex->num_word; i++)
	//{
	//	printf("%s %s\n", lex->sw[i].word, lex->sw[i].phoneme_seq);
	//}

	free(line);
}
void load_pronunciation_table(PRONUNCIATION_TABLE *pt, FILE *fpt)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int i, start_idx, end_idx;

	for (FILEEND = 0, pt->size = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fpt);
		if (line[0] != '\0')
		{
			pt->size++;
		}
	}
	pt->size -= 1;
	pt->coda = (char **)malloc(pt->size * sizeof(char *));
	pt->final = (char **)malloc(pt->size * sizeof(char *));
	pt->initial = (char **)malloc(pt->size * sizeof(char *));
	pt->medials = (char **)malloc(pt->size * sizeof(char *));
	pt->nucleus = (char **)malloc(pt->size * sizeof(char *));
	pt->phcode = (char **)malloc(pt->size * sizeof(char *));
	pt->phonetic = (char **)malloc(pt->size * sizeof(char *));
	pt->pinyin = (char **)malloc(pt->size * sizeof(char *));
	pt->rime = (char **)malloc(pt->size * sizeof(char *));

	fseek(fpt, 0, SEEK_SET);
	FILEEND = read_a_line(&line, &capacity_of_line, fpt);
	for (i = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fpt);
		if (line[0] != '\0')
		{
			for (start_idx = 0, end_idx = 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->phcode[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->phcode[i], &line[start_idx], end_idx - start_idx);
			pt->phcode[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->phonetic[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->phonetic[i], &line[start_idx], end_idx - start_idx);
			pt->phonetic[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->pinyin[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->pinyin[i], &line[start_idx], end_idx - start_idx);
			pt->pinyin[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->initial[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->initial[i], &line[start_idx], end_idx - start_idx);
			pt->initial[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->final[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->final[i], &line[start_idx], end_idx - start_idx);
			pt->final[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->medials[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->medials[i], &line[start_idx], end_idx - start_idx);
			pt->medials[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->rime[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->rime[i], &line[start_idx], end_idx - start_idx);
			pt->rime[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t'; end_idx++)
			{
				continue;
			}
			pt->nucleus[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->nucleus[i], &line[start_idx], end_idx - start_idx);
			pt->nucleus[i][end_idx - start_idx] = '\0';
			for (start_idx = end_idx; line[start_idx] == ' ' || line[start_idx] == '\t'; start_idx++)
			{
				continue;
			}

			for (end_idx = start_idx + 1; line[end_idx] != ' ' && line[end_idx] != '\t' && line[end_idx] != '\0'; end_idx++)
			{
				continue;
			}
			pt->coda[i] = (char *)malloc((end_idx - start_idx + 1) * sizeof(char));
			strncpy(pt->coda[i], &line[start_idx], end_idx - start_idx);
			pt->coda[i][end_idx - start_idx] = '\0';

			i++;
		}
	}

	free(line);
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
int Retreat_a_utf8_word(char *line, int *index)
{
	if ((*index) == 0)
	{
		return 0;
	}

	if (line[(*index) - 1] > 0)
	{
		(*index) -= 1;
		return 1;
	}
	else if (line[(*index) - 2] >= -64 && line[(*index) - 2] <= -33)
	{
		(*index) -= 2;
		return 2;
	}
	else if (line[(*index) - 3] >= -32 && line[(*index) - 3] <= -17)
	{
		(*index) -= 3;
		return 3;
	}
	else if (line[(*index) - 4] >= -16 && line[(*index) - 4] <= -9)
	{
		(*index) -= 4;
		return 4;
	}
	else if (line[(*index) - 5] >= -8 && line[(*index) - 5] <= -5)
	{
		(*index) -= 5;
		return 5;
	}
	else if (line[(*index) - 6] >= -4 && line[(*index) - 6] <= -3)
	{
		(*index) -= 6;
		return 6;
	}
	else
	{
		fprintf(stdout, "Retreat error : not utf-8 format.\n");
		exit(1);
	}
	return 1;
}
int lexiconcomparefun(const void *a, const void *b)
{
	const char *pa = ((SINGLEWORD *)a)->word;
	const char *pb = ((SINGLEWORD *)b)->word;
	return strcmp(pa, pb);
}
int lexilenconcomparefun(const void *a, const void *b)
{
	const char *pa = ((SINGLEWORD *)a)->word;
	const char *pb = ((SINGLEWORD *)b)->word;
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
int read_a_utf8_word(char *line, char *word, int *index)
{
	unsigned char trick = 64;	//0x01000000
	unsigned char trick1 = 128; //0x01000000
	int i = 1;

	word[0] = line[(*index)++];
	if (word[0] == '\0')
	{
		word[i] = '\0';
		(*index)--;
		return 1;
	}
	while (word[0] & trick && word[0] & trick1)
	{
		word[i++] = line[(*index)++];
		trick = trick >> 1;
	}
	word[i] = '\0';

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
int bin_lexicon_search_len_match(char *word, LEXICON *lex)
{
	int i, low = 0, up = lex->num_word - 1, middle;

	while (1)
	{
		middle = (low + up) * 0.5;
		if (strncmp(word, lex->sw[middle].word, strlen(lex->sw[middle].word)) > 0)
		{
			low = middle + 1;
		}
		else if (strncmp(word, lex->sw[middle].word, strlen(lex->sw[middle].word)) == 0)
		{
			return middle;
		}
		else if (strncmp(word, lex->sw[middle].word, strlen(lex->sw[middle].word)) < 0)
		{
			up = middle - 1;
		}
		if ((up == low && up == middle) || up < low)
		{
			return -1;
		}
	}
}
unsigned long long int utf8word2decimal(char *word)
{
	unsigned long long int ret = 0;
	int num_byte = strlen(word);
	int i;

	for (i = 0; i < num_byte; i++)
	{
		ret = ret << 8;
		ret += ((unsigned long long int)word[i]) & 255;
	}

	return ret;
}
int syllablecomparefun(const void *a, const void *b)
{
	const char *pa = ((SYLLABLE_TOKEN *)a)->name;
	const char *pb = ((SYLLABLE_TOKEN *)b)->name;
	const char pat = ((SYLLABLE_TOKEN *)a)->tone;
	const char pbt = ((SYLLABLE_TOKEN *)b)->tone;

	if (strcmp(pa, pb) > 0)
	{
		if (pat > pbt)
		{
			return 3;
		}
		else if (pat == pbt)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
	else if (strcmp(pa, pb) == 0)
	{
		if (pat > pbt)
		{
			return 1;
		}
		else if (pat == pbt)
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
		if (pat > pbt)
		{
			return -1;
		}
		else if (pat == pbt)
		{
			return -2;
		}
		else
		{
			return -3;
		}
	}
	return strcmp(pa, pb);
}
int lexiconWPcomparefun(const void *a, const void *b)
{
	int retval = 0;
	const char *pa = ((SINGLEWORD *)a)->word;
	const char *pb = ((SINGLEWORD *)b)->word;
	const char *pap = ((SINGLEWORD *)a)->phoneme_seq;
	const char *pbp = ((SINGLEWORD *)b)->phoneme_seq;
	
	if (strcmp(pa, pb) > 0)
	{
		if (strcmp(pap, pbp) > 0)
		{
			return 3;
		}
		else if (strcmp(pap, pbp) == 0)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
	else if (strcmp(pa, pb) == 0)
	{
		if (strcmp(pap, pbp) > 0)
		{
			return 1;
		}
		else if (strcmp(pap, pbp) == 0)
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
		if (strcmp(pap, pbp) > 0)
		{
			return -1;
		}
		else if (strcmp(pap, pbp) == 0)
		{
			return -2;
		}
		else
		{
			return -3;
		}
	}

	return strcmp(pa, pb);
}