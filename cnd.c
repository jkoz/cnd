#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <csv.h>
#include "can.h"
#include "hashtable.h"

#define DA_SA_GENERIC "./j1939db/j1939da.csv.5"
#define DA_SA_HIGHWAY "./j1939db/j1939da.csv.9"
#define DA_PGN_SPN "./j1939db/j1939da.csv.2"

#define SP_LABEL "SP Label"
#define SP_LENGTH "SP Length"
#define PG_LABEL "PG Label"
#define PGN "PGN"
#define SOURCE_ADDRESS_ID "Source Address ID"
#define SA_NAME "Name"

    // <./j1939db/j1939da.csv.2 tail -n +4 | mlr --icsv --ojsonl filter 'is_not_empty($PGN)'
  /* "Revised": "", */
  /* "PG Revised": "", */
  /* "SP Revised": "", */
  /* "SP to PG Map Revised": "", */
  /* "PGN": 256, */
  /* "PG Label": "Transmission Control 1", */
  /* "PG Acronym": "TC1", */
  /* "PG Description": "Messages that are requesting control over the receiving device are transmitted at high rate only during the time when the control is active, but may be optionally sent at a slow rate as a \"heartbeat.\"", */
  /* "EDP": 0, */
  /* "DP": 0, */
  /* "PF": 1, */
  /* "PS": "DA", */
  /* "Multipacket": "No", */
  /* "Transmission Rate": "System dependent; either 50 ms as needed for active control, or as a continuous 50 ms periodic broadcast.", */
  /* "PG Data Length": 8, */
  /* "Default Priority": 3, */
  /* "PG Reference": "", */
  /* "SP Position in PG": 4.7, */
  /* "SPN": 688, */
  /* "SP Label": "Disengage Differential Lock Request - Rear Axle 2", */
  /* "SP Description": "Command signal used to disengage the various differential locks, e.g., to allow an undistributed individual wheel control by ABS.  The differential locks are located as defined in Figure SPN564_A.\n\n00b = Engage differential lock\n01b = Disengage differential lock\n10b = Reserved\n11b = Take no action", */
  /* "SP Length": "2 bits", */
  /* "Resolution": "4 states/2 bit", */
  /* "Offset": 0, */
  /* "Data Range": "0 to 3", */
  /* "Operational Range": "", */
  /* "Units": "bit", */
  /* "SLOT Identifier": 87, */
  /* "SLOT Name": "SAEbs02", */
  /* "SP Type": "Status", */
  /* "SP Reference": "See SPN 564 in 'Supporting Information' tab", */
  /* "SP Document": "J1939DA", */
  /* "PG Document": "J1939DA", */
  /* "SP Created or Modified Date": "2013/01/01", */
  /* "PG Created or Modified Date": "2017/08/10", */
 
