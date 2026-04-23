#include "whclib.h"

typedef struct _KALDI_TEXT_TOKEN_UNIT_
{
	int possible_prou_num;
	char *word;
	char **phoneme_seqs;
} KALDI_TEXT_TOKEN_UNIT;
typedef struct _KALDI_TEXT_TOKEN_
{
	char *utt_name;
	int word_num;
	KALDI_TEXT_TOKEN_UNIT *kttu;
} KALDI_TEXT_TOKEN;
typedef struct _KALDI_TEXT_
{
	int size;
	KALDI_TEXT_TOKEN *ktt;
} KALDI_TEXT;
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
typedef struct _KALDI_ALI_TOKEN_
{
	int id_num;
	char *utt_name;
	int *ids;
	int kaldi_text_index;
} KALDI_ALI_TOKEN;
typedef struct _KALDI_ALI_
{
	int size;
	KALDI_ALI_TOKEN *kat;
} KALDI_ALI;
typedef struct _KALDI_TRANSITION_ID_
{
	int tran_st;
	char *ph;
	int hmm_st;
	int pdf;
	float prob;
	int des_st;
	int is_final_state;
	int is_final_id;
} KALDI_TRANSITION_ID;
typedef struct _KALDI_TRANSITION_
{
	int size;
	KALDI_TRANSITION_ID *ktsi;
} KALDI_TRANSITION;
typedef struct _MODEL_
{
	char *name;
	int hmm_num;
	char **hmm_name;
	float **interval;
} MODEL;
typedef struct _LAB_
{
	char *name;
	int size;
	MODEL *mo;
} LAB;
typedef struct _HTKLABEL_
{
	int num_lab;
	LAB *lab;
} HTKLABEL;
typedef struct _COMPARE_STRUCT_TOKEN_
{
	char *seq;
	int raw_idx;
} COMPARE_STRUCT_TOKEN;
typedef struct _COMPARE_STRUCT_
{
	int total_size;
	int *split_idx, *split_size;
	int maximum_len;
	COMPARE_STRUCT_TOKEN *cst;
} COMPARE_STRUCT;
typedef struct _HMM_STATE_LABEL_TOKEN_
{
	int size;
	float **interval;
	char **name;
	int *state_idx;
	int *pdf;
	int *is_final;
} HMM_STATE_LABEL_TOKEN;
typedef struct _HMM_STATE_LABELS_
{
	int size;
	HMM_STATE_LABEL_TOKEN *hslt;
} HMM_STATE_LABELS;
typedef struct _HMM_LABELS_TOKEN_
{
	int size;
	char **hmm_name;
	float **interval;
} HMM_LABELS_TOKEN;
typedef struct _HMM_LABELS_
{
	int size;
	HMM_LABELS_TOKEN *hlt;
} HMM_LABELS;
typedef struct _LINKED_LIST_GRAPH_
{
	char *word;
	int num_phseq;
	char **phseqs;
	int choiced_idx;
	struct _LINKED_LIST_GRAPH_ *prev, *next;
} LINKED_LIST_GRAPH;
typedef struct _SEARCH_GRAPH_TOKEN_
{
	char *name;
	LINKED_LIST_GRAPH *llg;
} SEARCH_GRAPH_TOKEN;
typedef struct _SEARCH_GRAPH_
{
	int size;
	SEARCH_GRAPH_TOKEN *sgt;
} SEARCH_GRAPH;

