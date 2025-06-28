#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "can.h"
#include "csv.h"
#include "ht.h"
#include "guards.h"

#define DA_SA "./j1939db/j1939da.csv.5"


void can_loadda(void) {
	FILE *f = fopen(DA_SA, "r");
    guards_oom(f, __FILE__, __LINE__);
	int error, done = 0;
	for (int i = 0; i < 10; i++) {
		char *rec = fread_csv_line(f, 65536, &done, &error);
        printf("%s\n------------------------\n", rec);
        free(rec);
	}
	fclose(f);
}

int main(void) {
    can_loadda();

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
            can_pprint(&c);
        }
        free(rec);
    }


    return 0;
}
