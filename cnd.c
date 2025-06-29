#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "can.h"
#include "csv.h"
#include "ht.h"
#include "guards.h"

#define DA_SA_GENERIC "./j1939db/j1939da.csv.5"
#define DA_SA_HIGHWAY "./j1939db/j1939da.csv.9"

void can_loadda(FILE *f, ht *m, int skip, int size) {
    guards_oom(f, __FILE__, __LINE__);
	int error, done = 0;
	for (int i = 0; i < size; i++) {
		char *rec = fread_csv_line(f, 65536, &done, &error);
        guards_oom(rec, __FILE__, __LINE__);
	 	char **fields = csv_parse(rec);
        guards_oom(fields, __FILE__, __LINE__);

        if ( i > skip) {
            // strdup fields[2] as its value will be cleaned in destroy of ht
            ht_set(m, fields[1], strdup(fields[2]));			
        }
		free_csv_line(fields);
        free(rec);
	}
}

char* ts(void *data) 
{
    return data;
}

/* int main(int argc, char **argv, char **envp) */
int main(void) {
    // libc free() is passed to free value
	ht *m = ht_init(256, free); 
    guards_oom(m, __FILE__, __LINE__);

	FILE *f = fopen(DA_SA_GENERIC, "r");
    can_loadda(f, m, 3, 109);

	FILE *f1 = fopen(DA_SA_HIGHWAY, "r");
    can_loadda(f1, m, 3, 76);

    /* ht_print(m, ts); */

    char buffer[BUFSIZ];
    char *line, *token;

    char *delims = " ";

    while (fgets(buffer, BUFSIZ, stdin) != NULL) {
        char *rec = malloc(strlen(buffer));
        strncpy(rec, buffer, strlen(buffer));

        for (line = rec; *line == ' '; line++); // skip leading space

        if (isdigit(*line)) { // record start with time digit

            can c = can_init();
            token = strtok(line, delims);
            for(int i = 0; token != NULL ; i++) {
                if (i == 0) { // 1261.543685
                } else if (i == 1) { // 1
                } else if (i == 2) { // 18FEF200x
                    can_setid(&c, (unsigned int) strtol(token, NULL, 16));
                } else if (i == 3) { // Rx
                } else if (i == 4) { // d
                } else if (i == 5) { // 8
                } else {
                    can_adddata(&c, (unsigned char) strtol(token, NULL, 16));
                }
                token = strtok(NULL, delims);
            }
            can_decode_id(&c);

            int l = snprintf(NULL, 0, "%d", c.sa);
            char sa[l+1];
            snprintf(sa, l + 1, "%d", c.sa);
            
            void *v = ht_get(m, sa);
            if (v == NULL) { // Source address is not found in DA file, set to Reserved
                printf("%d %s\n", c.sa, "Reserved");
            } else {
                printf("%d %s\n", c.sa, (char *) v);
            }
            /* can_pprint(&c); */
        }
        free(rec);
    }
	
	ht_destroy(m);
    fclose(f1);
    fclose(f);

    return 0;
}
