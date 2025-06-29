#ifndef _CSV_H
#define _CSV_H

#define CSV_ERR_LONGLINE 0
#define CSV_ERR_NO_MEMORY 1

char *fread_csv_line(FILE *fp, int max_line_size, int *done, int *err);

/*
 * Given a string which might contain unescaped newlines, split it up into
 * lines which do not contain unescaped newlines, returned as a
 * NULL-terminated array of malloc'd strings.
 */
char **csv_parse_unescaped(const char *txt);

void free_csv_line( char **parsed );

char **csv_parse( const char *line );

#endif
