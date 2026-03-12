#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char English[104][4] = {{'A', '\0'}, /**0  Ａ**/ {'B', '\0'}, /**1  Ｂ**/ {'C', '\0'}, /**2  Ｃ**/ {'D', '\0'}, /**3  Ｄ**/ {'E', '\0'}, /**4  Ｅ**/ {'F', '\0'}, /**5  Ｆ**/ {'G', '\0'}, /**6  Ｇ**/ {'H', '\0'}, /**7  Ｈ**/ {'I', '\0'}, /**8  Ｉ**/ {'J', '\0'}, /**9  Ｊ**/ {'K', '\0'}, /**10  Ｋ**/ {'L', '\0'}, /**11  Ｌ**/ {'M', '\0'}, /**12  Ｍ**/ {'N', '\0'}, /**13  Ｎ**/ {'O', '\0'}, /**14  Ｏ**/ {'P', '\0'}, /**15  Ｐ**/ {'Q', '\0'}, /**16  Ｑ**/ {'R', '\0'}, /**17  Ｒ**/ {'S', '\0'}, /**18  Ｓ**/ {'T', '\0'}, /**19  Ｔ**/ {'U', '\0'}, /**20  Ｕ**/ {'V', '\0'}, /**21  Ｖ**/ {'W', '\0'}, /**22  Ｗ**/ {'X', '\0'}, /**23  Ｘ**/ {'Y', '\0'}, /**24  Ｙ**/ {'Z', '\0'}, /**25  Ｚ**/ {'a', '\0'}, /**26  ａ**/ {'b', '\0'}, /**27  ｂ**/ {'c', '\0'}, /**28  ｃ**/ {'d', '\0'}, /**29  ｄ**/ {'e', '\0'}, /**30  ｅ**/ {'f', '\0'}, /**31  ｆ**/ {'g', '\0'}, /**32  ｇ**/ {'h', '\0'}, /**33  ｈ**/ {'i', '\0'}, /**34  ｉ**/ {'j', '\0'}, /**35  ｊ**/ {'k', '\0'}, /**36  ｋ**/ {'l', '\0'}, /**37  ｌ**/ {'m', '\0'}, /**38  ｍ**/ {'n', '\0'}, /**39  ｎ**/ {'o', '\0'}, /**40  ｏ**/ {'p', '\0'}, /**41  ｐ**/ {'q', '\0'}, /**42  ｑ**/ {'r', '\0'}, /**43  ｒ**/ {'s', '\0'}, /**44  ｓ**/ {'t', '\0'}, /**45  ｔ**/ {'u', '\0'}, /**46  ｕ**/ {'v', '\0'}, /**47  ｖ**/ {'w', '\0'}, /**48  ｗ**/ {'x', '\0'}, /**49  ｘ**/ {'y', '\0'}, /**50  ｙ**/ {'z', '\0'}, /**51  ｚ**/ {-17, -68, -95, '\0'}, /**52  Ａ**/ {-17, -68, -94, '\0'}, /**53  Ｂ**/ {-17, -68, -93, '\0'}, /**54  Ｃ**/ {-17, -68, -92, '\0'}, /**55  Ｄ**/ {-17, -68, -91, '\0'}, /**56  Ｅ**/ {-17, -68, -90, '\0'}, /**57  Ｆ**/ {-17, -68, -89, '\0'}, /**58  Ｇ**/ {-17, -68, -88, '\0'}, /**59  Ｈ**/ {-17, -68, -87, '\0'}, /**60  Ｉ**/ {-17, -68, -86, '\0'}, /**61  Ｊ**/ {-17, -68, -85, '\0'}, /**62  Ｋ**/ {-17, -68, -84, '\0'}, /**63  Ｌ**/ {-17, -68, -83, '\0'}, /**64  Ｍ**/ {-17, -68, -82, '\0'}, /**65  Ｎ**/ {-17, -68, -81, '\0'}, /**66  Ｏ**/ {-17, -68, -80, '\0'}, /**67  Ｐ**/ {-17, -68, -79, '\0'}, /**68  Ｑ**/ {-17, -68, -78, '\0'}, /**69  Ｒ**/ {-17, -68, -77, '\0'}, /**70  Ｓ**/ {-17, -68, -76, '\0'}, /**71  Ｔ**/ {-17, -68, -75, '\0'}, /**72  Ｕ**/ {-17, -68, -74, '\0'}, /**73  Ｖ**/ {-17, -68, -73, '\0'}, /**74  Ｗ**/ {-17, -68, -72, '\0'}, /**75  Ｘ**/ {-17, -68, -71, '\0'}, /**76  Ｙ**/ {-17, -68, -70, '\0'}, /**77  Ｚ**/ {-17, -67, -127, '\0'}, /**78  ａ**/ {-17, -67, -126, '\0'}, /**79  ｂ**/ {-17, -67, -125, '\0'}, /**80  ｃ**/ {-17, -67, -124, '\0'}, /**81  ｄ**/ {-17, -67, -123, '\0'}, /**82  ｅ**/ {-17, -67, -122, '\0'}, /**83  ｆ**/ {-17, -67, -121, '\0'}, /**84  ｇ**/ {-17, -67, -120, '\0'}, /**85  ｈ**/ {-17, -67, -119, '\0'}, /**86  ｉ**/ {-17, -67, -118, '\0'}, /**87  ｊ**/ {-17, -67, -117, '\0'}, /**88  ｋ**/ {-17, -67, -116, '\0'}, /**89  ｌ**/ {-17, -67, -115, '\0'}, /**90  ｍ**/ {-17, -67, -114, '\0'}, /**91  ｎ**/ {-17, -67, -113, '\0'}, /**92  ｏ**/ {-17, -67, -112, '\0'}, /**93  ｐ**/ {-17, -67, -111, '\0'}, /**94  ｑ**/ {-17, -67, -110, '\0'}, /**95  ｒ**/ {-17, -67, -109, '\0'}, /**96  ｓ**/ {-17, -67, -108, '\0'}, /**97  ｔ**/ {-17, -67, -107, '\0'}, /**98  ｕ**/ {-17, -67, -106, '\0'}, /**99  ｖ**/ {-17, -67, -105, '\0'}, /**100  ｗ**/ {-17, -67, -104, '\0'}, /**101  ｘ**/ {-17, -67, -103, '\0'}, /**102  ｙ**/ {-17, -67, -102, '\0'}, /**103  ｚ**/};
char space_and_tab[3][4] = {{-29, -128, -128, '\0'}, /**473  　**/ {' ', '\0'}, {'\t', '\0'}};
char apostrophe[2][4] = {{-30, -128, -103, '\0'}, {"\'"}};
char Number[10][4] = {"０" /**0  ０**/, "１" /**1  １**/, "２" /**2  ２**/, "３" /**3  ３**/, "４" /**4  ４**/, "５" /**5  ５**/, "６" /**6  ６**/, "７" /**7  ７**/, "８" /**8  ８**/, "９" /**9  ９**/};
int isCJK_words(char *word);
int isspace_and_tab(char *word);
int isapostrophe(char *word);
unsigned long long int utf8word2decimal(char *word);
void normalize_English(char *word);
int isenglish(char *word);
char *fullwidth_num(char *word);
int isNUM(char *word);
int read_a_utf8_word(char *line, char *word, int *index);
int Retreat_a_utf8_word(char *line, int *index);
int compare(const void *a, const void *b);
int read_a_line(char **line, int *capacity_of_line, FILE *fin);
void run(char *input, char *output);
void output_a_line(FILE *fout, char *line);
int main(int argc, char **argv)
{

	if (argc != 3)
	{
		printf("Usage: excute.exe intput.txt output.txt.\n");
		exit(1);
	}
	run(argv[1], argv[2]);
	return 0;
}
void output_a_line(FILE *fout, char *line)
{
	int idx;
	char word[7];
	int eng_flag;

	for (idx = 0, eng_flag = 0; !read_a_utf8_word(line, word, &idx);)
	{
		// puts(word);
		if (isenglish(word) == 1)
		{
			normalize_English(word);
			if (eng_flag == 0)
			{
				fprintf(fout, "%s", word);
			}
			else
			{
				fprintf(fout, "%s", word);
			}
			eng_flag = 1;
		}
		else
		{
			if (eng_flag == 1)
			{
				if (isapostrophe(word) == 1)
				{
					fprintf(fout, "\'");
				}
				else
				{
					fprintf(fout, " ");
				}
			}
			if (isNUM(word) == 1)
			{
				fprintf(fout, "%s ", fullwidth_num(word));
			}

			eng_flag = 0;
			if (isCJK_words(word) == 1)
			{
				fprintf(fout, "%s ", word);
			}
		}
	}
	if (eng_flag == 1)
	{
		fprintf(fout, " ");
	}
	// fprintf(fout," ");
}
void run(char *input, char *output)
{
	FILE *fin, *fout;
	char *line = (char *)malloc(sizeof(char));
	int i, FILEEND, capacity_of_line = 0, sentence_start;

	if (!(fin = fopen(input, "r")))
	{
		printf("can not open file: %s\n", input);
		exit(1);
	}
	if (!(fout = fopen(output, "w")))
	{
		printf("can not open file: %s\n", output);
		exit(1);
	}

	for (sentence_start = 0, FILEEND = 0; FILEEND == 0;)
	{
		FILEEND = read_a_line(&line, &capacity_of_line, fin);
		if (line[0] != '\0')
		{
			output_a_line(fout, line);
		}
	}

	free(line);
	fclose(fin);
	fclose(fout);
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
int compare(const void *a, const void *b)
{
	char **a1 = (char **)a, **b1 = (char **)b;
	return (strcmp(a1[0], b1[0]));
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
int isenglish(char *word)
{
	int i;
	for (i = 0; i < 104; i++)
	{
		if (strcmp(word, English[i]) == 0)
		{
			return 1;
		}
	}
	return 0;
}
void normalize_English(char *word)
{
	if (strlen(word) == 1)
	{
		if (word[0] >= 97 && word[0] <= 122)
		{
			word[0] -= 32;
		}
	}
	else
	{
		if (word[2] >= -95 && word[2] <= -70)
		{
			word[0] = word[2] + 160;
			word[1] = '\0';
		}
		else
		{
			word[0] = word[2] + 192;
			word[1] = '\0';
		}
	}
}
int isNUM(char *word)
{
	if (word[0] >= 48 && word[0] <= 57)
	{
		return 1;
	}
	return 0;
}
char *fullwidth_num(char *word)
{
	int i = (word[0] - 48);
	return Number[i];
}
int isspace_and_tab(char *word)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		if (strcmp(word, space_and_tab[i]) == 0)
		{
			return 1;
		}
	}
	return 0;
}
int isapostrophe(char *word)
{
	int i;
	for (i = 0; i < 2; i++)
	{
		if (strcmp(word, apostrophe[i]) == 0)
		{
			return 1;
		}
	}
	return 0;
}
int isCJK_words(char *word)
{
	/*
		class									unicode		UTF-8				UTF-8_deciaml
		CJK_Radicals_Supplement					2E80-2EFF	e2ba80-e2bbbf		14858880-14859199
		CJK_Unified_Ideographs_Extension_A		3400-4DBF	e39080-e4b6bf		14913664-14988991‬
		CJK-Unified_Ideographs					4E00-9FFF	e4b880-e9bfbf		14989440-15318975
		CJK-Compatibility_Ideographs			F900-FAFF	efa480-efabbf		15705216-15707071
		CJK_Unified_Ideographs_Extension_B		20000-2A6DF	f0a08080-f0aa9b9f	4037050496-4037712799
		CJK_Unified_Ideographs_Extension_C		2A700-2B73F	f0aa9c80-f0ab9cbf	4037713024-4037778623
		CJK_Unified_Ideographs_Extension_D		2B740-2B81F	f0ab9d80-f0aba09f	4037778816-4037779615
		CJK_Unified_Ideographs_Extension_E		2B820-2CEAF	f0aba0a0-f0acbaaf	4037779616-4037851823
		CJK_Unified_Ideographs_Extension_F		2CEB0-2EBEF	f0acbab0-f0aeafaf	4037851824-4037980079
		CJK-Compatibility_Ideographs_Supplement	2F800-2FA1F	f0afa080-f0afa89f	4038041728-4038043807
	*/
	unsigned long long int utf8_value = 0;
	if (strlen(word) == 3)
	{
		utf8_value = utf8word2decimal(word);
		if ((utf8_value >= 14858880 && utf8_value <= 14859199) ||
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
			(utf8_value >= 4038041728 && utf8_value <= 4038043807))
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