void decode_stdin(hashtable *sam) {
    char buffer[BUFSIZ];
    char *line, *token;
    char *delims = " ";

    while (fgets(buffer, BUFSIZ, stdin) != NULL) {
        for (line = buffer; *line == ' '; line++); // skip leading space

        if (isdigit(*line)) { // record start with time digit
            can c = can_init();
            token = strtok(line, delims);
            for(int i = 0; token != NULL ; i++) {
                if (i == 0) { // 1261.543685
                    printf("%s ", token);
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
            can_print(&c, sam);
        }
    }
}

static inline void s_print(const void *item) {
    char *p;
    while ((p = strchr((char*) item, '"')) != NULL) *p = '*';
    while ((p = strchr((char*) item, '\n')) != NULL) *p = ' ';
    while ((p = strchr((char*) item, '\r')) != NULL) *p = ' ';
    printf("\"%s\"", (char*) item);
}

static inline void o_print(const void *item) {
    hashtable_print((hashtable*) item, s_print);
}

typedef struct da_parser {
    long unsigned fields;
    long unsigned rows;
    long unsigned title_row;
    long unsigned max;
    int titles_size;
    int* titles_id;
    char* titles_name[128];
    hashtable *map;
    hashtable *cur;
} da_parser;

/* All fields will be null terminated before calling this as per CSV_APPEND_NULL */
void cb1 (void *s, size_t len, void *data) {
    da_parser *d = (da_parser*)data;

    /* Load titles name from based on titles_row, titles_id , indexing from 0 to titles_size */ 
    if (d->rows == d->title_row) {
        for (int i = 0; i < d->titles_size; i++) {
            if (d->titles_id[i] == (int) d->fields) {
                d->titles_name[d->fields] = malloc(len + 1);
                strncpy(d->titles_name[d->fields], (char*) s, len + 1);
            }
        }
    }

    if (d->rows > d->title_row && d->rows < d->max) {
        if (d->titles_size < 2) { 
            fprintf(stderr, "Column titles number has to be greater than 1");
        }
        for (int i = 0; i < d->titles_size; i++) {
            if ((int) d->fields == d->titles_id[i]) {
                if (d->cur == NULL) {
                    d->cur = hashtable_create(1 << 8, free);
                }
                hashtable_add(d->cur, d->titles_name[d->titles_id[i]], strdup((char*) s), NULL, NULL, 2);
            }
        }
    }

    d->fields++;
}

void sa_cb2 (int c, void *data) {
    da_parser *d = (da_parser*)data;
    d->rows++;
    d->fields = 0;

    if (d->cur == NULL) {
        return;
    }

    char *sa = hashtable_get(d->cur, SOURCE_ADDRESS_ID);
    if (sa != NULL) {
        char *name = hashtable_get(d->map, sa);
        if (name == NULL) {
            name = hashtable_get(d->cur, SA_NAME);
            hashtable_add(d->map, sa, strdup(name), NULL, NULL, 1);
        } else {
            /* fprintf(stderr, "Duplicated SA ID %s: %s\n", sa, (char*) hashtable_get(d->cur, SA_NAME)); */
        }
    }
    hashtable_drop(d->cur);
    d->cur = NULL;
}

void cb2 (int c, void *data) {
    da_parser *d = (da_parser*)data;
    d->rows++;
    d->fields = 0;

    if (d->cur == NULL) {
        return;
    }

    char *pg = hashtable_get(d->cur, PGN);

    // check if current row has a valid pgn, not an empty string
    if (pg == NULL || strlen(pg) < 1) {
        goto ni;
    }

    // check if pgn is already in the pgn_map, if not create one
    hashtable *pgn = hashtable_get(d->map, pg);
    if (pgn == NULL) {
        pgn = hashtable_create(1 << 8, free);
        hashtable_add(d->map, pg, pgn, (void (*)(void *))hashtable_drop, (void (*)(void *))o_print, 1);
    }

    /* PGN Attributes */
    char *pg_label = hashtable_get(pgn, PG_LABEL);
    if (pg_label == NULL) {
        pg_label = hashtable_get(d->cur, PG_LABEL);
        if (pg_label != NULL) {
            hashtable_add(pgn, PG_LABEL, strdup(pg_label), free, (void (*)(void*))s_print, 1); 
        } 
    }

    /* SPN Attributes */

    // Not interest in any spn with empty sp length, don't care if it has SPN or not
    char *sp_length = hashtable_get(d->cur, SP_LENGTH);
    if (sp_length == NULL || strlen(sp_length) < 1) {
        goto ni;
    } 

    // check if current row has a valid spn, not an empty string
    char *sp = hashtable_get(d->cur, "SPN");
    if (sp != NULL && strlen(sp) > 0) {

        // check if SPNs container is already in pgn_map
        hashtable *spns = hashtable_get(pgn, "SPNs");
        if (spns == NULL) {
            spns = hashtable_create(1 << 8, (void (*)(void *))hashtable_drop); 
            hashtable_add(pgn, "SPNs", spns, (void (*)(void *))hashtable_drop, (void (*)(void *))o_print, 1);
        }

        // check if sp number is already in spns
        hashtable *spn = hashtable_get(spns, sp);
        if (spn != NULL) {
            fprintf(stderr, "Duplicated SPN: \"%s\" in PGN: %s\n ignore", sp, pg);
            goto ni;
        } 

        spn = hashtable_create(1 << 5, free);
        hashtable_add(spns, sp, spn, NULL, (void (*)(void *))o_print, 1);

        char *sp_label = hashtable_get(d->cur, SP_LABEL);
        if (sp_label != NULL) {
            hashtable_add(spn, SP_LABEL, strdup(sp_label), NULL, NULL, 1); 
        } 

        hashtable_add(spn, SP_LENGTH, strdup(sp_length), NULL, NULL, 1);
    }

ni:
    hashtable_drop(d->cur);
    d->cur = NULL;
}

void load_sas(FILE *fp, hashtable *sa_map, long unsigned max)
{
    struct csv_parser p;
    char buf[1024];
    size_t bytes_read;

    int sa_titles[] = {
        1,// "Source Address ID"
        2 // "Name"
    };

    struct da_parser c = {
        .titles_size = sizeof(sa_titles) / sizeof(int),
        .fields = 0,
        .rows = 0,
        .title_row = 3,
        .titles_id = sa_titles,
        .map = sa_map,
        .cur = NULL,
        .max = max
    };

    if (csv_init(&p, CSV_APPEND_NULL) != 0) exit(EXIT_FAILURE);

    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0) {
        if (csv_parse(&p, buf, bytes_read, cb1, sa_cb2, &c) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n", csv_strerror(csv_error(&p)) );
            exit(EXIT_FAILURE);
        }
    }

    csv_fini(&p, cb1, sa_cb2, &c);
    csv_free(&p);
    for (int i = 0; i < c.titles_size; i ++) free(c.titles_name[c.titles_id[i]]);
}


void load_pgns(FILE *fp, hashtable *pgn_map, long unsigned max)
{
    struct csv_parser p;
    char buf[1024];
    size_t bytes_read;

    int sa_titles[] = {
        4,  // "PGN": "256"
        18, // "SPN": "688"
        5,  // "PG Label": "Transmission Control 1"
        17, // "SP Position in PG": "4.7"
        21, // "SP Length": "2 bits"
        22, // "Resolution": "4 states/2 bit",
        19, // "SP Label": "Disengage Differential Lock Request - Rear Axle 2"
        6,  // "PG Acronym": "TC1"
        23,
        24,
        26  // "Units": "bit"
  
    };

    struct da_parser c = {
        .titles_size = sizeof(sa_titles) / sizeof(int),
        .fields = 0,
        .rows = 0,
        .title_row = 3,
        .titles_id = sa_titles,
        .map = pgn_map,
        .cur = NULL,
        .max = max
    };

    if (csv_init(&p, CSV_APPEND_NULL) != 0) exit(EXIT_FAILURE);

    while ((bytes_read=fread(buf, 1, 1024, fp)) > 0) {
        if (csv_parse(&p, buf, bytes_read, cb1, cb2, &c) != bytes_read) {
            fprintf(stderr, "Error while parsing file: %s\n",
                    csv_strerror(csv_error(&p)) );
            exit(EXIT_FAILURE);
        }
    }

    csv_fini(&p, cb1, cb2, &c);
    csv_free(&p);
    for (int i = 0; i < c.titles_size; i ++) free(c.titles_name[c.titles_id[i]]);
}


int main(int argc, char **argv, char **envp)
{
    /* Load PGNs SPNs */
    hashtable *pgn_map = hashtable_create(1 << 14, (void (*)(void*)) hashtable_drop);
    FILE *fp = fopen(DA_PGN_SPN, "rb");
    if (!fp) exit(EXIT_FAILURE);
    load_pgns(fp, pgn_map, 66810);
    fclose(fp);

    /* Load generic source address */
    hashtable *sa_map = hashtable_create(1 << 9, free);
	FILE *gf = fopen(DA_SA_GENERIC, "r");
    if (!gf) exit(EXIT_FAILURE);
    load_sas(gf, sa_map, 109);
    fclose(gf);
	FILE *hf = fopen(DA_SA_HIGHWAY, "r");
    if (!hf) exit(EXIT_FAILURE);
    load_sas(hf, sa_map, 76);
    fclose(hf);

    /* hashtable_print(pgn_map, o_print); */
    /* hashtable_print(sa_map, s_print); */

    hashtable_drop(sa_map);
    hashtable_drop(pgn_map);

    return 0;
}
