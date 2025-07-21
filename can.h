#ifndef _CAN_H
#define _CAN_H

#include "hashtable.h"

/*
 
    PDU: Protocol Data Unit
      0    C    F    0    0    4    E    E
    0000 1100 1111 0000 0000 0100 1110 1110
    
            0C             F0         04        EE
    000   011   0   0   11110000   00000100   11101110
     -   prio   R   DP    PF         PS         SA
    
    00001100111100000000010011101110
	
    3 bit priority
    1 bit Reserve or Extended Data Page
    1 bit Data Page
    8 bit PDU Format
    8 bit PDU Specific
    8 bit Source Address, SA
	 
*/

struct can{
    unsigned int can_id, 
           pri,  // Priority
           edp,  // Extended Data Page
           dp,   // Data Page
           pf,   // PDU Format
           ps,   // PDU Specific
           sa,   // Source Address
           da,
           pgn,
           ge;
    unsigned char can_data[BUFSIZ];
    int size; // number of byte in can_data
};


/* 
 *  Represent an entry of can logged from can device
 */
typedef struct can can;

/*  
 *  Initialize a entry of can, zero out the entry
 */ 
can can_init(void);

/*  
 * Parse can_id as j1939 specs 
 */ 
void can_decode_id(can *c);

/*  
 * Pretty print necessary can info
 */ 
void can_setid(can *self, const unsigned int canid);

/*  
 * 
 */ 
void can_adddata(can *self, const char byte);

/*  
 * sam - Source Address Map
 */ 
void can_print(const can *self, hashtable *sam);

#endif
