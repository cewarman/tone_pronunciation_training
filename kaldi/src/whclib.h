#ifndef _WHCLIB_
#define _WHCLIB_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _TEXT_
{
	int num_of_sentence;
	char **sentence;
} TEXT;

int bin_string_search(char *sentence, char **set, int set_size);
int string_len_comparefun(const void *a, const void *b);
int stringcomparefun(const void *a, const void *b);
int read_a_utf8_word(char *line, char *word, int *index);
int utf8_word_length(char *word);
void destroy_text(TEXT *text);
void load_text(FILE *fin, TEXT *text_in);
int read_a_line(char **line, int *capacity_of_line, FILE *fin);
int Retreat_a_utf8_word(char *line, int *index);
int isCJK_words(char *word);
unsigned long long int utf8word2decimal(char *word);

#endif