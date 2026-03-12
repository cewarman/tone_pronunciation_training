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

void run(FILE *fman, FILE *ftab, FILE *feng, char *type, char *s5_dir);
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
void load_text(TEXT *text, char *s5_dir);
void listing_words(TEXT *text, LIST *words);
int strlencomparefun(const void *a, const void *b);
int stringcomparefun(const void *a, const void *b);
void build_used_dict(LEXICON *Mlex, LENDICT *Mld, PRONUNCIATION_TABLE *pt, LEXICON *Elex, LENDICT *Eld, LEXICON *Ulex, LENDICT *Uld, LIST *words, char *type);
int bin_string_search(char *sentence, char **set, int set_size);
int bin_lexicon_search(char *word, LENDICT *Eld);
void add_mandarin_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex, char *type);
void add_english_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex);
void list_all_phoneme(LEXICON *Ulex, LIST *phonemes);
void output_dict(LEXICON *Ulex, LIST *phonemes, char *s5_dir);
void output_silence_phones(char *dict_dir);
void output_nonsilence_phones(LIST *phonemes, char *dict_dir);
void output_lexicon(LEXICON *Ulex, char *dict_dir);
void output_extra_questions(LIST *phonemes, char *dict_dir);
void find_same_stress_or_tone(char *phlist, LIST *phonemes, char language, char stree_or_tone);
void output_optional_silence(char *dict_dir);
void remove_OOV_data(char *s5_dir, LENDICT *Mld, LENDICT *Eld);
void text_loader(FILE *fin, TEXT *text_in);
void destroy_text(TEXT *text);
void load_data_file(TEXT *text, char *s5_dir, char *target);
void pickup_OOV_sample(TEXT *text, TEXT *utt2spk, TEXT *wav_scp, int *rm_identifier, char *s5_dir, char *dir_text, char *dir_utt2spk, char *dir_wav_scp, char *dir_text_old, char *dir_utt2spk_old, char *dir_wav_old_scp);
int main(int argc, char **argv)
{
	FILE *fman, *ftab, *feng;
	if (argc != 6)
	{
		fprintf(stderr, "Usage: excute.exe mandarin_dict phone_tab type(phoneme/inifin/syllable) english_dict s5_dir\n");
		exit(1);
	}
	if (!(fman = fopen(argv[1], "r")))
	{
		fprintf(stderr, "can not open file: %s\n", argv[1]);
		exit(1);
	}
	if (!(ftab = fopen(argv[2], "r")))
	{
		fprintf(stderr, "can not open file: %s\n", argv[2]);
		exit(1);
	}
	if (strcmp(argv[3], "phoneme") != 0 && strcmp(argv[3], "inifin") != 0 && strcmp(argv[3], "syllable") != 0)
	{
		fprintf(stderr, "type must be one of \'phoneme/inifin/syllable\'\n");
		exit(1);
	}
	if (!(feng = fopen(argv[4], "r")))
	{
		fprintf(stderr, "can not open file: %s\n", argv[4]);
		exit(1);
	}
	run(fman, ftab, feng, argv[3], argv[5]);

	fclose(fman);
	fclose(ftab);
	fclose(feng);
	return 0;
}
void run(FILE *fman, FILE *ftab, FILE *feng, char *type, char *s5_dir)
{
	LEXICON Mlex, Elex, Ulex;
	LENDICT Mld, Eld, Uld;
	PRONUNCIATION_TABLE pt;
	TEXT text;
	LIST words, phonemes;

	int i;
	char *checked_s5_dir = (char *)malloc((strlen(s5_dir) + 2) * sizeof(char));

	strcpy(checked_s5_dir, s5_dir);
	if (checked_s5_dir[strlen(checked_s5_dir) - 1] != '/')
	{
		checked_s5_dir[strlen(checked_s5_dir) + 1] = '\0';
		checked_s5_dir[strlen(checked_s5_dir)] = '/';
	}

	build_mandarin_lexicon(fman, &Mlex, &Mld);
	build_english_lexicon(feng, &Elex, &Eld);
	load_pronunciation_table(&pt, ftab);
	load_text(&text, checked_s5_dir);
	//for (i = 0; i < text.num_of_sentence; i++)
	//{
	//	printf("%s\n", text.sentence[i]);
	//}
	listing_words(&text, &words);
	build_used_dict(&Mlex, &Mld, &pt, &Elex, &Eld, &Ulex, &Uld, &words, type);
//	list_all_phoneme(&Ulex, &phonemes);
//	output_dict(&Ulex, &phonemes, checked_s5_dir);

	remove_OOV_data(checked_s5_dir, &Mld, &Eld);

	free(checked_s5_dir);
}
void pickup_OOV_sample(TEXT *text, TEXT *utt2spk, TEXT *wav_scp, int *rm_identifier, char *s5_dir, char *dir_text, char *dir_utt2spk, char *dir_wav_scp, char *dir_text_old, char *dir_utt2spk_old, char *dir_wav_old_scp)
{
	FILE *ftext, *futt, *fwav, *ftext_old, *futt_old, *fwav_old;
	char *path = (char *)malloc((strlen(s5_dir) + 128) * sizeof(char));
	int i;

	strcpy(path, s5_dir);
	strcat(path, dir_text_old);
	if (!(ftext_old = fopen(path, "w")))
	{
		printf("warning: can not open file: %s\n", path);
	}
	else
	{
		strcpy(path, s5_dir);
		strcat(path, dir_text);
		if (!(ftext = fopen(path, "w")))
		{
			printf("warning: can not open file: %s\n", path);
		}
		else
		{
			for (i = 0; i < text->num_of_sentence; i++)
			{
				if (text->sentence[i][0] != '\0')
				{
					if (rm_identifier[i] == 0)
					{
						fprintf(ftext, "%s\n", text->sentence[i]);
					}
					fprintf(ftext_old, "%s\n", text->sentence[i]);
				}
			}
			fclose(ftext);
		}
		fclose(ftext_old);
	}

	strcpy(path, s5_dir);
	strcat(path, dir_utt2spk_old);
	if (!(futt_old = fopen(path, "w")))
	{
		printf("warning: can not open file: %s\n", path);
	}
	else
	{
		strcpy(path, s5_dir);
		strcat(path, dir_utt2spk);
		if (!(futt = fopen(path, "w")))
		{
			printf("warning: can not open file: %s\n", path);
		}
		else
		{
			for (i = 0; i < utt2spk->num_of_sentence; i++)
			{
				if (utt2spk->sentence[i][0] != '\0')
				{
					if (rm_identifier[i] == 0)
					{
						fprintf(futt, "%s\n", utt2spk->sentence[i]);
					}
					fprintf(futt_old, "%s\n", utt2spk->sentence[i]);
				}
			}
			fclose(futt);
		}
		fclose(futt_old);
	}

	strcpy(path, s5_dir);
	strcat(path, dir_wav_old_scp);
	if (!(fwav_old = fopen(path, "w")))
	{
		printf("warning: can not open file: %s\n", path);
	}
	else
	{
		strcpy(path, s5_dir);
		strcat(path, dir_wav_scp);
		if (!(fwav = fopen(path, "w")))
		{
			printf("warning: can not open file: %s\n", path);
		}
		else
		{
			for (i = 0; i < wav_scp->num_of_sentence; i++)
			{
				if (wav_scp->sentence[i][0] != '\0')
				{
					if (rm_identifier[i] == 0)
					{
						fprintf(fwav, "%s\n", wav_scp->sentence[i]);
					}
					fprintf(fwav_old, "%s\n", wav_scp->sentence[i]);
				}
			}
			fclose(fwav);
		}
		fclose(fwav_old);
	}

	free(path);
}
void load_data_file(TEXT *text, char *s5_dir, char *target)
{
	FILE *ftext;
	char *path = (char *)malloc((strlen(s5_dir) + strlen(target) + 1) * sizeof(char));
	strcpy(path, s5_dir);
	strcat(path, target);
	if (!(ftext = fopen(path, "r")))
	{
		printf("warning: can not open file: %s\n", path);
	}
	else
	{
		text->num_of_sentence = 0;
		text_loader(ftext, text);
		fclose(ftext);
	}
	free(path);
}
void remove_OOV_data(char *s5_dir, LENDICT *Mld, LENDICT *Eld)
{
	TEXT text, utt2spk, wav_scp;
	char *word = (char *)malloc(sizeof(char));
	int i, j, idx, step, ret, rmnum, *rm_identifier = (int *)malloc(sizeof(int));
	char dir_text[3][32] = {"data/train/text", "data/dev/text", "data/test/text"};
	char dir_utt2spk[3][32] = {"data/train/utt2spk", "data/dev/utt2spk", "data/test/utt2spk"};
	char dir_wav_scp[3][32] = {"data/train/wav.scp", "data/dev/wav.scp", "data/test/wav.scp"};
	char dir_text_old[3][32] = {"data/train/text_old", "data/dev/text_old", "data/test/text_old"};
	char dir_utt2spk_old[3][32] = {"data/train/utt2spk_old", "data/dev/utt2spk_old", "data/test/utt2spk_old"};
	char dir_wav_old_scp[3][32] = {"data/train/wav_old.scp", "data/dev/wav_old.scp", "data/test/wav_old.scp"};
	char *path = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));

	for (i = 0, text.num_of_sentence = utt2spk.num_of_sentence = wav_scp.num_of_sentence = 0; i < 3; i++, text.num_of_sentence = utt2spk.num_of_sentence = wav_scp.num_of_sentence = 0)
	{
		load_data_file(&text, s5_dir, dir_text[i]);
		load_data_file(&utt2spk, s5_dir, dir_utt2spk[i]);
		load_data_file(&wav_scp, s5_dir, dir_wav_scp[i]);

		if (text.num_of_sentence > 0)
		{
			rmnum = 0;
			rm_identifier = (int *)realloc(rm_identifier, text.num_of_sentence * sizeof(int));
			for (j = 0; j < text.num_of_sentence; j++)
			{
				rm_identifier[j] = 0;
			}
			for (j = 0, ret = 0; j < text.num_of_sentence; j++, ret = 0)
			{
				word = (char *)realloc(word, strlen(text.sentence[j]) * sizeof(char));
				for (idx = 1; text.sentence[j][idx - 1] != ' ' && text.sentence[j][idx] != '\0'; idx++)
				{
					continue;
				}
				for (; text.sentence[j][idx] != '\0';)
				{
					for (step = 0; text.sentence[j][idx + step] != ' ' && text.sentence[j][idx + step] != '\0'; step++)
					{
						continue;
					}
					strncpy(word, &text.sentence[j][idx], step);
					word[step] = '\0';
					//printf("%s ",word);

					ret = bin_lexicon_search(word, Mld);
					if (ret == -1)
					{
						ret = bin_lexicon_search(word, Eld);
						if (ret == -1)
						{
							printf("%s\t%s\n", word, text.sentence[j]);
							rmnum++;
							rm_identifier[j] = 1;
							break;
						}
					}

					idx += step;
					if (text.sentence[j][idx] == ' ')
					{
						idx++;
					}
				}
			}
			if (rmnum > 0)
			{
				pickup_OOV_sample(&text, &utt2spk, &wav_scp, rm_identifier, s5_dir, dir_text[i], dir_utt2spk[i], dir_wav_scp[i], dir_text_old[i], dir_utt2spk_old[i], dir_wav_old_scp[i]);
			}
		}

		if (text.num_of_sentence > 0)
		{
			destroy_text(&text);
		}
		if (utt2spk.num_of_sentence > 0)
		{
			destroy_text(&utt2spk);
		}
		if (wav_scp.num_of_sentence > 0)
		{
			destroy_text(&wav_scp);
		}
	}

	free(path);
	free(word);
	free(rm_identifier);
}
void output_dict(LEXICON *Ulex, LIST *phonemes, char *s5_dir)
{
	char *dict_path = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));

	strcpy(dict_path, s5_dir);
	strcat(dict_path, "data/local/dict/");
	//printf("%s\n", dict_path);
	output_silence_phones(dict_path);
	output_nonsilence_phones(phonemes, dict_path);
	output_lexicon(Ulex, dict_path);
	output_extra_questions(phonemes, dict_path);
	output_optional_silence(dict_path);

	free(dict_path);
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
	char tail_c, temp[8192], phlist[8192] = "";
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
		if ((i > 0 && phonemes->token[i][0] >= 97 && phonemes->token[i][0] <= 122) || phonemes->token[i][0] == '_')
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
			if ((phonemes->token[i][0] >= 97 && phonemes->token[i][0] <= 122 || phonemes->token[i][0] == '_') && tail_c == stree_or_tone)
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
void list_all_phoneme(LEXICON *Ulex, LIST *phonemes)
{
	LIST temp;
	int i, j, idx, phseq_len, len, ini_cap = 8192;

	temp.token = (char **)malloc(ini_cap * sizeof(char *));
	for (i = 0, temp.size = 0; i < Ulex->num_word; i++)
	{
		for (idx = 0, len = 0, phseq_len = strlen(Ulex->sw[i].phoneme_seq); idx < phseq_len; len++)
		{
			if (Ulex->sw[i].phoneme_seq[idx + len] == ' ' || Ulex->sw[i].phoneme_seq[idx + len] == '\0')
			{
				//printf("%s %d %d\n", &Ulex->sw[i].phoneme_seq[idx + len], idx, len);
				for (j = 0; j < temp.size; j++)
				{
					if (strlen(temp.token[j]) == len && strncmp(&Ulex->sw[i].phoneme_seq[idx], temp.token[j], len) == 0)
					{
						break;
					}
				}
				if (j == temp.size)
				{
					temp.token[temp.size] = (char *)malloc((len + 1) * sizeof(char));
					strncpy(temp.token[temp.size], &Ulex->sw[i].phoneme_seq[idx], len);
					temp.token[temp.size][len] = '\0';
					temp.size++;
				}
				idx += len + 1;
				len = 0;
			}
		}
	}
	//printf("temp.size=%d\n", temp.size);
	qsort(temp.token, temp.size, sizeof(char *), stringcomparefun);
	//for (i = 0; i < temp.size; i++)
	//{
	//	printf("%s\n", temp.token[i]);
	//}
	phonemes->size = temp.size;
	phonemes->token = (char **)malloc(phonemes->size * sizeof(char *));
	for (i = 0; i < temp.size; i++)
	{
		phonemes->token[i] = (char *)malloc((strlen(temp.token[i]) + 1) * sizeof(char));
		strcpy(phonemes->token[i], temp.token[i]);
	}
	for (i = 0; i < temp.size; i++)
	{
		free(temp.token[i]);
	}
	free(temp.token);
}
void build_used_dict(LEXICON *Mlex, LENDICT *Mld, PRONUNCIATION_TABLE *pt, LEXICON *Elex, LENDICT *Eld, LEXICON *Ulex, LENDICT *Uld, LIST *words, char *type)
{
	int i, j, maximum_word_len = strlen(words->token[words->size - 1]);
	char ***split_word = (char ***)malloc(maximum_word_len * sizeof(char **));
	int *split_num = (int *)malloc(maximum_word_len * sizeof(int));
	int len, ret;

	for (i = 0; i < maximum_word_len; i++)
	{
		split_num[i] = 0;
	}
	for (i = 0, len = 1; i < words->size; i++)
	{
		if (strlen(words->token[i]) == len && split_num[len - 1] == 0)
		{
			split_word[len - 1] = &words->token[i];
			split_num[len - 1] = 1;
		}
		else if (strlen(words->token[i]) == len)
		{
			split_num[len - 1]++;
		}
		else if (strlen(words->token[i]) > len)
		{
			len = strlen(words->token[i]);
			split_word[len - 1] = &words->token[i];
			split_num[len - 1] = 1;
		}
	}
	for (i = 0, Ulex->capacity = 0; i < Mld->maximum_len; i++)
	{
		for (j = 0; j < Mld->lexs[i].num_word; j++)
		{
			len = strlen(Mld->lexs[i].sw[j].word);
			if (maximum_word_len < len)
			{
				break;
			}
			ret = bin_string_search(Mld->lexs[i].sw[j].word, split_word[len - 1], split_num[len - 1]);
			if (ret != -1)
			{
				Ulex->capacity++;
			}
		}
	}
	for (i = 0; i < Eld->maximum_len; i++)
	{
		for (j = 0; j < Eld->lexs[i].num_word; j++)
		{
			len = strlen(Eld->lexs[i].sw[j].word);
			if (maximum_word_len < len)
			{
				break;
			}
			ret = bin_string_search(Eld->lexs[i].sw[j].word, split_word[len - 1], split_num[len - 1]);
			if (ret != -1)
			{
				Ulex->capacity++;
			}
		}
	}
	Ulex->sw = (SINGLEWORD *)malloc(Ulex->capacity * sizeof(SINGLEWORD));
	for (i = 0, Ulex->num_word = 0; i < Mld->maximum_len; i++)
	{
		for (j = 0; j < Mld->lexs[i].num_word; j++)
		{
			len = strlen(Mld->lexs[i].sw[j].word);
			if (maximum_word_len < len)
			{
				break;
			}
			ret = bin_string_search(Mld->lexs[i].sw[j].word, split_word[len - 1], split_num[len - 1]);
			if (ret != -1)
			{
				//printf("%s %d %d\n", Mld->lexs[i].sw[j].word, i, j);
				add_mandarin_word(&Mld->lexs[i].sw[j], pt, Ulex, type);
			}
		}
	}
	for (i = 0; i < Eld->maximum_len; i++)
	{
		for (j = 0; j < Eld->lexs[i].num_word; j++)
		{
			len = strlen(Eld->lexs[i].sw[j].word);
			if (maximum_word_len < len)
			{
				break;
			}
			ret = bin_string_search(Eld->lexs[i].sw[j].word, split_word[len - 1], split_num[len - 1]);
			if (ret != -1)
			{
				add_english_word(&Eld->lexs[i].sw[j], pt, Ulex);
			}
		}
	}
	//printf("Ulex->capacity=%d Ulex->num_word=%d\n", Ulex->capacity, Ulex->num_word);
	//for (i = 0; i < Ulex->num_word; i++)
	//{
	//	printf("%s %s\n", Ulex->sw[i].word, Ulex->sw[i].phoneme_seq);
	//}
	for (i = 0; i < maximum_word_len; i++)
	{
		for (j = 0; j < split_num[i]; j++)
		{
			ret = bin_lexicon_search(split_word[i][j], Eld);
			if (ret == -1)
			{
				ret = bin_lexicon_search(split_word[i][j], Mld);
				if (ret == -1)
				{
					printf("Warning: \'%s\' dosen't exist in dictionary.\n", split_word[i][j]);
				}
			}
		}
	}
	free(split_word);
	free(split_num);
}
void add_english_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex)
{
	Ulex->sw[Ulex->num_word].word = (char *)malloc((strlen(woinfo->word) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].word, woinfo->word);

	Ulex->sw[Ulex->num_word].phoneme_seq = (char *)malloc((strlen(woinfo->phoneme_seq) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].phoneme_seq, woinfo->phoneme_seq);

	Ulex->num_word++;
}
void add_mandarin_word(SINGLEWORD *woinfo, PRONUNCIATION_TABLE *pt, LEXICON *Ulex, char *type)
{
	char tone, asyl[4]={'\0'};
	int idx, len, code_idx, phseq_len;

	//printf("%s %s %s\n", woinfo->word, woinfo->syl_code_seq, woinfo->phoneme_seq);

	Ulex->sw[Ulex->num_word].word = (char *)malloc((strlen(woinfo->word) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].word, woinfo->word);

	Ulex->sw[Ulex->num_word].syl_code_seq = (char *)malloc((strlen(woinfo->syl_code_seq) + 1) * sizeof(char));
	strcpy(Ulex->sw[Ulex->num_word].syl_code_seq, woinfo->syl_code_seq);

	for (idx = 0, phseq_len = 0, len = strlen(woinfo->syl_code_seq); idx < len; idx += 4)
	{
		strncpy(asyl, &woinfo->syl_code_seq[idx + 1], 3);
		code_idx = atoi(asyl) - 1;
		phseq_len += strlen(pt->initial[code_idx]) + strlen(pt->medials[code_idx]) + strlen(pt->nucleus[code_idx]) + strlen(pt->coda[code_idx]) + strlen(pt->final[code_idx]) + strlen(pt->pinyin[code_idx]) + 5;
	}
	Ulex->sw[Ulex->num_word].phoneme_seq = (char *)malloc((phseq_len + 1) * sizeof(char));
	for (idx = 0, len = strlen(woinfo->syl_code_seq), Ulex->sw[Ulex->num_word].phoneme_seq[0] = '\0'; idx < len; idx += 4)
	{
		tone = woinfo->syl_code_seq[idx];
		strncpy(asyl, &woinfo->syl_code_seq[idx + 1], 3);
		code_idx = atoi(asyl) - 1;
		if (strcmp(type, "phoneme") == 0)
		{
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
		else if (strcmp(type, "inifin") == 0)
		{
			if (pt->initial[code_idx][0] != '/')
			{
				strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->initial[code_idx]);
				strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
			}
			if (pt->final[code_idx][0] != '/')
			{
				strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->final[code_idx]);
				Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq) + 1] = '\0';
				Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq)] = tone;
				strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
			}
		}
		else
		{
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, pt->pinyin[code_idx]);
			Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq) + 1] = '\0';
			Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq)] = tone;
			strcat(Ulex->sw[Ulex->num_word].phoneme_seq, " ");
		}
	}
	Ulex->sw[Ulex->num_word].phoneme_seq[strlen(Ulex->sw[Ulex->num_word].phoneme_seq) - 1] = '\0';
	//printf("%s\n", Ulex->sw[Ulex->num_word].phoneme_seq);

	Ulex->num_word++;
}
void listing_words(TEXT *text, LIST *words)
{
	LIST temp;
	int i, j, idx, wlen, space_num, wsize;

	for (i = 0, temp.size = 0; i < text->num_of_sentence; i++)
	{
		for (idx = 0, space_num = 0; text->sentence[i][idx] != '\0'; idx++)
		{
			if (text->sentence[i][idx] == ' ')
			{
				space_num++;
			}
		}
		temp.size += space_num + 1;
	}
	temp.token = (char **)malloc(temp.size * sizeof(char *));
	for (i = j = 0; i < text->num_of_sentence; i++)
	{
		for (idx = 0, wlen = 0;;)
		{
			if (text->sentence[i][idx + wlen] == ' ' || text->sentence[i][idx + wlen] == '\0')
			{
				temp.token[j] = (char *)malloc((wlen + 1) * sizeof(char));
				strncpy(temp.token[j], &text->sentence[i][idx], wlen + 1);
				temp.token[j++][wlen] = '\0';
				if (text->sentence[i][idx + wlen] == '\0')
				{
					break;
				}
				idx += wlen + 1;
				wlen = 0;
			}
			else
			{
				wlen++;
			}
		}
	}
	qsort(temp.token, temp.size, sizeof(char *), strlencomparefun);
	for (i = 0, idx = 0, wlen = 1, wsize = 0; i < temp.size; i++)
	{
		if (strlen(temp.token[i]) == wlen)
		{
			wsize++;
		}
		else
		{
			qsort(&temp.token[idx], wsize, sizeof(char *), stringcomparefun);
			idx += wsize;
			wlen = strlen(temp.token[i]);
			wsize = 1;
		}
		if (i == temp.size - 1)
		{
			qsort(&temp.token[idx], wsize, sizeof(char *), stringcomparefun);
		}
	}
	//for (i = 0; i < temp.size; i++)
	//{
	//	printf("%s\n", temp.token[i]);
	//}
	//printf("total_words_number: %d\n", temp.size);
	for (i = 1, words->size = 1; i < temp.size; i++)
	{
		if (strcmp(temp.token[i], temp.token[i - 1]) != 0)
		{
			words->size++;
		}
	}
	words->token = (char **)malloc(words->size * sizeof(char *));
	words->token[0] = (char *)malloc((strlen(temp.token[0]) + 1) * sizeof(char));
	strcpy(words->token[0], temp.token[0]);
	for (i = 1, j = 1; i < temp.size; i++)
	{
		if (strcmp(temp.token[i], temp.token[i - 1]) != 0)
		{
			words->token[j] = (char *)malloc((strlen(temp.token[i]) + 1) * sizeof(char));
			strcpy(words->token[j++], temp.token[i]);
		}
	}
	//printf("used_words_number: %d\n", words->size);
	//for (i = 0; i < words->size; i++)
	//{
	//	printf("%s\n", words->token[i]);
	//}

	for (i = 0; i < temp.size; i++)
	{
		free(temp.token[i]);
	}
	free(temp.token);
}
void load_text(TEXT *text, char *s5_dir)
{
	FILE *fin;
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int i, j, idx;
	char dir_text[3][32] = {"data/train/text", "data/dev/text", "data/test/text"};
	char *path = (char *)malloc((strlen(s5_dir) + 64) * sizeof(char));

	for (i = 0, text->num_of_sentence = 0; i < 3; i++)
	{
		strcpy(path, s5_dir);
		strcat(path, dir_text[i]);
		if (!(fin = fopen(path, "r")))
		{
			printf("warning: can not open file: %s\n", path);
		}
		else
		{
			for (FILEEND = 0; FILEEND == 0;)
			{
				FILEEND = read_a_line(&line, &capacity_of_line, fin);
				if (line[0] != '\0')
				{
					text->num_of_sentence++;
				}
			}
			fclose(fin);
		}
	}
	text->sentence = (char **)malloc(text->num_of_sentence * sizeof(char *));
	for (i = 0, j = 0; i < 3; i++)
	{
		strcpy(path, s5_dir);
		strcat(path, dir_text[i]);
		if (!(fin = fopen(path, "r")))
		{
			continue;
		}
		else
		{
			for (FILEEND = 0; FILEEND == 0;)
			{
				FILEEND = read_a_line(&line, &capacity_of_line, fin);
				if (line[0] != '\0')
				{
					for (idx = 1; line[idx - 1] != ' '; idx++)
					{
						continue;
					}
					text->sentence[j] = malloc((strlen(&line[idx]) + 1) * sizeof(char));
					strcpy(text->sentence[j++], &line[idx]);
				}
			}
			fclose(fin);
		}
	}

	free(path);
	free(line);
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
	//for (i = 0; i < lex->num_word; i++)
	//{
	//	printf("%s %s\n", lex->sw[i].word, lex->sw[i].phoneme_seq);
	//}
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
	//for(i=0;i<lex->num_word;i++){
	//	printf("%s %s\n",lex->sw[i].word,lex->sw[i].phoneme_seq);
	//}

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
	//for (i = 0; i < lex->num_word; i++)
	//{
	//	printf("%s %s %.31lf\n", lex->sw[i].word, lex->sw[i].syl_code_seq, lex->sw[i].prob_outcome);
	//}
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
			//printf("%s %s\n",lex->sw[row].word,lex->sw[row].ph_seq);
			//PAUSE_AND_CONTINUE;
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
int bin_lexicon_search(char *word, LENDICT *Eld)
{
	int i, low = 0, up, middle, len;

	if (strlen(word) > Eld->maximum_len)
	{
		return -1;
	}
	len = strlen(word);
	up = Eld->lexs[len - 1].num_word - 1;
	if (up < 0)
	{
		return -1;
	}

	while (1)
	{
		middle = (low + up) * 0.5;

		if (strcmp(word, Eld->lexs[len - 1].sw[middle].word) > 0)
		{
			low = middle + 1;
		}
		else if (strcmp(word, Eld->lexs[len - 1].sw[middle].word) == 0)
		{
			return middle;
		}
		else if (strcmp(word, Eld->lexs[len - 1].sw[middle].word) < 0)
		{
			up = middle - 1;
		}
		if ((up == low && up == middle) || up < low)
		{
			return -1;
		}
	}
}
void text_loader(FILE *fin, TEXT *text_in)
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
void destroy_text(TEXT *text)
{
	int i;

	for (i = 0; i < text->num_of_sentence; i++)
	{
		free(text->sentence[i]);
	}
	free(text->sentence);
}