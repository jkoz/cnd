#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

#define READ_BLOCK_SIZE 65536
#define QUICK_GETC( ch, fp )\
do\
{\
    if ( read_ptr == read_end ) {\
        fread_len = fread( read_buf, sizeof(char), READ_BLOCK_SIZE, fp );\
        if ( fread_len < READ_BLOCK_SIZE ) {\
            read_buf[fread_len] = '\0';\
        }\
        read_ptr = read_buf;\
    }\
    ch = *read_ptr++;\
}\
while(0)

char *fread_csv_line(FILE *fp, int max_line_size, int *done, int *err) {
    static FILE *bookmark;
    static char read_buf[READ_BLOCK_SIZE], *read_ptr, *read_end;
    static int fread_len, prev_max_line_size = -1;
    static char *buf;
    char *bptr, *limit;
    char ch;
    int fQuote;

    if ( max_line_size > prev_max_line_size ) {
        if ( prev_max_line_size != -1 ) {
            free( buf );
        }
        buf = malloc( max_line_size + 1 );
        if ( !buf ) {
            *err = CSV_ERR_NO_MEMORY;
            prev_max_line_size = -1;
            return NULL;
        }
        prev_max_line_size = max_line_size;
    }
    bptr = buf;
    limit = buf + max_line_size;

    if ( bookmark != fp ) {
        read_ptr = read_end = read_buf + READ_BLOCK_SIZE;
        bookmark = fp;
    }

    for ( fQuote = 0; ; ) {
        QUICK_GETC(ch, fp);

        if ( !ch || (ch == '\n' && !fQuote)) {
            break;
        }

        if ( bptr >= limit ) {
            free( buf );
            *err = CSV_ERR_LONGLINE;
            return NULL;
        }
        *bptr++ = ch;

        if ( fQuote ) {
            if ( ch == '\"' ) {
                QUICK_GETC(ch, fp);

                if ( ch != '\"' ) {
                    if ( !ch || ch == '\n' ) {
                        break;
                    }
                    fQuote = 0;
                }
                *bptr++ = ch;
            }
        } else if ( ch == '\"' ) {
            fQuote = 1;
        }
    }

    *done = !ch;
    *bptr = '\0';
    return strdup(buf);
}


char **csv_parse_unescaped(const char *txt)
{
    const char *ptr, *lineStart;
    char **buf, **bptr;
    int fQuote, nLines;

    /* First pass: count how many lines we will need */
    for ( nLines = 1, ptr = txt, fQuote = 0; *ptr; ptr++ ) {
        if ( fQuote ) {
            if ( *ptr == '\"' ) {
                fQuote = 0;
            }
        } else if ( *ptr == '\"' ) {
            fQuote = 1;
        } else if ( *ptr == '\n' ) {
            nLines++;
        }
    }

    buf = malloc( sizeof(char*) * (nLines+1) );

    if ( !buf ) {
        return NULL;
    }

    /* Second pass: populate results */
    lineStart = txt;
    for ( bptr = buf, ptr = txt, fQuote = 0; ; ptr++ ) {
        if ( fQuote ) {
            if ( *ptr == '\"' ) {
                fQuote = 0;
                continue;
            } else if ( *ptr ) {
                continue;
            }
        }

        if ( *ptr == '\"' ) {
            fQuote = 1;
        } else if ( *ptr == '\n' || !*ptr ) {
            size_t len = ptr - lineStart;

            if ( len == 0 ) {
                *bptr = NULL;
                return buf;
            }

            *bptr = malloc( len + 1 );

            if ( !*bptr ) {
                for ( bptr--; bptr >= buf; bptr-- ) {
                    free( *bptr );
                }
                free( buf );
                return NULL;
            }

            memcpy( *bptr, lineStart, len );
            (*bptr)[len] = '\0';

            if ( *ptr ) {
                lineStart = ptr + 1;
                bptr++;
            } else {
                bptr[1] = NULL;
                return buf;
            }
        }
    }
}

void free_csv_line( char **parsed )
{
    char **ptr;

    for ( ptr = parsed; *ptr; ptr++ ) {
        free( *ptr );
    }

    free( parsed );
}



static int count_fields( const char *line ) {
    const char *ptr;
    int cnt, fQuote;

    for ( cnt = 1, fQuote = 0, ptr = line; *ptr; ptr++ ) {
        if ( fQuote ) {
            if ( *ptr == '\"' ) {
                fQuote = 0;
            }
            continue;
        }

        switch( *ptr ) {
            case '\"':
                fQuote = 1;
                continue;
            case ',':
                cnt++;
                continue;
            default:
                continue;
        }
    }

    if ( fQuote ) {
        return -1;
    }

    return cnt;
}

/*
 *  Given a string containing no linebreaks, or containing line breaks
 *  which are escaped by "double quotes", extract a NULL-terminated
 *  array of strings, one for every cell in the row.
 */
char **csv_parse( const char *line )
{
    char **buf, **bptr, *tmp, *tptr;
    const char *ptr;
    int fieldcnt, fQuote, fEnd;

    fieldcnt = count_fields( line );

    if ( fieldcnt == -1 ) {
        return NULL;
    }

    buf = malloc( sizeof(char*) * (fieldcnt+1) );

    if ( !buf ) {
        return NULL;
    }

    tmp = malloc( strlen(line) + 1 );

    if ( !tmp ) {
        free( buf );
        return NULL;
    }

    bptr = buf;

    for ( ptr = line, fQuote = 0, *tmp = '\0', tptr = tmp, fEnd = 0; ; ptr++ ) {
        if ( fQuote ) {
            if ( !*ptr ) {
                break;
            }

            if ( *ptr == '\"' ) {
                if ( ptr[1] == '\"' ) {
                    *tptr++ = '\"';
                    ptr++;
                    continue;
                }
                fQuote = 0;
            }
            else {
                *tptr++ = *ptr;
            }

            continue;
        }

        switch( *ptr ) {
            case '\"':
                fQuote = 1;
                continue;
            case '\0':
                fEnd = 1;
            case ',':
                *tptr = '\0';
                *bptr = strdup( tmp );

                if ( !*bptr ) {
                    for ( bptr--; bptr >= buf; bptr-- ) {
                        free( *bptr );
                    }
                    free( buf );
                    free( tmp );

                    return NULL;
                }

                bptr++;
                tptr = tmp;

                if ( fEnd ) {
                    break;
                } else {
                    continue;
                }

            default:
                *tptr++ = *ptr;
                continue;
        }

        if ( fEnd ) {
            break;
        }
    }

    *bptr = NULL;
    free( tmp );
    return buf;
}
