# Compile csv.c to module with flag c
# 	cc -Wall -Wextra -ansi -g -c csv.c
# 	cc -Wall -Wextra --std=c99 -c decode.c
# 	cc -Wall -Wextra --std=c99 -c ht.c


CC = cc
CFLAGS = -Wall -Wextra -g -std=c11 -pedantic 

SRC = cnd.c ht.c csv.c guards.c can.c
OBJ = ${SRC:.c=.o}

PREFIX = ${HOME}
PROG = cnd

all: ${PROG}

.c.o: 
	${CC} ${CFLAGS} -c $<

# $@ is the name of the target being generated: cnd
${PROG}: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o $@  

clean:
	rm -f ${PROG} ${OBJ}

install: all
	cp -f ${PROG} ${PREFIX}/bin
	chmod 755 ${PREFIX}/bin/${PROG}

.PHONY: all clean install uninstall
