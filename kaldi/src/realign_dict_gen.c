#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct _TEXT_
{
	int num_of_sentence;
	char **sentence;
} TEXT;
/***************************************/
typedef struct _SINGLEWORD_			 /**/
{									 /**/
	char *word;						 /**/
	double prob_outcome;			 /**/
	int outcome_times;				 /**/
	char *syl_code_seq;				 /**/
	char *phoneme_seq;				 /**/
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
typedef struct _LIST_
{
	int size;
	char **token;
} LIST;
char Number_char[20][4] = {
	{48, '\0'},				/** 0**/
	{49, '\0'},				/** 1**/
	{50, '\0'},				/** 2**/
	{51, '\0'},				/** 3**/
	{52, '\0'},				/** 4**/
	{53, '\0'},				/** 5**/
	{54, '\0'},				/** 6**/
	{55, '\0'},				/** 7**/
	{56, '\0'},				/** 8**/
	{57, '\0'},				/** 9**/
	{-17, -68, -112, '\0'}, /** ０**/
	{-17, -68, -111, '\0'}, /** １**/
	{-17, -68, -110, '\0'}, /** ２**/
	{-17, -68, -109, '\0'}, /** ３**/
	{-17, -68, -108, '\0'}, /** ４**/
	{-17, -68, -107, '\0'}, /** ５**/
	{-17, -68, -106, '\0'}, /** ６**/
	{-17, -68, -105, '\0'}, /** ７**/
	{-17, -68, -104, '\0'}, /** ８**/
	{-17, -68, -103, '\0'}, /** ９**/
};
void run(FILE *fman, FILE *ftab, FILE *feng, char *phoneme_type, int tone_flag, FILE *fout);
void build_mandarin_lexicon(FILE *fdic, LEXICON *lex, LENDICT *ld);
void load_mandarin_lexicon(FILE *fdict, LEXICON *lex);
void load_english_lexicon(FILE *fdict, LEXICON *lex);
int lexilenconcomparefun(const void *a, const void *b);
int lexiconcomparefun(const void *a, const void *b);
void split_dictionary(LENDICT *ld, LEXICON *lex);
void check_dict(LEXICON *lex);
int utf8_word_length(char *word);
int read_a_line(char **line, int *capacity_of_line, FILE *fin);
void build_english_lexicon(FILE *fdic, LEXICON *lex, LENDICT *ld);
void load_pronunciation_table(PRONUNCIATION_TABLE *pt, FILE *fpt);
int strlencomparefun(const void *a, const void *b);
int stringcomparefun(const void *a, const void *b);
void build_realign_dict(LEXICON *Mlex, LENDICT *Mld, PRONUNCIATION_TABLE *pt, LEXICON *Ulex, LENDICT *Uld, char *phoneme_type, int tone_flag);
int bin_string_search(char *sentence, char **set, int set_size);
void add_mandarin_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex);
void add_english_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex);
void output_dict(LEXICON *Ulex, LIST *phonemes, char *s5_dir);
void output_silence_phones(char *dict_dir);
void output_nonsilence_phones(LIST *phonemes, char *dict_dir);
void output_lexicon(LEXICON *Ulex, char *dict_dir);
void output_extra_questions(LIST *phonemes, char *dict_dir);
void find_same_stress_or_tone(char *phlist, LIST *phonemes, char language, char stree_or_tone);
void output_optional_silence(char *dict_dir);
int read_a_utf8_word(char *line, char *word, int *index);
int isCJK_words(char *word);
int isnumber(char *word);
unsigned long long int utf8word2decimal(char *word);
int main(int argc, char **argv)
{
	FILE *fman, *ftab, *feng, *fout;
	if (argc != 7)
	{
		printf("Usage: excute.exe mandarin_dict phone_tab english_dict phoneme_type(phoneme/inifin/syllable) tone_flag output_name\n");
		exit(1);
	}
	if (!(fman = fopen(argv[1], "r")))
	{
		printf("can not open file: %s\n", argv[1]);
		exit(1);
	}
	if (!(ftab = fopen(argv[2], "r")))
	{
		printf("can not open file: %s\n", argv[2]);
		exit(1);
	}
	if (!(feng = fopen(argv[3], "r")))
	{
		printf("can not open file: %s\n", argv[3]);
		exit(1);
	}
	if (strcmp(argv[4], "phoneme") != 0 && strcmp(argv[4], "inifin") != 0 && strcmp(argv[4], "syllable") != 0)
	{
		printf("phoneme_type must be one of below.\nphoneme inifin syllable\n");
		exit(1);
	}
	if (atoi(argv[5]) != 0 && atoi(argv[5]) != 1)
	{
		printf("tone_flag must be zero or one.\n");
		exit(1);
	}
	if (!(fout = fopen(argv[6], "w")))
	{
		printf("can not open file: %s\n", argv[6]);
		exit(1);
	}
	run(fman, ftab, feng, argv[4], atoi(argv[5]), fout);

	fclose(fman);
	fclose(ftab);
	fclose(feng);
	fclose(fout);
	return 0;
}
void run(FILE *fman, FILE *ftab, FILE *feng, char *phoneme_type, int tone_flag, FILE *fout)
{
	LEXICON Mlex, Elex, Ulex;
	LENDICT Mld, Eld, Uld;
	PRONUNCIATION_TABLE pt;

	int i;
	
	build_mandarin_lexicon(fman, &Mlex, &Mld);
	build_english_lexicon(feng, &Elex, &Eld);
	load_pronunciation_table(&pt, ftab);
	build_realign_dict(&Mlex, &Mld, &pt, &Ulex, &Uld, phoneme_type, tone_flag);

	fprintf(fout, "<SILENCE> SIL\n<SPOKEN_NOISE> SPN\n<UNK> SPN\n<NOISE> NSN\n");
	for (i = 0; i < Ulex.num_word; i++)
	{
		fprintf(fout, "%s %s\n", Ulex.sw[i].word, Ulex.sw[i].phoneme_seq);
	}
	for (i = 0; i < Elex.num_word; i++)
	{
		fprintf(fout, "%s %s\n", Elex.sw[i].word, Elex.sw[i].phoneme_seq);
	}
}
void output_optional_silence(char *dict_dir)
{
	FILE *fos;
	char *name = (char *)malloc((strlen(dict_dir) + 64) * sizeof(char));

	strcpy(name, dict_dir);
	strcat(name, "optional_silence.txt");

	if (!(fos = fopen(name, "w")))
	{
		printf("can not open file: %s\n", name);
		exit(1);
	}
	fprintf(fos, "SIL\n");

	fclose(fos);
	free(name);
}
void output_extra_questions(LIST *phonemes, char *dict_dir)
{
	FILE *feq;
	char *name = (char *)malloc((strlen(dict_dir) + 64) * sizeof(char));
	char tail_c, temp[1024], phlist[4096] = "";
	int i, j;

	strcpy(name, dict_dir);
	strcat(name, "extra_questions.txt");

	if (!(feq = fopen(name, "w")))
	{
		printf("can not open file: %s\n", name);
		exit(1);
	}
	fprintf(feq, "SIL SPN NSN\n");
	for (i = 0; i < phonemes->size; i++)
	{
		if (i > 0 && phonemes->token[i][0] >= 97 && phonemes->token[i][0] <= 122)
		{
			if (phonemes->token[i - 1][0] >= 65 && phonemes->token[i - 1][0] <= 90)
			{
				phlist[strlen(phlist) - 1] = '\0';
				fprintf(feq, "%s\n", phlist);
				phlist[0] = '\0';
			}
		}
		tail_c = phonemes->token[i][strlen(phonemes->token[i]) - 1];
		if (tail_c >= 48 && tail_c <= 57)
		{
			strcpy(temp, phonemes->token[i]);
			temp[strlen(temp) - 1] = '\0';
			strcat(phlist, temp);
			strcat(phlist, " ");
			for (j = 1; i + j < phonemes->size; j++)
			{
				if (strlen(phonemes->token[i + j]) - 1 == strlen(temp) && strncmp(phonemes->token[i + j], temp, strlen(temp)) == 0)
				{
					continue;
				}
				else
				{
					break;
				}
			}
			i += j - 1;
		}
		else
		{
			strcat(phlist, phonemes->token[i]);
			strcat(phlist, " ");
		}
	}
	phlist[strlen(phlist) - 1] = '\0';
	fprintf(feq, "%s\n", phlist);

	find_same_stress_or_tone(phlist, phonemes, 'e', '0');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}
	find_same_stress_or_tone(phlist, phonemes, 'e', '1');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}
	find_same_stress_or_tone(phlist, phonemes, 'e', '2');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}

	find_same_stress_or_tone(phlist, phonemes, 'c', '1');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}
	find_same_stress_or_tone(phlist, phonemes, 'c', '2');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}
	find_same_stress_or_tone(phlist, phonemes, 'c', '3');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}
	find_same_stress_or_tone(phlist, phonemes, 'c', '4');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}
	find_same_stress_or_tone(phlist, phonemes, 'c', '5');
	if (strlen(phlist) > 0)
	{
		fprintf(feq, "%s\n", phlist);
	}

	fclose(feq);
	free(name);
}
void find_same_stress_or_tone(char *phlist, LIST *phonemes, char language, char stree_or_tone)
{
	int i;
	char tail_c;
	for (i = 0, phlist[0] = '\0'; i < phonemes->size; i++)
	{
		tail_c = phonemes->token[i][strlen(phonemes->token[i]) - 1];
		if (language == 'e')
		{
			if (phonemes->token[i][0] >= 65 && phonemes->token[i][0] <= 90 && tail_c == stree_or_tone)
			{
				strcat(phlist, phonemes->token[i]);
				strcat(phlist, " ");
			}
		}
		else if (language == 'c')
		{
			if (phonemes->token[i][0] >= 97 && phonemes->token[i][0] <= 122 && tail_c == stree_or_tone)
			{
				strcat(phlist, phonemes->token[i]);
				strcat(phlist, " ");
			}
		}
	}
	if (strlen(phlist) > 0)
	{
		phlist[strlen(phlist) - 1] = '\0';
	}
}
void output_lexicon(LEXICON *Ulex, char *dict_dir)
{
	FILE *flex;
	char *name = (char *)malloc((strlen(dict_dir) + 64) * sizeof(char));
	int i;

	strcpy(name, dict_dir);
	strcat(name, "lexicon.txt");

	if (!(flex = fopen(name, "w")))
	{
		printf("can not open file: %s\n", name);
		exit(1);
	}
	fprintf(flex, "<SILENCE> SIL\n<SPOKEN_NOISE> SPN\n<UNK> SPN\n<NOISE> NSN\n");
	for (i = 0; i < Ulex->num_word; i++)
	{
		fprintf(flex, "%s %s\n", Ulex->sw[i].word, Ulex->sw[i].phoneme_seq);
	}

	fclose(flex);
	free(name);
}
void output_nonsilence_phones(LIST *phonemes, char *dict_dir)
{
	FILE *fnsp;
	char *name = (char *)malloc((strlen(dict_dir) + 64) * sizeof(char));
	char tail_c, temp[1024];
	int i, j;

	strcpy(name, dict_dir);
	strcat(name, "nonsilence_phones.txt");

	if (!(fnsp = fopen(name, "w")))
	{
		printf("can not open file: %s\n", name);
		exit(1);
	}
	for (i = 0; i < phonemes->size; i++)
	{
		tail_c = phonemes->token[i][strlen(phonemes->token[i]) - 1];
		if (tail_c >= 48 && tail_c <= 57)
		{
			strcpy(temp, phonemes->token[i]);
			temp[strlen(temp) - 1] = '\0';
			fprintf(fnsp, "%s %s", temp, phonemes->token[i]);
			for (j = 1; i + j < phonemes->size; j++)
			{
				if (strlen(phonemes->token[i + j]) - 1 == strlen(temp) && strncmp(phonemes->token[i + j], temp, strlen(temp)) == 0)
				{
					fprintf(fnsp, " %s", phonemes->token[i + j]);
				}
				else
				{
					break;
				}
			}
			i += j - 1;
			fprintf(fnsp, "\n");
		}
		else
		{
			fprintf(fnsp, "%s\n", phonemes->token[i]);
		}
	}

	fclose(fnsp);
	free(name);
}
void output_silence_phones(char *dict_dir)
{
	FILE *fsp;
	char *name = (char *)malloc((strlen(dict_dir) + 64) * sizeof(char));

	strcpy(name, dict_dir);
	strcat(name, "silence_phones.txt");

	if (!(fsp = fopen(name, "w")))
	{
		printf("can not open file: %s\n", name);
		exit(1);
	}
	fprintf(fsp, "SIL\nSPN\nNSN\n");

	fclose(fsp);
	free(name);
}
void build_realign_dict(LEXICON *Mlex, LENDICT *Mld, PRONUNCIATION_TABLE *pt, LEXICON *Ulex, LENDICT *Uld, char *phoneme_type, int tone_flag)
{
	int i, j, k, idx, s;
	int len, ret, phcode;
	char wtoken[7], tone[2] = {'\0', '\0'};
	char ptoken[5] = {'\0', '\0', '\0', '\0', '\0'};

	for (i = 0, Ulex->capacity = 0; i < Mld->maximum_len; i++)
	{
		for (j = 0; j < Mld->lexs[i].num_word; j++)
		{
			Ulex->capacity += strlen(Mld->lexs[i].sw[j].word);
		}
	}
	//printf("Ulex->capacity=%d\n", Ulex->capacity);
	Ulex->sw = (SINGLEWORD *)malloc(Ulex->capacity * sizeof(SINGLEWORD));
	for (i = 0, Ulex->num_word = 0; i < Mld->maximum_len; i++)
	{
		for (j = 0; j < Mld->lexs[i].num_word; j++)
		{
			len = strlen(Mld->lexs[i].sw[j].word);
			for (k = idx = 0; read_a_utf8_word(Mld->lexs[i].sw[j].word, wtoken, &idx) == 0; k++)
			{
				strncpy(ptoken, &Mld->lexs[i].sw[j].syl_code_seq[4*k], 4);
				if (isCJK_words(wtoken) == 0 && isnumber(wtoken) == 0)
				{
					continue;
				}
				//printf("%s %s\n", wtoken, ptoken);
				
				for (s = 0; s < Ulex->num_word; s++)
				{
					if (strcmp(wtoken, Ulex->sw[s].word) == 0 && strcmp(ptoken, Ulex->sw[s].syl_code_seq) == 0)
					{
						break;
					}
				}
				if (s == Ulex->num_word)
				{
					Ulex->sw[Ulex->num_word].word = (char *)malloc((strlen(wtoken) + 1) * sizeof(char));
					strcpy(Ulex->sw[Ulex->num_word].word, wtoken);
					Ulex->sw[Ulex->num_word].syl_code_seq = (char *)malloc(5 * sizeof(char));
					strcpy(Ulex->sw[Ulex->num_word].syl_code_seq, ptoken);
					tone[0] = ptoken[0];
					phcode = atoi(&ptoken[1]) - 1;
					//printf("%s %s %d\n", wtoken, ptoken, phcode);
					len = strlen(pt->initial[phcode]) + strlen(pt->final[phcode]) + strlen(pt->medials[phcode]) + strlen(pt->nucleus[phcode]) + strlen(pt->coda[phcode]) + 16;
					Ulex->sw[Ulex->num_word].phoneme_seq = (char *)malloc(len * sizeof(char));
					Ulex->sw[Ulex->num_word].phoneme_seq[0] = '\0';
					if (strcmp(phoneme_type, "phoneme") == 0)
					{
						if (pt->initial[phcode][0] != '/')
						{
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->initial[phcode]);
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
						}
						if (pt->medials[phcode][0] != '/')
						{
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->medials[phcode]);
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
						}
						strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->nucleus[phcode]);
						if (tone_flag == 1)
						{
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, tone);
						}
						if (pt->coda[phcode][0] != '/')
						{
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->coda[phcode]);
						}
					}
					else if (strcmp(phoneme_type, "inifin") == 0)
					{
						if (pt->initial[phcode][0] != '/')
						{
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->initial[phcode]);
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
						}
						strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->final[phcode]);
						if (tone_flag == 1)
						{
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, tone);
						}
					}
					else if (strcmp(phoneme_type, "syllable") == 0)
					{
						strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->pinyin[phcode]);
						if (tone_flag == 1)
						{
							strcat(Ulex->sw[Ulex->num_word].phoneme_seq, tone);
						}
					}
					Ulex->num_word++;
				}
				
			}
		}
	}
	qsort(Ulex->sw, Ulex->num_word, sizeof(SINGLEWORD), lexiconcomparefun);
	//printf("Ulex->capacity=%d Ulex->num_word=%d\n", Ulex->capacity, Ulex->num_word);
	//for (i = 0; i < Ulex->num_word; i++)
	//{
	//	printf("%d %s %s %s\n", i, Ulex->sw[i].word, Ulex->sw[i].syl_code_seq, Ulex->sw[i].phoneme_seq);
	//}
}
void add_english_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex)
{
	Ulex->sw[Ulex->num_word].word = (char *)malloc((strlen(woinfo->word) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].word, woinfo->word);

	Ulex->sw[Ulex->num_word].phoneme_seq = (char *)malloc((strlen(woinfo->phoneme_seq) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].phoneme_seq, woinfo->phoneme_seq);

	Ulex->num_word++;
}
void add_mandarin_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex)
{
	char tone, asyl[4] = {'\0'};
	int idx, len, code_idx, phseq_len;

	// printf("%s %s %s\n", woinfo->word, woinfo->syl_code_seq, woinfo->phoneme_seq);

	Ulex->sw[Ulex->num_word].word = (char *)malloc((strlen(woinfo->word) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].word, woinfo->word);

	Ulex->sw[Ulex->num_word].syl_code_seq = (char *)malloc((strlen(woinfo->syl_code_seq) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].syl_code_seq, woinfo->syl_code_seq);

	for (idx = 0, phseq_len = 0, len = strlen(woinfo->syl_code_seq); idx < len; idx += 4)
	{
		strncpy(asyl, &woinfo->syl_code_seq[idx + 1], 3);
		code_idx = atoi(asyl) - 1;
		phseq_len += strlen(pt->initial[code_idx]) + strlen(pt->medials[code_idx]) + strlen(pt->nucleus[code_idx]) + strlen(pt->coda[code_idx]) + 5;
	}
	Ulex->sw[Ulex->num_word].phoneme_seq = (char *)malloc((phseq_len + 1) * sizeof(char));
	for (idx = 0, len = strlen(woinfo->syl_code_seq), Ulex->sw[Ulex->num_word].phoneme_seq[0] = '\0'; idx < len; idx += 4)
	{
		tone = woinfo->syl_code_seq[idx];
		strncpy(asyl, &woinfo->syl_code_seq[idx + 1], 3);
		code_idx = atoi(asyl) - 1;
		if (pt->initial[code_idx][0] != '/')
		{
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->initial[code_idx]);
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
		}
		if (pt->medials[code_idx][0] != '/')
		{
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->medials[code_idx]);
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
		}
		if (pt->nucleus[code_idx][0] != '/')
		{
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->nucleus[code_idx]);
			Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq) + 1] = '\0';
			Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq)] = tone;
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
		}
		if (pt->coda[code_idx][0] != '/')
		{
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->coda[code_idx]);
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
		}
	}
	Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq) - 1] = '\0';
	// printf("%s\n", Ulex->sw[Ulex->num_word].phoneme_seq);

	Ulex->num_word++;
}
void build_english_lexicon(FILE *fdic, LEXICON *lex, LENDICT *ld)
{
	int i;
	load_english_lexicon(fdic, lex);
	qsort(lex->sw, lex->num_word, sizeof(SINGLEWORD), lexilenconcomparefun);
	split_dictionary(ld, lex);
	for (i = 0; i < ld->maximum_len; i++)
	{
		qsort(ld->lexs[i].sw, ld->lexs[i].num_word, sizeof(SINGLEWORD), lexiconcomparefun);
	}
	// for (i = 0; i < lex->num_word; i++)
	//{
	//	printf("%s %s\n", lex->sw[i].word, lex->sw[i].phoneme_seq);
	// }
}
void load_english_lexicon(FILE *fdict, LEXICON *lex)
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
	lex->sw = (SINGLEWORD *)malloc(lex->num_word * sizeof(SINGLEWORD));
	fseek(fdict, 0, SEEK_SET);
	for (FILEEND = 0, row = 0; FILEEND == 0; row++)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fdict);
		if (line[0] != '\0')
		{
			for (idx = 0; line[idx] != '\t' && line[idx] != ' '; idx++)
			{
				continue;
			}
			lex->sw[row].word = (char *)malloc((idx + 1) * sizeof(char));
			for (i = idx = 0; line[idx] != '\t' && line[idx] != ' '; idx++)
			{
				lex->sw[row].word[i++] = line[idx];
			}
			lex->sw[row].word[i] = '\0';
			for (; line[idx + 1] == '\t' || line[idx + 1] == ' '; idx++)
			{
				continue;
			}
			lex->sw[row].phoneme_seq = (char *)malloc((strlen(line) - idx + 1) * sizeof(char));
			strcpy(lex->sw[row].phoneme_seq, &line[idx + 1]);
		}
	}
	qsort(lex->sw, lex->num_word, sizeof(SINGLEWORD), lexiconcomparefun);
	// for(i=0;i<lex->num_word;i++){
	//	printf("%s %s\n",lex->sw[i].word,lex->sw[i].phoneme_seq);
	// }

	free(line);
}
void build_mandarin_lexicon(FILE *fdic, LEXICON *lex, LENDICT *ld)
{
	int i;

	load_mandarin_lexicon(fdic, lex);
	check_dict(lex);
	qsort(lex->sw, lex->num_word, sizeof(SINGLEWORD), lexilenconcomparefun);
	split_dictionary(ld, lex);
	for (i = 0; i < ld->maximum_len; i++)
	{
		qsort(ld->lexs[i].sw, ld->lexs[i].num_word, sizeof(SINGLEWORD), lexiconcomparefun);
	}
	// for (i = 0; i < lex->num_word; i++)
	//{
	//	printf("%s %s %.31lf\n", lex->sw[i].word, lex->sw[i].syl_code_seq, lex->sw[i].prob_outcome);
	// }
}
void load_mandarin_lexicon(FILE *fdict, LEXICON *lex)
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
			for (; line[idx + 1] == '\t' || line[idx + 1] == ' '; idx++)
				;
			lex->sw[row].syl_code_seq = (char *)malloc((strlen(line) - idx + 1) * sizeof(char));
			for (i = 0, idx++; line[idx] != '\0' && line[idx] != '\t' && line[idx] != ' '; idx++)
			{
				lex->sw[row].syl_code_seq[i++] = line[idx];
			}
			lex->sw[row].syl_code_seq[i] = '\0';
			// printf("%s %s\n",lex->sw[row].word,lex->sw[row].ph_seq);
			// PAUSE_AND_CONTINUE;
			if (line[idx] != '\0')
			{
				lex->sw[row].outcome_times = atoi(&line[idx + 1]);
			}
			else
			{
				lex->sw[row].outcome_times = 0;
			}
		}
	}
	qsort(lex->sw, lex->num_word, sizeof(SINGLEWORD), lexiconcomparefun);
	/*for(i=0;i<lex->num_word;i++){
		printf("%s %s\n",lex->sw[i].word,lex->sw[i].ph_seq);
		PAUSE_AND_CONTINUE;
	}*/

	free(line);
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
int lexiconcomparefun(const void *a, const void *b)
{
	const char *pa = ((SINGLEWORD *)a)->word;
	const char *pb = ((SINGLEWORD *)b)->word;
	return strcmp(pa, pb);
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
	// printf("maximum_len=%d\n", ld->maximum_len);
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
		// printf("len=%d,num=%d\n", i + 1, ld->lexs[i].capacity);
		// ld->lexs[i].sw = (SINGLEWORD *)malloc(ld->lexs[i].capacity * sizeof(SINGLEWORD));
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
void check_dict(LEXICON *lex)
{
	int i, utf8_char_num;

	for (i = 0; i < lex->num_word; i++)
	{
		utf8_char_num = utf8_word_length(lex->sw[i].word);
		if (utf8_char_num * 4 != strlen(lex->sw[i].syl_code_seq))
		{
			printf("Warning: \"%s %s\" may be error.\n", lex->sw[i].word, lex->sw[i].syl_code_seq);
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
			// printf("%d %d %d\n", times, (times - 1) * 1024 - times + 1, *capacity_of_line);
			(*line)[(times - 1) * 1024 - times + 1] = '\0';
		}
		if (cnt == 1022)
		{
			temp[cnt + 1] = '\0';
		}
		strcat(*line, temp);
	}
	// printf("%s\n", *line);

	return FILEEND;
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
int strlencomparefun(const void *a, const void *b)
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
int stringcomparefun(const void *a, const void *b)
{
	const char *pa = (*(char **)a);
	const char *pb = (*(char **)b);
	return strcmp(pa, pb);
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
		// printf("%s %d %d %d\n", set[middle], low, up, middle);
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
int read_a_utf8_word(char *line, char *word, int *index)
{
	unsigned char trick = 64;	// 0x01000000
	unsigned char trick1 = 128; // 0x01000000
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
	// class										unicode		UTF-8				UTF-8_deciaml
	// CJK_Kangxi_Radicals						2F00-2FDF	e2bc80-e2bf9f		14859392-14860191
	// CJK_Radicals_Supplement					2E80-2EFF	e2ba80-e2bbbf		14858880-14859199
	// CJK_Unified_Ideographs_Extension_A		3400-4DBF	e39080-e4b6bf		14913664-14988991‬
	// CJK-Unified_Ideographs					4E00-9FFF	e4b880-e9bfbf		14989440-15318975
	// CJK-Compatibility_Ideographs				F900-FAFF	efa480-efabbf		15705216-15707071
	// CJK_Unified_Ideographs_Extension_B		20000-2A6DF	f0a08080-f0aa9b9f	4037050496-4037712799
	// CJK_Unified_Ideographs_Extension_C		2A700-2B73F	f0aa9c80-f0ab9cbf	4037713024-4037778623
	// CJK_Unified_Ideographs_Extension_D		2B740-2B81F	f0ab9d80-f0aba09f	4037778816-4037779615
	// CJK_Unified_Ideographs_Extension_E		2B820-2CEAF	f0aba0a0-f0acbaaf	4037779616-4037851823
	// CJK_Unified_Ideographs_Extension_F		2CEB0-2EBEF	f0acbab0-f0aeafaf	4037851824-4037980079
	// CJK-Compatibility_Ideographs_Supplement	2F800-2FA1F	f0afa080-f0afa89f	4038041728-4038043807
	// CJK_Unified_Ideographs_Extension_G		30000-3134A	f0b08080-f0b18d8a	4038099072-4038167946

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
int isnumber(char *word)
{
	int i;
	for (i = 0; i < 20; i++)
	{
		if (strcmp(word, Number_char[i]) == 0)
		{
			return 1;
		}
	}
	return 0;
}