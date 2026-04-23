#include "whclib.h"

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
int stringcomparefun(const void *a, const void *b)
{
	const char *pa = (*(char **)a);
	const char *pb = (*(char **)b);
	return strcmp(pa, pb);
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
		printf("%s\n", line);
		printf("%d %d %d %d %d %d %d\n", line[(*index) - 6], line[(*index) - 5], line[(*index) - 4], line[(*index) - 3], line[(*index) - 2], line[(*index) - 1], line[(*index)]);
		fprintf(stdout, "Retreat error : not utf-8 format.\n");
		fflush(stdout);
		exit(1);
	}
	return 1;
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
			// printf("%d %d %d\n", times, (times - 1) * 1024 - times + 1,
			// *capacity_of_line);
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
void destroy_text(TEXT *text)
{
	int i;

	for (i = 0; i < text->num_of_sentence; i++)
	{
		free(text->sentence[i]);
	}
	free(text->sentence);
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
int read_a_utf8_word(char *line, char *word, int *index)
{
	unsigned char trick = 64;	// 0x01000000
	unsigned char trick1 = 128; // 0x10000000
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
int isCJK_words(char *word)
{
	//class										unicode		UTF-8				UTF-8_deciaml
	//CJK_Kangxi_Radicals						2F00-2FDF	e2bc80-e2bf9f		14859392-14860191
	//CJK_Radicals_Supplement					2E80-2EFF	e2ba80-e2bbbf		14858880-14859199
	//CJK_Unified_Ideographs_Extension_A		3400-4DBF	e39080-e4b6bf		14913664-14988991‬
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