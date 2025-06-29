#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "can.h"

#define BUFSIZ 1024
#define PRIORITY_MASK 0x1C000000 // 0001 1100 0000 0000 0000 0000 0000 0000
#define EDP_MASK 0x02000000      // 0000 0010 0000 0000 0000 0000 0000 0000
#define DP_MASK 0x01000000       // 0000 0001 0000 0000 0000 0000 0000 0000
#define PF_MASK 0x00ff0000       // 0000 0000 1111 1111 0000 0000 0000 0000
#define PS_MASK 0x0000ff00       // 0000 0000 0000 0000 1111 1111 0000 0000
#define PDU1_PGN_MASK 0x03ff0000 // 0000 0011 1111 1111 0000 0000 0000 0000
#define PDU2_PGN_MASK 0x03ffff00 // 0000 0011 1111 1111 1111 1111 0000 0000
#define SA_MASK 0x000000ff       // 0000 0000 0000 0000 0000 0000 1111 1111


can can_init(void) {
    can c = { 0 };
    c.size = 0;
    return c;
}

void can_decode_id(can *c) {
    c->pri = (PRIORITY_MASK & c->can_id) >> 26; // skip first 3 bits, 4-6 bits are priority
    c->edp = (EDP_MASK & c->can_id) >> 25;      // next 1 bit is extended data page
    c->dp = (DP_MASK & c->can_id) >> 24;        // next 1 bit is data page
    c->pf = (PF_MASK & c->can_id) >> 16;        // next 8 bit is PDU Format - protocol data unit format
    c->ps  = (PS_MASK & c->can_id) >> 8;        // next 8 bit is PDU Specific - protocol data unit specific
    c->sa  = (SA_MASK & c->can_id);             // last 8 bit is Source Address
                                                    //
	if (c->pf >= 0xf0) { // PDU2 broadcast, F0 is key to see if this is PDU2 format
		c->pgn = (PDU2_PGN_MASK & c->can_id) >> 8;
        c->da = 0xff; // Destination address
        c->ge = c->ps;
	} else { // PDU1 specific address, PGN does not include ps bits
		c->pgn = (PDU1_PGN_MASK & c->can_id) >> 8;
        c->da = c->ps; // Destination address
	}
}

void can_pprint(const can *self)
{
    // print can_id & data
    printf("%08x ", self->can_id);
    for (int i = 0; i < self->size; i++) {
        printf("%02X", self->can_data[i]);
    }
    /* printf("\nPriority: %u\nPGN: %u\nDA: %u\nSA: %u\n\n", self->pri, self->pgn, self->da, self->sa); */
    printf(" Priority: %u PGN: %u DA: %u SA: %u\n", self->pri, self->pgn, self->da, self->sa);
}

void can_setid(can *self, const unsigned int canid)
{
    self->can_id = canid;
}

void can_adddata(can *self, const char byte)
{
    self->can_data[self->size++] = byte; 
}
