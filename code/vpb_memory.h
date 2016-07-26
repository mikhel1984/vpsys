/** @file vpb_memory.h
~~~ Base Memory ~~~
Memory menegment */

#ifndef VPB_MEMORY_H
#define VPB_MEMORY_H

#include "vpb_abstract.h"
#include "vpb_number.h"

typedef struct vpMemory
{
    int pos;
    int length;
    void* base;
} Memory;

/* Pointer types */
typedef Ticket* TicketPtr;
typedef Element* ElementPtr;
typedef Graph* GraphPtr;
typedef Var* VarPtr;

/** Get memory for new ticket
    @fn memAddTicket
    @return ticket position */
int memAddTicket(void);
/** Get pointer to ticket
    @fn memGetTicket
    @param pos - ticket position
    @return ticket pointer */
Ticket* memGetTicket(int pos);
/** Find ticket with given type
    @fn findTicket
    @param type - type name
    @return ticket position */
int findTicket(const char* type);
/** Get memory for new element
    @fn memAddElement
    @return element position */
int memAddElement(void);
/** Get pointer to element
    @fn memGetElement
    @param pos - element position
    @return element pointer */
Element* memGetElement(int pos);
/** Get memory for new graph
    @fn memGetGraph
    @return graph pointer */
Graph* memGetGraph(void);
/** Find graph with given name
    @fn findGraph
    @return pointer to graph */
Graph* findGraph(const char* name);
/** Add name to list
    @fn memName
    @param nm - new name
    @return pointer to string */
char* memName(const char* nm);

char* memDescript(const char* nm);
/** Find next graph
    @fn memNextGraph
    @param from - current graph
    @return pointer to next graph */
Graph* memNextGraph(Graph* from);
/** Free graph memory
    @fn freeGraph
    @param ptr - pointer to graph */
//void freeGraph(Graph* ptr);
/** Create new constant/variable
    @fn memAddConst
    @param nm - constant name
    @param val - number value
    @param isConst - flag constant/variable
    @return pointer to number */
Number* memAddConst(char *nm, Number val, Bool isConst);
/** Find constant with given name
    @fn findConst
    @param nm - name ov number
    @param isConst - set flag about cons/var
    @return pointer to number */
Number* findConst(char *nm, u_char* isConst);


void memEltDelete(Element *e);




#endif /* VPB_MEMORY_H */
