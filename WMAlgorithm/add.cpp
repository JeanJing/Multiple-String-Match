#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "add.h"
void terminate(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr,format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}
char *read_text(char const *text_file, unsigned *text_len)
{
	FILE *fp_text;
   char *text;
	
	if ((fp_text = fopen(text_file, "rb")) == NULL)
		terminate("can not open text file!");

	fseek(fp_text, 0, SEEK_END);
	*text_len = ftell(fp_text);
	rewind(fp_text);

	if ((text = (char *)malloc((*text_len +1)*sizeof(char))) == NULL)
		terminate("无法为文本分 配内存!\n");
	if(fread(text, *text_len, 1, fp_text) != 1)
		fprintf(stderr, "Can not fread file: %s\n", text_file);;
	text[*text_len] = '\0';
	return text;
}