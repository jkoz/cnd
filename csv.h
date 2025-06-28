#ifndef _CSV_H
#define _CSV_H

#define CSV_ERR_LONGLINE 0
#define CSV_ERR_NO_MEMORY 1

char *fread_csv_line(FILE *fp, int max_line_size, int *done, int *err);

#endif
