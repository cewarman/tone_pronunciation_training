#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _TEXT_
{
	int num_of_sentence;
	char **sentence;
} TEXT;
typedef struct _FILE_NAME_
{
	int idx;
	char *name;
} FILE_NAME;
typedef struct _FILE_NAME_LIST_
{
	int size;
	FILE_NAME *file_name;
	int max_name_length, *len_idx, *len_size;
} FILE_NAME_LIST;

void run(FILE *fsrc, FILE *ftag, FILE *fout);
void load_text(FILE *fin, TEXT *text_in);
int read_a_line(char **line, int *capacity_of_line, FILE *fin);
int filename_len_comparefun(const void *a, const void *b);
int bin_string_search(char *sentence, char **set, int set_size);
int stringcomparefun(const void *a, const void *b);
void list_file_name(TEXT *kaldi_text, FILE_NAME_LIST *fnl);
void change_tag(TEXT *src, TEXT *tag, FILE_NAME_LIST *fnl);
int get_tag_idx(char *name, FILE_NAME_LIST *fnl);
int filename_string_comparefun(const void *a, const void *b);
void output_tag(TEXT *tag, FILE *fout);
int main(int argc, char **argv)
{
	FILE *fsrc, *ftag, *fout;
	if (argc != 4)
	{
		printf("Usage: text_substitute source_text target_text output_text\n");
		exit(1);
	}
	if (!(fsrc = fopen(argv[1], "r")))
	{
		printf("can not open file: %s\n", argv[1]);
		exit(1);
	}
	if (!(ftag = fopen(argv[2], "r")))
	{
		printf("can not open file: %s\n", argv[2]);
		exit(1);
	}
	if (!(fout = fopen(argv[3], "w")))
	{
		printf("can not open file: %s\n", argv[3]);
		exit(1);
	}
	run(fsrc, ftag, fout);

	fclose(fsrc);
	fclose(ftag);
	fclose(fout);
	return 0;
}
void run(FILE *fsrc, FILE *ftag, FILE *fout)
{
	TEXT src, tag;
	FILE_NAME_LIST fnl;
	int i;

	load_text(fsrc, &src);
	load_text(ftag, &tag);

	list_file_name(&tag, &fnl);
	//for (i = 0; i < fnl.size; i++)
	//{
	//	printf("%d %s\n", fnl.file_name[i].idx, fnl.file_name[i].name);
	//}
	change_tag(&src, &tag, &fnl);
	output_tag(&tag, fout);
}
void output_tag(TEXT *tag, FILE *fout)
{
	int i;

	for (i = 0; i < tag->num_of_sentence; i++)
	{
		fprintf(fout, "%s\n", tag->sentence[i]);
	}
}
void change_tag(TEXT *src, TEXT *tag, FILE_NAME_LIST *fnl)
{
	int i, idx, ret;
	char name[2048];

	for (i = 0; i < src->num_of_sentence; i++)
	{
		if (src->sentence[i][0] != '\0')
		{
			for (idx = 0; src->sentence[i][idx] != ' '; idx++)
			{
				continue;
			}
			strncpy(name, src->sentence[i], idx);
			name[idx]='\0';
			//printf("%d %s\n", i, name);
			ret = get_tag_idx(name, fnl);
			if (ret != -1)
			{
				//printf("%s %s %d\n", name, fnl->file_name[ret].name, fnl->file_name[ret].idx);
				tag->sentence[fnl->file_name[ret].idx] = (char *)realloc(tag->sentence[fnl->file_name[ret].idx], (strlen(src->sentence[i]) + 1) * sizeof(char));
				strcpy(tag->sentence[fnl->file_name[ret].idx], src->sentence[i]);
			}
		}
	}
}
int get_tag_idx(char *name, FILE_NAME_LIST *fnl)
{
	int size, len, idx;
	int middle, low, up;

	len = strlen(name);
	size = fnl->len_size[len - 1];
	idx = fnl->len_idx[len - 1];

	if (size == 0)
	{
		return -1;
	}

	low = idx;
	up = idx + size - 1;

	while (1)
	{
		middle = (low + up) * 0.5;
		// printf("%s %d %d %d\n", set[middle], low, up, middle);
		if (strcmp(name, fnl->file_name[middle].name) > 0)
		{
			low = middle + 1;
		}
		else if (strcmp(name, fnl->file_name[middle].name) == 0)
		{
			return middle;
		}
		else if (strcmp(name, fnl->file_name[middle].name) < 0)
		{
			up = middle - 1;
		}
		if ((up == low && up == middle) || up < low)
		{
			return -1;
		}
	}
}
void list_file_name(TEXT *kaldi_text, FILE_NAME_LIST *fnl)
{
	int i, idx;

	for (i = fnl->size = 0; i < kaldi_text->num_of_sentence; i++)
	{
		if (kaldi_text->sentence[i][0] != '\0')
		{
			fnl->size++;
		}
	}

	fnl->file_name = (FILE_NAME *)malloc(fnl->size * sizeof(FILE_NAME));

	for (i = 0; i < kaldi_text->num_of_sentence; i++)
	{
		if (kaldi_text->sentence[i][0] != '\0')
		{
			for (idx = 0; kaldi_text->sentence[i][idx] != ' '; idx++)
			{
				continue;
			}
			fnl->file_name[i].idx = i;
			fnl->file_name[i].name = (char *)malloc((idx + 1) * sizeof(char));
			strncpy(fnl->file_name[i].name, kaldi_text->sentence[i], idx);
			fnl->file_name[i].name[idx] = '\0';
		}
	}

	qsort((void *)fnl->file_name, fnl->size, sizeof(FILE_NAME), filename_len_comparefun);

	fnl->max_name_length = strlen(fnl->file_name[fnl->size - 1].name);
	fnl->len_idx = (int *)malloc(fnl->max_name_length * sizeof(int));
	fnl->len_size = (int *)malloc(fnl->max_name_length * sizeof(int));

	for (i = 0; i < fnl->max_name_length; i++)
	{
		fnl->len_idx[i] = 0;
	}

	for (i = 0; i < fnl->size; i++)
	{
		fnl->len_size[strlen(fnl->file_name[i].name) - 1]++;
	}

	for (i = 1; i < fnl->max_name_length; i++)
	{
		fnl->len_idx[i] = fnl->len_size[i - 1] + fnl->len_idx[i - 1];
	}

	for (i = 0; i < fnl->max_name_length; i++)
	{
		//printf("%d %d %d\n", i + 1, fnl->len_idx[i], fnl->len_size[i]);
		qsort((void *)(&fnl->file_name[fnl->len_idx[i]]), fnl->len_size[i], sizeof(FILE_NAME), filename_string_comparefun);
	}
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
int filename_len_comparefun(const void *a, const void *b)
{
	const char *pa = ((FILE_NAME *)a)->name;
	const char *pb = ((FILE_NAME *)b)->name;
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
int filename_string_comparefun(const void *a, const void *b)
{
	const char *pa = ((FILE_NAME *)a)->name;
	const char *pb = ((FILE_NAME *)b)->name;

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
int stringcomparefun(const void *a, const void *b)
{
	const char *pa = (*(char **)a);
	const char *pb = (*(char **)b);
	return strcmp(pa, pb);
}