void check_text_and_alignment(KALDI_TEXT *kt, KALDI_ALI *ka);
void output_htklab(HTKLABEL *htklab, HMM_STATE_LABELS *hsl, int state_level_flag, char *output_dir);
void final_optional_word_patching(LINKED_LIST_GRAPH *llg, char *phseq);
int recursive_unit(LINKED_LIST_GRAPH *llg, char *phseq, int idx, int depth);
void depth_first_search(SEARCH_GRAPH_TOKEN *sgt, char *phseq);
void phoneme_sequence_search(SEARCH_GRAPH *sg, HMM_LABELS *hl);
void make_search_graph(KALDI_TEXT *kt, SEARCH_GRAPH *sg, KALDI_ALI *ka);
void fill_htklabel(HTKLABEL *htklab, KALDI_TEXT *kt, KALDI_ALI *ka, HMM_LABELS *hl, SEARCH_GRAPH *sg);
void state_lab2hmm_lab(HMM_STATE_LABELS *hsl, HMM_LABELS *hl);
void convert_ali2hmmstatelab(HMM_STATE_LABELS *hsl, KALDI_ALI *ka, KALDI_TRANSITION *kts, float fs);
void ini_htklabel(HTKLABEL *htklab, KALDI_TEXT *kt, KALDI_ALI *ka);
void convert_kali2htmlf(HTKLABEL *htklab, KALDI_TEXT *kt, KALDI_ALI *ka, KALDI_TRANSITION *kts, HMM_STATE_LABELS *hsl, HMM_LABELS *hl, SEARCH_GRAPH *sg, float fs);
void load_kaldi_transition(FILE *ftit, KALDI_TRANSITION *kts);
void load_kaldi_ali(FILE *fali, KALDI_ALI *ka);
int bin_kaldilexiconp_search(char *word, KALDI_LEXICONP *kl);
int kaldilexiconcomparefun(const void *a, const void *b);
int bin_comparestruct_search(char *word, COMPARE_STRUCT *cs);
int comparestructomparefun(const void *a, const void *b);
void load_kaldi_lexp(FILE *fdic, KALDI_LEXICONP *kl, KALDI_LEXICONP_LIST *kll);
void normalize_continous_space_tab(char *raw, char *mod);
void load_kaldi_text(FILE *ftxt, KALDI_TEXT *kt, KALDI_LEXICONP_LIST *kll);
void run(FILE *ftxt, FILE *fdic, FILE *fali, FILE *ftit, float fs, int state_level_flag, char *output_dir);
int main(int argc, char **argv)
{
	FILE *ftxt, *fdic, *fali, *ftit;
	float fs;
	char *output_dir;

	if (argc != 8)
	{
		printf("Usage: kaldi_ali2htk_mlf text.txt dictionary kaldi.ali transition_id_table frame_shift(second) state_level(0/1) output_dir\n");
		exit(1);
	}
	if (!(ftxt = fopen(argv[1], "r")))
	{
		printf("can not open file: %s\n", argv[1]);
		exit(1);
	}
	if (!(fdic = fopen(argv[2], "r")))
	{
		printf("can not open file: %s\n", argv[2]);
		exit(1);
	}
	if (!(fali = fopen(argv[3], "r")))
	{
		printf("can not open file: %s\n", argv[3]);
		exit(1);
	}
	if (!(ftit = fopen(argv[4], "r")))
	{
		printf("can not open file: %s\n", argv[4]);
		exit(1);
	}
	fs = atof(argv[5]);
	if (fs <= 0.0)
	{
		printf("frame_shift must be >=0.0\n");
		exit(1);
	}
	if (atoi(argv[6]) != 0 && atoi(argv[6]) != 1)
	{
		printf("state_level flag must be 0 or 1\n");
		exit(1);
	}

	output_dir = (char *)malloc((strlen(argv[7]) + 2) * sizeof(char));
	strcpy(output_dir, argv[7]);
	if (output_dir[strlen(output_dir) - 1] != '/')
	{
		strcat(output_dir, "/");
	}
	run(ftxt, fdic, fali, ftit, fs, atoi(argv[6]), output_dir);

	fclose(ftxt);
	fclose(fali);
	fclose(ftit);
	free(output_dir);
}
void run(FILE *ftxt, FILE *fdic, FILE *fali, FILE *ftit, float fs, int state_level_flag, char *output_dir)
{
	int i, j, k;
	KALDI_TEXT kt;
	KALDI_LEXICONP kl;
	KALDI_LEXICONP_LIST kll;
	KALDI_ALI ka;
	KALDI_TRANSITION kts;
	HTKLABEL htklab;
	HMM_STATE_LABELS hsl;
	HMM_LABELS hl;
	SEARCH_GRAPH sg;

	load_kaldi_lexp(fdic, &kl, &kll);
	load_kaldi_text(ftxt, &kt, &kll);
	//for (i = 0; i < kt.size; i++)
	//{
	//	printf("%s\n", kt.ktt[i].utt_name);
	//	for (j = 0; j < kt.ktt[i].word_num; j++)
	//	{
	//		printf("\t%s\n", kt.ktt[i].kttu[j].word);
	//		for (k = 0; k < kt.ktt[i].kttu[j].possible_prou_num; k++)
	//		{
	//			printf("\t\t%d %s\n", k + 1, kt.ktt[i].kttu[j].phoneme_seqs[k]);
	//		}
	//	}
	//	printf("\n");
	//}
	//printf("text_size=%d\n", kt.size);
	load_kaldi_ali(fali, &ka);
	//for (i = 0; i < ka.size; i++)
	//{
	//	printf("%s\t", ka.kat[i].utt_name);
	//	for (j = 0; j < ka.kat[i].id_num; j++)
	//	{
	//		printf("%d ", ka.kat[i].ids[j]);
	//	}
	//	printf("\n");
	//}
	check_text_and_alignment(&kt, &ka);
	load_kaldi_transition(ftit, &kts);
	//for (i = 0; i < kts.size; i++)
	//{
	//	printf("tran_id=%d, tran_st=%d, phoneme=%s, pdf=%d, prob=%f [%d -> %d] is_final_state=%d is_final_id=%d\n", i + 1, kts.ktsi[i].tran_st, kts.ktsi[i].ph, kts.ktsi[i].pdf, kts.ktsi[i].prob, kts.ktsi[i].hmm_st, kts.ktsi[i].des_st, kts.ktsi[i].is_final_state, kts.ktsi[i].is_final_id);
	//}
	convert_kali2htmlf(&htklab, &kt, &ka, &kts, &hsl, &hl, &sg, fs);
	output_htklab(&htklab, &hsl, state_level_flag, output_dir);
}
void check_text_and_alignment(KALDI_TEXT *kt, KALDI_ALI *ka)
{
	KALDI_TEXT tmp_kt;
	KALDI_ALI tmp_ka;
	int i, j, k;

	for (i = 0, tmp_kt.size = tmp_ka.size = 0; i < kt->size; i++)
	{
		for (j = 0; j < ka->size; j++)
		{
			if (strcmp(kt->ktt[i].utt_name, ka->kat[j].utt_name) == 0)
			{
				tmp_kt.size++;
				tmp_ka.size++;
				break;
			}
		}
	}
	printf("raw_text_size=%d raw_align_size=%d\n", kt->size, ka->size);
	printf("paired_size=%d\n", tmp_kt.size);

	if (tmp_kt.size == 0)
	{
		exit(1);
	}

	tmp_kt.ktt = (KALDI_TEXT_TOKEN *)malloc(tmp_kt.size * sizeof(KALDI_TEXT_TOKEN));
	tmp_ka.kat = (KALDI_ALI_TOKEN *)malloc(tmp_kt.size * sizeof(KALDI_ALI_TOKEN));

	for (i = 0, k = 0; i < kt->size; i++)
	{
		for (j = 0; j < ka->size; j++)
		{
			if (strcmp(kt->ktt[i].utt_name, ka->kat[j].utt_name) == 0)
			{
				tmp_kt.ktt[k].kttu = kt->ktt[i].kttu;
				tmp_kt.ktt[k].utt_name = kt->ktt[i].utt_name;
				tmp_kt.ktt[k].word_num = kt->ktt[i].word_num;

				tmp_ka.kat[k].id_num = ka->kat[j].id_num;
				tmp_ka.kat[k].ids = ka->kat[j].ids;
				tmp_ka.kat[k].kaldi_text_index = ka->kat[j].kaldi_text_index;
				tmp_ka.kat[k].utt_name = ka->kat[j].utt_name;

				k++;
				break;
			}
		}
	}

	kt->size = tmp_kt.size;
	kt->ktt = tmp_kt.ktt;

	ka->size = tmp_ka.size;
	ka->kat = tmp_ka.kat;
}
void output_htklab(HTKLABEL *htklab, HMM_STATE_LABELS *hsl, int state_level_flag, char *output_dir)
{
	int i, j, k, hmm_state_idx;
	char *output_name = (char *)malloc(sizeof(char));
	FILE *fout;

	for (i = 0; i < htklab->num_lab; i++)
	{
		output_name = (char *)realloc(output_name, (strlen(output_dir) + strlen(htklab->lab[i].name) + 5) * sizeof(char));
		strcpy(output_name, output_dir);
		strcat(output_name, htklab->lab[i].name);
		strcat(output_name, ".lab");
		if (!(fout = fopen(output_name, "w")))
		{
			printf("can not open file: %s\n", output_name);
			exit(1);
		}
		if (state_level_flag == 0)
		{
			for (j = 0; j < htklab->lab[i].size; j++)
			{
				fprintf(fout, "%f %f %s %s\n", htklab->lab[i].mo[j].interval[0][0], htklab->lab[i].mo[j].interval[0][1], htklab->lab[i].mo[j].hmm_name[0], htklab->lab[i].mo[j].name);
				for (k = 1; k < htklab->lab[i].mo[j].hmm_num; k++)
				{
					fprintf(fout, "%f %f %s\n", htklab->lab[i].mo[j].interval[k][0], htklab->lab[i].mo[j].interval[k][1], htklab->lab[i].mo[j].hmm_name[k]);
				}
			}
		}
		else
		{
			for (j = k = hmm_state_idx = 0; hmm_state_idx < hsl->hslt[i].size; hmm_state_idx++)
			{
				fprintf(fout, "%f %f %s_%d_%d", hsl->hslt[i].interval[hmm_state_idx][0], hsl->hslt[i].interval[hmm_state_idx][1], hsl->hslt[i].name[hmm_state_idx], hsl->hslt[i].state_idx[hmm_state_idx], hsl->hslt[i].pdf[hmm_state_idx]);
				if (hsl->hslt[i].state_idx[hmm_state_idx] == 0)
				{
					if (hsl->hslt[i].interval[hmm_state_idx][0] == htklab->lab[i].mo[j].interval[k][0])
					{
						if (k == 0)
						{
							fprintf(fout, " %s %s\n", htklab->lab[i].mo[j].hmm_name[k], htklab->lab[i].mo[j].name);
						}
						else
						{
							fprintf(fout, " %s\n", htklab->lab[i].mo[j].hmm_name[k]);
						}
						k++;
					}
					else
					{
						fprintf(fout, "\n");
					}
					if (k == htklab->lab[i].mo[j].hmm_num)
					{
						j++;
						k = 0;
					}
				}
				else
				{
					fprintf(fout, "\n");
				}
			}
		}
		fclose(fout);
	}

	free(output_name);
}
void convert_kali2htmlf(HTKLABEL *htklab, KALDI_TEXT *kt, KALDI_ALI *ka, KALDI_TRANSITION *kts, HMM_STATE_LABELS *hsl, HMM_LABELS *hl, SEARCH_GRAPH *sg, float fs)
{
	int i, j, k;

	ini_htklabel(htklab, kt, ka);
	convert_ali2hmmstatelab(hsl, ka, kts, fs);
	state_lab2hmm_lab(hsl, hl);
	fill_htklabel(htklab, kt, ka, hl, sg);

	//for (i = 0; i < htklab->num_lab; i++)
	//{
	//	printf("%s\n",htklab->lab[i].name);
	//	for (j = 0; j < htklab->lab[i].size; j++)
	//	{
	//		printf("%f %f %s %s\n", htklab->lab[i].mo[j].interval[0][0], htklab->lab[i].mo[j].interval[0][1], htklab->lab[i].mo[j].hmm_name[0], htklab->lab[i].mo[j].name);
	//		for (k = 1; k < htklab->lab[i].mo[j].hmm_num; k++)
	//		{
	//			printf("%f %f %s\n", htklab->lab[i].mo[j].interval[k][0], htklab->lab[i].mo[j].interval[k][1], htklab->lab[i].mo[j].hmm_name[k]);
	//		}
	//	}
	//	printf("\n");
	//}
}
void fill_htklabel(HTKLABEL *htklab, KALDI_TEXT *kt, KALDI_ALI *ka, HMM_LABELS *hl, SEARCH_GRAPH *sg)
{
	int i, j, k, idx, hl_idx;
	int ph_num, wd_num;
	LINKED_LIST_GRAPH *temp;

	make_search_graph(kt, sg, ka);
	phoneme_sequence_search(sg, hl);

	for (i = 0; i < hl->size; i++)
	{
		for (wd_num = 0, temp = sg->sgt[i].llg; temp != NULL; temp = temp->next)
		{
			wd_num++;
		}
		htklab->lab[i].size = wd_num;
		htklab->lab[i].mo = (MODEL *)malloc(htklab->lab[i].size * sizeof(MODEL));
		for (j = 0, hl_idx = 0, temp = sg->sgt[i].llg; temp != NULL; temp = temp->next, j++)
		{
			htklab->lab[i].mo[j].name = (char *)malloc((strlen(temp->word) + 1) * sizeof(char));
			strcpy(htklab->lab[i].mo[j].name, temp->word);
			for (ph_num = 1, idx = 0; temp->phseqs[temp->choiced_idx][idx] != '\0'; idx++)
			{
				if (temp->phseqs[temp->choiced_idx][idx] == ' ')
				{
					ph_num++;
				}
			}
			//printf("%s %s %d\n", temp->word, temp->phseqs[temp->choiced_idx], ph_num);
			htklab->lab[i].mo[j].hmm_num = ph_num;
			htklab->lab[i].mo[j].hmm_name = (char **)malloc(ph_num * sizeof(char*));
			htklab->lab[i].mo[j].interval = (float **)malloc(ph_num * sizeof(float*));
			for (k = 0; k < ph_num; k++, hl_idx++)
			{
				htklab->lab[i].mo[j].interval[k] = (float *)malloc(2 * sizeof(float));
				htklab->lab[i].mo[j].interval[k][0] = hl->hlt[i].interval[hl_idx][0];
				htklab->lab[i].mo[j].interval[k][1] = hl->hlt[i].interval[hl_idx][1];
				htklab->lab[i].mo[j].hmm_name[k] = (char *)malloc((strlen(hl->hlt[i].hmm_name[hl_idx]) + 1) * sizeof(char));
				strcpy(htklab->lab[i].mo[j].hmm_name[k], hl->hlt[i].hmm_name[hl_idx]);
			}
		}
	}
}
void phoneme_sequence_search(SEARCH_GRAPH *sg, HMM_LABELS *hl)
{
	int i, j, size, ret;
	char *phseq = (char *)malloc(sizeof(char));

	if (sg->size != hl->size)
	{
		printf("error: search_graph nmuber(%d) != HMM_labels number(%d)\n", sg->size, hl->size);
		exit(1);
	}
	for (i = 0; i < sg->size; i++)
	{
		for (j = 0, size = 0; j < hl->hlt[i].size; j++)
		{
			size += strlen(hl->hlt[i].hmm_name[j]) + 1;
		}
		phseq = (char *)realloc(phseq, (size + 1) * sizeof(char));
		for (j = 0, phseq[0] = '\0'; j < hl->hlt[i].size; j++)
		{
			strcat(phseq, hl->hlt[i].hmm_name[j]);
			strcat(phseq, " ");
		}
		phseq[strlen(phseq) - 1] = '\0';

		depth_first_search(&sg->sgt[i], phseq);
	}
	
	free(phseq);
}
void depth_first_search(SEARCH_GRAPH_TOKEN *sgt, char *phseq)
{
	int i, ret;
	LINKED_LIST_GRAPH *temp;

	final_optional_word_patching(sgt->llg, phseq);
	ret = recursive_unit(sgt->llg, phseq, 0, 0);
	if (sgt->llg->prev != NULL)
	{
		sgt->llg = sgt->llg->prev;
	}
	if (ret == -1)
	{
		printf("%s: phoneme sequence and alingment are not match.\n", sgt->name);
		exit(1);
	}
	//for (temp = sgt->llg; temp != NULL; temp = temp->next)
	//{
	//	printf("%s_%d ", temp->word, temp->choiced_idx);
	//}
	//printf("\n");
}
int recursive_unit(LINKED_LIST_GRAPH *llg, char *phseq, int idx, int depth)
{
	int i, ret;
	LINKED_LIST_GRAPH *temp;

	if (strncmp(&phseq[idx], "SIL", 3) == 0 && strcmp(llg->word, "<SILENCE>") != 0)
	{
		temp = (LINKED_LIST_GRAPH *)malloc(sizeof(LINKED_LIST_GRAPH));
		temp->choiced_idx = 0;
		temp->num_phseq = 1;
		temp->phseqs = (char **)malloc(sizeof(char*));
		temp->phseqs[0] = (char *)malloc(4 * sizeof(char));
		strcpy(temp->phseqs[0], "SIL");
		temp->word = (char *)malloc(10 * sizeof(char));
		strcpy(temp->word, "<SILENCE>");
		temp->prev = llg->prev;
		if (idx != 0)
		{
			llg->prev->next = temp;
		}
		temp->next = llg;
		llg->prev = temp;
		llg = llg->prev;
	}
	for (i = 0; i < llg->num_phseq; i++)
	{
		//printf("%d\t%s\n%d\t%s\n", depth, llg->phseqs[i], depth, &phseq[idx]);

		if (strncmp(llg->phseqs[i], &phseq[idx], strlen(llg->phseqs[i])) == 0)
		{
			llg->choiced_idx = i;
			if (llg->next != NULL)
			{
				ret = recursive_unit(llg->next, phseq, idx + strlen(llg->phseqs[i]) + 1, depth + 1);
				if (ret == 1)
				{
					return 1;
				}
				else if (ret == -1)
				{
					if (i < llg->num_phseq - 1)
					{
						continue;
					}
					else
					{
						return -1;
					}
				}
			}
			else
			{
				if (phseq[idx + strlen(llg->phseqs[i])] == '\0')
				{
					return 1;
				}
				else
				{
					if (i < llg->num_phseq - 1)
					{
						continue;
					}
					else
					{
						return -1;
					}
				}
			}
		}
	}
	if (depth == 0)
	{
		return -1;
	}
	return 0;
}
void final_optional_word_patching(LINKED_LIST_GRAPH *llg, char *phseq)
{
	int i, idx, sil_num, exist_sil_num;
	LINKED_LIST_GRAPH *temp, *last;

	//for (temp = llg; temp != NULL; temp = temp->next)
	//{
	//	printf("%s ",temp->word);
	//}
	//printf("\n");
	//printf("%s\n", phseq);

	for (idx = strlen(phseq), sil_num = 0; idx >= 0; idx--)
	{
		if (phseq[idx] == ' ')
		{
			if (strncmp(&phseq[idx + 1], "SIL", 3) == 0)
			{
				sil_num++;
			}
			else
			{
				break;
			}
		}
	}
	//printf("FINAL_SIL_NUM=%d\n", sil_num);
	if (sil_num > 0)
	{
		for (temp = llg; temp->next != NULL; temp = temp->next)
		{
			continue;
		}
		for (last = temp, exist_sil_num = 0; temp != NULL; temp = temp->prev)
		{
			if (strcmp(temp->word, "<SILENCE>") == 0)
			{
				exist_sil_num++;
			}
			else
			{
				break;
			}
		}
		for (i = 0; i < sil_num - exist_sil_num; i++)
		{
			temp = (LINKED_LIST_GRAPH *)malloc(sizeof(LINKED_LIST_GRAPH));
			temp->choiced_idx = -1;
			temp->num_phseq = 1;
			temp->phseqs = (char **)malloc(sizeof(char *));
			temp->phseqs[0] = (char *)malloc(4 * sizeof(char));
			strcpy(temp->phseqs[0], "SIL");
			temp->word = (char *)malloc(10 * sizeof(char));
			strcpy(temp->word, "<SILENCE>");
			last->next = temp;
			temp->prev = last;
			temp->next = NULL;
			last = temp;
		}
	}

	//for (temp = llg; temp != NULL; temp = temp->next)
	//{
	//	printf("%s ", temp->word);
	//}
	//printf("\n");

}
void make_search_graph(KALDI_TEXT *kt, SEARCH_GRAPH *sg, KALDI_ALI *ka)
{
	int i, j, k, text_idx;
	sg->size = ka->size;
	LINKED_LIST_GRAPH *temp, *last;

	sg->sgt = (SEARCH_GRAPH_TOKEN *)malloc(sg->size * sizeof(SEARCH_GRAPH_TOKEN));
	for (i = 0; i < sg->size; i++)
	{
		text_idx = ka->kat[i].kaldi_text_index;
		sg->sgt[i].name = (char *)malloc((strlen(kt->ktt[text_idx].utt_name) + 1) * sizeof(char));
		strcpy(sg->sgt[i].name, kt->ktt[text_idx].utt_name);

		sg->sgt[i].llg = (LINKED_LIST_GRAPH *)malloc(sizeof(LINKED_LIST_GRAPH));
		sg->sgt[i].llg->prev = sg->sgt[i].llg->next = NULL;
		sg->sgt[i].llg->choiced_idx = -1;
		sg->sgt[i].llg->word = (char *)malloc((strlen(kt->ktt[text_idx].kttu[0].word) + 1) * sizeof(char));
		strcpy(sg->sgt[i].llg->word, kt->ktt[text_idx].kttu[0].word);
		sg->sgt[i].llg->num_phseq = kt->ktt[text_idx].kttu[0].possible_prou_num;
		sg->sgt[i].llg->phseqs = (char **)malloc(sg->sgt[i].llg->num_phseq * sizeof(char *));
		for (k = 0; k < kt->ktt[text_idx].kttu[0].possible_prou_num; k++)
		{
			sg->sgt[i].llg->phseqs[k] = (char *)malloc((strlen(kt->ktt[text_idx].kttu[0].phoneme_seqs[k]) + 1) * sizeof(char));
			strcpy(sg->sgt[i].llg->phseqs[k], kt->ktt[text_idx].kttu[0].phoneme_seqs[k]);
		}
		last = sg->sgt[i].llg;

		for (j = 1; j < kt->ktt[text_idx].word_num; j++)
		{
			temp = (LINKED_LIST_GRAPH *)malloc(sizeof(LINKED_LIST_GRAPH));
			temp->prev = last;
			last->next = temp;
			temp->choiced_idx = -1;
			temp->word = (char *)malloc((strlen(kt->ktt[text_idx].kttu[j].word) + 1) * sizeof(char));
			strcpy(temp->word, kt->ktt[text_idx].kttu[j].word);
			temp->num_phseq = kt->ktt[text_idx].kttu[j].possible_prou_num;
			temp->phseqs = (char **)malloc(temp->num_phseq * sizeof(char *));
			for (k = 0; k < kt->ktt[text_idx].kttu[j].possible_prou_num; k++)
			{
				temp->phseqs[k] = (char *)malloc((strlen(kt->ktt[text_idx].kttu[j].phoneme_seqs[k]) + 1) * sizeof(char));
				strcpy(temp->phseqs[k], kt->ktt[text_idx].kttu[j].phoneme_seqs[k]);
			}
			temp->next = NULL;
			last = temp;
		}
		temp = sg->sgt[i].llg;
		//printf("%s\t", sg->sgt[i].name);
		//while (temp != NULL)
		//{
		//	printf("%s ", temp->word);
		//	temp = temp->next;
		//}
		//printf("\n");
	}
}
void state_lab2hmm_lab(HMM_STATE_LABELS *hsl, HMM_LABELS *hl)
{
	int idx, i, j, k;

	hl->size = hsl->size;
	hl->hlt = (HMM_LABELS_TOKEN *)malloc(hl->size * sizeof(HMM_LABELS_TOKEN));
	for (i = 0; i < hsl->size; i++)
	{
		for (j = 0, hl->hlt[i].size = 0; j < hsl->hslt[i].size; j++)
		{
			if (hsl->hslt[i].is_final[j] == 1)
			{
				hl->hlt[i].size++;
			}
		}
	}
	for (i = 0; i < hsl->size; i++)
	{
		hl->hlt[i].hmm_name = (char **)malloc(hl->hlt[i].size * sizeof(char *));
		hl->hlt[i].interval = (float **)malloc(hl->hlt[i].size * sizeof(float *));
		for (j = 0; j < hl->hlt[i].size; j++)
		{
			hl->hlt[i].interval[j] = (float *)malloc(2 * sizeof(float));
		}
		for (idx = 0, j = 0, k = 0; j < hsl->hslt[i].size; j++)
		{
			if (hsl->hslt[i].is_final[j] == 1)
			{
				hl->hlt[i].hmm_name[k] = (char *)malloc((strlen(hsl->hslt[i].name[j - 1]) + 1) * sizeof(char *));
				strcpy(hl->hlt[i].hmm_name[k], hsl->hslt[i].name[j]);
				hl->hlt[i].interval[k][0] = hsl->hslt[i].interval[idx][0];
				hl->hlt[i].interval[k][1] = hsl->hslt[i].interval[j][1];
				idx = j + 1;
				k++;
			}
		}
	}
	//for (i = 0; i < hl->size; i++)
	//{
	//	for (j = 0; j < hl->hlt[i].size; j++)
	//	{
	//		printf("%f %f %s\n", hl->hlt[i].interval[j][0], hl->hlt[i].interval[j][1], hl->hlt[i].hmm_name[j]);
	//	}
	//}
}
void convert_ali2hmmstatelab(HMM_STATE_LABELS *hsl, KALDI_ALI *ka, KALDI_TRANSITION *kts, float fs)
{
	int i, j, k;
	float off_set = 0.0;

	hsl->size = ka->size;
	hsl->hslt = (HMM_STATE_LABEL_TOKEN *)malloc(ka->size * sizeof(HMM_STATE_LABEL_TOKEN));
	for (i = 0; i < ka->size; i++)
	{
		for (j = 1, hsl->hslt[i].size = 1; j < ka->kat[i].id_num; j++)
		{
			//printf("%d %d\n", kts->size, ka->kat[i].ids[j] - 1);
			if (kts->ktsi[ka->kat[i].ids[j] - 1].hmm_st != kts->ktsi[ka->kat[i].ids[j - 1] - 1].hmm_st || strcmp(kts->ktsi[ka->kat[i].ids[j] - 1].ph, kts->ktsi[ka->kat[i].ids[j - 1] - 1].ph) != 0)
			{
				hsl->hslt[i].size++;
			}
		}
		//printf("%s %d\n", ka->kat[i].utt_name, hsl->hslt[i].size);
		hsl->hslt[i].is_final = (int *)malloc(hsl->hslt[i].size * sizeof(int));
		hsl->hslt[i].pdf = (int *)malloc(hsl->hslt[i].size * sizeof(int));
		hsl->hslt[i].state_idx = (int *)malloc(hsl->hslt[i].size * sizeof(int));
		hsl->hslt[i].name = (char **)malloc(hsl->hslt[i].size * sizeof(char *));
		hsl->hslt[i].interval = (float **)malloc(hsl->hslt[i].size * sizeof(float *));
		for (j = 0; j < hsl->hslt[i].size; j++)
		{
			hsl->hslt[i].is_final[j] = 0;
			hsl->hslt[i].interval[j] = (float *)malloc(2 * sizeof(float));
		}
	}
	//printf("here\n");
	for (i = 0; i < ka->size; i++)
	{
		for (k = 0, j = 1, hsl->hslt[i].interval[0][0] = off_set; j < ka->kat[i].id_num; j++)
		{
			if (kts->ktsi[ka->kat[i].ids[j - 1] - 1].is_final_id == 1)
			{
				hsl->hslt[i].is_final[k] = 1;
			}
			if (kts->ktsi[ka->kat[i].ids[j] - 1].hmm_st != kts->ktsi[ka->kat[i].ids[j - 1] - 1].hmm_st || strcmp(kts->ktsi[ka->kat[i].ids[j] - 1].ph, kts->ktsi[ka->kat[i].ids[j - 1] - 1].ph) != 0)
			{
				hsl->hslt[i].interval[k][1] = hsl->hslt[i].interval[k + 1][0] = off_set + j * fs;
				hsl->hslt[i].name[k] = (char *)malloc((strlen(kts->ktsi[ka->kat[i].ids[j - 1] - 1].ph) + 1) * sizeof(char));
				strcpy(hsl->hslt[i].name[k], kts->ktsi[ka->kat[i].ids[j - 1] - 1].ph);
				hsl->hslt[i].pdf[k] = kts->ktsi[ka->kat[i].ids[j - 1] - 1].pdf;
				hsl->hslt[i].state_idx[k] = kts->ktsi[ka->kat[i].ids[j - 1] - 1].hmm_st;
				k++;
			}
		}
		hsl->hslt[i].is_final[k] = 1;
		hsl->hslt[i].interval[k][1] = off_set + j * fs;
		hsl->hslt[i].name[k] = (char *)malloc((strlen(kts->ktsi[ka->kat[i].ids[j - 1] - 1].ph) + 1) * sizeof(char));
		strcpy(hsl->hslt[i].name[k], kts->ktsi[ka->kat[i].ids[j - 1] - 1].ph);
		hsl->hslt[i].pdf[k] = kts->ktsi[ka->kat[i].ids[j - 1] - 1].pdf;
		hsl->hslt[i].state_idx[k] = kts->ktsi[ka->kat[i].ids[j - 1] - 1].hmm_st;
	}

	//for (i = 0; i < hsl->size; i++)
	//{
	//	for (j = 0; j < hsl->hslt[i].size; j++)
	//	{
	//		printf("%f %f %s %d %d %d\n", hsl->hslt[i].interval[j][0], hsl->hslt[i].interval[j][1], hsl->hslt[i].name[j], hsl->hslt[i].state_idx[j], hsl->hslt[i].pdf[j], hsl->hslt[i].is_final[j]);
	//	}
	//}
}
void ini_htklabel(HTKLABEL *htklab, KALDI_TEXT *kt, KALDI_ALI *ka)
{
	int i, j, idx, len, ret, nupair = 0;
	COMPARE_STRUCT cskt;

	cskt.total_size = kt->size;
	cskt.cst = (COMPARE_STRUCT_TOKEN *)malloc(cskt.total_size * sizeof(COMPARE_STRUCT_TOKEN));
	for (i = 0; i < kt->size; i++)
	{
		cskt.cst[i].raw_idx = i;
		cskt.cst[i].seq = (char *)malloc((strlen(kt->ktt[i].utt_name) + 1) * sizeof(char));
		strcpy(cskt.cst[i].seq, kt->ktt[i].utt_name);
	}
	qsort(cskt.cst, cskt.total_size, sizeof(COMPARE_STRUCT_TOKEN), comparestructomparefun);
	cskt.maximum_len = strlen(cskt.cst[cskt.total_size - 1].seq);
	cskt.split_idx = (int *)malloc(cskt.maximum_len * sizeof(int));
	cskt.split_size = (int *)malloc(cskt.maximum_len * sizeof(int));
	for (i = 0; i < cskt.maximum_len; i++)
	{
		cskt.split_idx[i] = cskt.split_size[i] = 0;
	}
	for (i = idx = 0, len = strlen(cskt.cst[0].seq); i < cskt.total_size; i++)
	{
		if (strlen(cskt.cst[i].seq) != len)
		{
			cskt.split_idx[len - 1] = idx;
			cskt.split_size[len - 1] = i - idx;
			idx = i;
			len = strlen(cskt.cst[i].seq);
		}
		if (i == cskt.total_size - 1)
		{
			cskt.split_idx[len - 1] = idx;
			cskt.split_size[len - 1] = i - idx + 1;
		}
	}
	//printf("maximum_len=%d\n", cskt.maximum_len);
	//for (i = 0; i < cskt.maximum_len; i++)
	//{
	//	printf("%d %d %d\n", i + 1, cskt.split_idx[i], cskt.split_size[i]);
	//}
	//for (i = 0; i < cskt.total_size; i++)
	//{
	//	printf("%s %d\n", cskt.cst[i].seq, cskt.cst[i].raw_idx);
	//}

	htklab->num_lab = ka->size;
	htklab->lab = (LAB *)malloc(htklab->num_lab * sizeof(LAB));
	for (i = 0; i < htklab->num_lab; i++)
	{
		htklab->lab[i].name = (char *)malloc((strlen(ka->kat[i].utt_name) + 1) * sizeof(char));
		strcpy(htklab->lab[i].name, ka->kat[i].utt_name);
		ret = bin_comparestruct_search(htklab->lab[i].name, &cskt);
		if (ret == -1)
		{
			printf("%d can't find raw-text----%s\n", nupair++, htklab->lab[i].name);
		}
		else
		{
			//printf("htklab->num_lab=%d %d %d\n", htklab->num_lab, i, ret);
			//printf("%s %s\n", htklab->lab[i].name, kt->ktt[ret].utt_name);
			ka->kat[i].kaldi_text_index = ret;
			//htklab->lab[i].size = kt->ktt[ret].word_num;
			//htklab->lab[i].mo = (MODEL *)malloc(htklab->lab[i].size * sizeof(MODEL));
			//for (j = 0; j < htklab->lab[i].size; j++)
			//{
			//	htklab->lab[i].mo[j].name = (char *)malloc((strlen(kt->ktt[ret].kttu[j].word) + 1) * sizeof(char));
			//	strcpy(htklab->lab[i].mo[j].name, kt->ktt[ret].kttu[j].word);
			//}
		}
	}
}
void load_kaldi_transition(FILE *ftit, KALDI_TRANSITION *kts)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	char *phoneme;
	int i, j, idx, len, tran_st, hmm_st, pdf, tran_id, max_state;

	for (kts->size = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, ftit);
		if (line[0] != '\0' && strncmp(line, " Transition-id", 14) == 0)
		{
			kts->size++;
		}
	}

	fseek(ftit, 0, SEEK_SET);
	phoneme = (char *)malloc(capacity_of_line * sizeof(char));
	kts->ktsi = (KALDI_TRANSITION_ID *)malloc(kts->size * sizeof(KALDI_TRANSITION_ID));
	//printf("Number of transition_id = %d\n", kts->size);

	for (FILEEND = 0, i = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, ftit);
		if (line[0] != '\0' && strncmp(line, "Transition-state ", 17) == 0)
		{
			for (idx = 17, len = 1; line[idx + len] != ':'; len++)
			{
				continue;
			}
			line[idx + len] = '\0';
			tran_st = atoi(&line[idx]);
			
			idx += len + 1 + strlen(" phone = ");
			for (len = 0; line[idx + len] != ' '; len++)
			{
				continue;
			}
			line[idx + len] = '\0';
			strcpy(phoneme, &line[idx]);

			idx += len + strlen(" hmm-state = ");
			for (len = 0; line[idx + len] != ' '; len++)
			{
				continue;
			}
			line[idx + len] = '\0';
			hmm_st = atoi(&line[idx]);

			idx += len + strlen(" pdf = ");
			pdf = atoi(&line[idx]);
		}
		if (line[0] != '\0' && strncmp(line, " Transition-id = ", 17) == 0)
		{
			kts->ktsi[i].tran_st = tran_st;
			kts->ktsi[i].hmm_st = hmm_st;
			kts->ktsi[i].pdf = pdf;
			kts->ktsi[i].ph = (char *)malloc((strlen(phoneme) + 1) * sizeof(char));
			strcpy(kts->ktsi[i].ph, phoneme);
			for (idx = 17, len = 1; line[idx + len] != ' '; len++)
			{
				continue;
			}
			line[idx + len] = '\0';
			tran_id = atoi(&line[idx]);
			idx += len + 1 + strlen("p = ");
			for (len = 0; line[idx + len] != ' '; len++)
			{
				continue;
			}
			line[idx + len] = '\0';
			kts->ktsi[i].prob = atof(&line[idx]);
			idx += len + 1;
			for (; line[idx] != '\0'; idx++)
			{
				if (strncmp(&line[idx], "->", 2) == 0)
				{
					idx += 2;
					break;
				}
			}
			if (line[idx] == '\0')
			{
				kts->ktsi[i].des_st = kts->ktsi[i].hmm_st;
			}
			else
			{
				kts->ktsi[i].des_st = atoi(&line[idx]);
			}

			//printf("tran_id=%d, tran_st=%d, phoneme=%s, pdf=%d, prob=%f [%d -> %d]\n", tran_id, kts->ktsi[i].tran_st, kts->ktsi[i].ph, kts->ktsi[i].pdf, kts->ktsi[i].prob, kts->ktsi[i].hmm_st, kts->ktsi[i].des_st);
			i++;
		}
	}
	for (i = 0; i < kts->size-1; i++)
	{
		if (kts->ktsi[i].hmm_st > kts->ktsi[i + 1].hmm_st)
		{
			for (j = 0;; j--)
			{
				if (kts->ktsi[i].hmm_st != kts->ktsi[i + j].hmm_st)
				{
					break;
				}
				kts->ktsi[i + j].is_final_state = 1;
			}
		}
		else
		{
			kts->ktsi[i].is_final_state = 0;
		}
	}
	for (j = 0;; j--)
	{
		if (kts->ktsi[kts->size - 1].hmm_st != kts->ktsi[kts->size - 1 + j].hmm_st)
		{
			break;
		}
		kts->ktsi[kts->size - 1 + j].is_final_state = 1;
	}

	for (i = 0; i < kts->size - 1; i++)
	{
		if (kts->ktsi[i].is_final_state == 1 && (kts->ktsi[i].des_st > kts->ktsi[i].hmm_st))
		{
			kts->ktsi[i].is_final_id = 1;
		}
		else
		{
			kts->ktsi[i].is_final_id = 0;
		}
	}
	kts->ktsi[kts->size - 1].is_final_id = 1;

	free(line);
	free(phoneme);
}
void load_kaldi_ali(FILE *fali, KALDI_ALI *ka)
{
	int FILEEND, capacity_of_line = 0;
	char *line = (char *)malloc(sizeof(char));
	int i, j, idx, len, sp_num, size;

	for (ka->size = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fali);
		if (line[0] != '\0')
		{
			ka->size++;
		}
	}
	fseek(fali, 0, SEEK_SET);
	ka->kat = (KALDI_ALI_TOKEN *)malloc(ka->size * sizeof(KALDI_ALI_TOKEN));
	for (i = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fali);
		if (line[0] != '\0')
		{
			for (idx = 0; line[idx] != ' ' && line[idx] != '\0'; idx++)
			{
				continue;
			}
			ka->kat[i].utt_name = (char *)malloc((idx + 1) * sizeof(char));
			strncpy(ka->kat[i].utt_name, line, idx);
			ka->kat[i].utt_name[idx] = '\0';
			//printf("%s\n", ka->kat[i].utt_name);
			size = strlen(line);
			for (idx++, len = 0, sp_num = 0; idx + len < size; len++)
			{
				if (line[idx + len] == ' ')
				{
					sp_num++;
				}
			}
			ka->kat[i].id_num = sp_num;
			ka->kat[i].ids = (int *)malloc(ka->kat[i].id_num * sizeof(int));
			for (len = 0, j = 0; idx + len < size; len++)
			{
				if (line[idx + len] == ' ' || idx + len == size - 1)
				{
					line[idx + len] == '\0';
					ka->kat[i].ids[j++] = atoi(&line[idx]);
					idx += len + 1;
					len = 0;
				}
			}
			i++;
		}
	}


	free(line);
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
			//printf("%s %f %s\n", kl->klt[i].word, kl->klt[i].prob, kl->klt[i].phseq);
			i++;
		}
	}
	qsort(kl->klt, kl->size, sizeof(KALDI_LEXICONP_TOKEN), kaldilexiconcomparefun);
	//for (i = 0; i < kl->size; i++)
	//{
	//	printf("%s %f %s\n", kl->klt[i].word, kl->klt[i].prob, kl->klt[i].phseq);
	//}
	//printf("%d\n", kl->size);
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
	//for (i = 0; i < kll->size; i++)
	//{
	//	printf("%d %d\n", i + 1, kll->kls[i].size);
	//}

	free(line);
	free(temp);
}
void load_kaldi_text(FILE *ftxt, KALDI_TEXT *kt, KALDI_LEXICONP_LIST *kll)
{
	int FILEEND, capacity_of_line = 0;
	char *temp;
	char *line = (char *)malloc(sizeof(char));
	int i, j, idx, len, ret, size;

	for (kt->size = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, ftxt);
		if (line[0] != '\0')
		{
			kt->size++;
		}
	}
	fseek(ftxt, 0, SEEK_SET);

	temp = (char *)malloc(capacity_of_line * sizeof(char));
	kt->ktt = (KALDI_TEXT_TOKEN *)malloc(kt->size * sizeof(KALDI_TEXT_TOKEN));

	for (i = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, ftxt);
		if (line[0] != '\0')
		{
			for (idx = strlen(line) - 1; line[idx] == ' '; idx--)
			{
				line[idx] = '\0';
			}
			temp[0] = '\0';
			normalize_continous_space_tab(line, temp);
			for (idx = 0, kt->ktt[i].word_num = 0; line[idx] != '\0'; idx++)
			{
				if (line[idx] == ' ')
				{
					kt->ktt[i].word_num++;
				}
			}
			kt->ktt[i].kttu = (KALDI_TEXT_TOKEN_UNIT *)malloc(kt->ktt[i].word_num * sizeof(KALDI_TEXT_TOKEN_UNIT));
			for (idx = 0; line[idx] != ' '; idx++)
			{
				continue;
			}
			kt->ktt[i].utt_name = (char *)malloc((idx + 1) * sizeof(char));
			strncpy(kt->ktt[i].utt_name, line, idx);
			kt->ktt[i].utt_name[idx] = '\0';
			//printf("%s\t", kt->ktt[i].utt_name);
			for (j = 0; line[idx] != '\0'; idx++)
			{
				if (line[idx] == ' ')
				{
					for (idx++, len = 0; line[idx + len] != ' ' && line[idx + len] != '\0'; len++)
					{
						continue;
					}
					kt->ktt[i].kttu[j].word = (char *)malloc((len + 1) * sizeof(char));
					strncpy(kt->ktt[i].kttu[j].word, &line[idx], len);
					kt->ktt[i].kttu[j].word[len] = '\0';
					j++;
				}
			}
			i++;
		}
	}
	for (i = 0; i < kt->size; i++)
	{
		for (j = 0; j < kt->ktt[i].word_num; j++)
		{
			if (strlen(kt->ktt[i].kttu[j].word) > kll->size)
			{
				ret = -1;
			}
			else
			{
				ret = bin_kaldilexiconp_search(kt->ktt[i].kttu[j].word, &kll->kls[strlen(kt->ktt[i].kttu[j].word) - 1]);
			}
			if (ret == -1)
			{
				printf("can't find this word \'%s\' in dictionary.\n", kt->ktt[i].kttu[j].word);
				exit(1);
			}
			else
			{
				//printf("%d %s %s\n", ret, kt->ktt[i].kttu[j].word, kll->kls[strlen(kt->ktt[i].kttu[j].word) - 1].klt[ret].word);
				len = strlen(kt->ktt[i].kttu[j].word);
				for (idx = ret;; idx--)
				{
					if (strcmp(kt->ktt[i].kttu[j].word, kll->kls[len - 1].klt[idx].word) != 0)
					{
						idx++;
						break;
					}
					if (idx == 0)
					{
						break;
					}
				}
				for (size = 1; idx + size < kll->kls[len - 1].size; size++)
				{
					if (strcmp(kt->ktt[i].kttu[j].word, kll->kls[len - 1].klt[idx + size].word) != 0)
					{
						break;
					}
				}
				//printf("%s %d %d %d\n", kt->ktt[i].kttu[j].word, ret, idx, size);
				
				kt->ktt[i].kttu[j].phoneme_seqs = (char **)malloc(size * sizeof(char *));
				for (kt->ktt[i].kttu[j].possible_prou_num = 0; kt->ktt[i].kttu[j].possible_prou_num < size; kt->ktt[i].kttu[j].possible_prou_num++)
				{
					kt->ktt[i].kttu[j].phoneme_seqs[kt->ktt[i].kttu[j].possible_prou_num] = (char *)malloc((strlen(kll->kls[len - 1].klt[idx + kt->ktt[i].kttu[j].possible_prou_num].phseq) + 1) * sizeof(char));
					strcpy(kt->ktt[i].kttu[j].phoneme_seqs[kt->ktt[i].kttu[j].possible_prou_num], kll->kls[len - 1].klt[idx + kt->ktt[i].kttu[j].possible_prou_num].phseq);
				}
			}
		}
	}

	free(line);
	free(temp);
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
int bin_kaldilexiconp_search(char *word, KALDI_LEXICONP *kl)
{
	int i, low = 0, up = kl->size - 1, middle;

	if (up == -1)
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
int comparestructomparefun(const void *a, const void *b)
{
	const char *pa = ((COMPARE_STRUCT_TOKEN *)a)->seq;
	const char *pb = ((COMPARE_STRUCT_TOKEN *)b)->seq;
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
int bin_comparestruct_search(char *word, COMPARE_STRUCT *cs)
{
	int i, low = 0, up, middle;
	COMPARE_STRUCT_TOKEN *cst;

	if (strlen(word) > cs->maximum_len)
	{
		return -1;
	}
	up = cs->split_size[strlen(word) - 1] - 1;
	cst = &cs->cst[cs->split_idx[strlen(word) - 1]];

	while (1)
	{
		middle = (low + up) * 0.5;
		if (strcmp(word, cst[middle].seq) > 0)
		{
			low = middle + 1;
		}
		else if (strcmp(word, cst[middle].seq) == 0)
		{
			return cst[middle].raw_idx;
		}
		else if (strcmp(word, cst[middle].seq) < 0)
		{
			up = middle - 1;
		}
		if ((up == low && up == middle) || up < low)
		{
			return -1;
		}
	}
}