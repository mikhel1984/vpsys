/** @file vpb_memory.c
~~~ Base Memory ~~~
Functions and pointers for working with memory */

/* ERROR FILE NUMBER - 6 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpb_memory.h"
#include "vpb_ticket.h"
#include "vpb_element.h"
#include "vpb_error.h"
#include "vpb_data.h"
#include "vpb_vars.h"

/* size for append arrays */
#define APPEND_TICKET_SIZE 16
#define APPEND_ELEMENT_SIZE 16
#define APPEND_GRAPH_SIZE 8
#define APPEND_VARS_SIZE 16
/* constant name max */

#define TMP_TEXT_BUFF 512

#define TMP_DATA_BUFF 48

/* > sort
struct tmpCompare
{
    Names *nm;
    void *rest;
};
*/
/* work with tickets */
Memory tickets = {0, 0, NULL};
/* work with elements */
Memory elements = {0, 0, NULL};
/* work with graphs */
Memory graphs = {0, 0, NULL};
/* work with variables */
Memory vars = {0, 0, NULL};
/* work with names */
Names* nameBase = NULL;
Names* descriptBase = NULL;

char tmpTxtBuff[TMP_TEXT_BUFF] = "";

Data tmpDataBuff[TMP_DATA_BUFF];

void* memAddNew(Memory *m, int *pos, int eltSize, int append);

/** Free ticket memory
    @fn memTicketFree */
void memTicketFree(void);
/** Free element memory
    @fn memElementFree */
void memElementFree(void);
/** Free graph memory
    @fn memGraphFree */
void memGraphFree(void);
/** Free names list
    @fn memNamesFree */
void memNamesFree(void);


void memVarFree(void);


int memAddVariable(char* nm);

//int memVarFind(char* nm);

Names* memAddName(const char* nm, u_char upper, Names** base);

void memSortName(Memory *m);

void memDataBuffFree(void);


/* Add new ticket */
int memAddTicket(void)
{
    Ticket *elt;
    TicketPtr *lst;
    int pos = -1;

    elt = (Ticket*) memAddNew(&tickets, &pos, sizeof(Ticket), APPEND_TICKET_SIZE);

    lst = (TicketPtr*) tickets.base;

    lst[pos] = elt;

    return pos;
}

/* Get pointer to ticket */
Ticket* memGetTicket(int pos)
{
    TicketPtr* lst = (TicketPtr*) tickets.base;

    return (pos >= 0 && pos < tickets.pos) ? lst[pos] : NULL;
}

/* Find ticket with given type */
int findTicket(const char* type)
{
    register int i;
    unsigned int hh;
    TicketPtr *lst = (TicketPtr*) tickets.base;

    hh = stringHashUp(type);

    for(i = 0; i < tickets.pos; i ++, lst ++) {
        if(*lst && (*lst)->type->name_hash == hh &&
            stringEqUp((*lst)->type->name, type)) return i;
    }

    return -1;
}

/* Free ticket memory */
void memTicketFree(void)
{
    TicketPtr* ptr = (TicketPtr*) tickets.base;

    while(tickets.pos --) {
        if(*ptr) ticketDelete(*ptr);
        ptr ++;
    }

    free(tickets.base);
}

/* Add new element */
int memAddElement(void)
{
    Element *elt;
    ElementPtr *lst;
    int pos = -1;

    elt = (Element*) memAddNew(&elements, &pos, sizeof(Element),
                        APPEND_ELEMENT_SIZE);

    lst = (ElementPtr*) elements.base;

    lst[pos] = elt;

    return pos;
}

/* Get pointer to element */
Element* memGetElement(int pos)
{
    ElementPtr *lst = (ElementPtr*) elements.base;

    return (pos >= 0 && pos < elements.pos) ? lst[pos] : NULL;
}

/* Clear element array */
void memElementFree(void)
{
    ElementPtr *lst = (ElementPtr*) elements.base;

    while(elements.pos --) {
        if(*lst) eltDelete(*lst);
        lst ++;
    }

    free(elements.base);
}

/* Free all memory */
void memFree(void)
{
    /* elements */
    memElementFree();
    /* tickets */
    memTicketFree();
    /* graphs */
    memGraphFree();
    /* names */
    memNamesFree();
    /* variables */
    memVarFree();
    /* data buffer */
    memDataBuffFree();
}

/* Add new graph */
Graph* memGetGraph(void)
{
    Graph* elt;
    GraphPtr *lst;
    int pos = -1;

    elt = (Graph*) memAddNew(&graphs, &pos, sizeof(Graph),
                                APPEND_GRAPH_SIZE);

    lst = (GraphPtr*) graphs.base;

    lst[pos] = elt;

    return elt;
}

/* Free graph memory */
void memGraphFree(void)
{
    Chain *ptr, *del;
    GraphPtr *lst = (GraphPtr*) graphs.base;

    while(graphs.pos --) {
        if(*lst) {
            /* clear chains */
            ptr = (*lst)->base;
            while(ptr) {
                del = ptr;
                ptr = ptr->next;
                free(del);
            }
            ptr = (*lst)->exec;
            while(ptr) {
                del = ptr;
                ptr = ptr->next;
                free(del);
            }
            /* clear graph */
            free(*lst);
        }
        lst ++;
    }

    free(graphs.base);
}

/* Add new name */
/* ERROR FUNCTION NUMBER - 1 */
char* memName(const char* nm)
{
    Names* ptr = memAddName(nm, True, &nameBase);
    return ptr ? ptr->name : NULL;
}

char* memDescript(const char* nm)
{
    Names* ptr = memAddName(nm, False, &descriptBase);
    return ptr ? ptr->name : NULL;
}

Names* addName(const char* name)
{
    return memAddName(name, True, &nameBase);
}

Names* memAddName(const char* nm, u_char upper, Names** base)
{
    Names *namePtr, *newName;
    unsigned int tmpHash;

    if(!nm) return NULL;

    /* find */
    tmpHash = stringHashUp(nm);
    namePtr = *base;
    while(namePtr) {
        if(namePtr->name_hash == tmpHash && stringEqUp(namePtr->name, nm))
            return namePtr;
        /* break if next is NULL - ? */
        if(namePtr->next)
            namePtr = namePtr->next;
        else
            break;
    }
    /* add */
    _CRITICAL(newName = (Names*) malloc(sizeof(Names)))
    /* set data */
    newName->name = upper ? stringCopyUp(nm, NULL) : stringCopy(nm, NULL);
    if(!newName->name)  {
        free(newName);
        return NULL;
    }

    newName->name_hash = tmpHash;
    newName->next = NULL;
    /* add to list */
    if(*base)
        namePtr->next = newName;
    else
        *base = newName;

    return newName;
}

/* Free names */
void memNamesFree(void)
{
    Names *ptr = nameBase, *del;
    /* names */
    while(ptr) {
        del = ptr;
        ptr = ptr->next;

        free(del->name);
        free(del);
    }
    /* descriptions */
    ptr = descriptBase;
    while(ptr) {
        del = ptr;
        ptr = ptr->next;

        free(del->name);
        free(del);
    }
}

/* Find graph with given name */
Graph* findGraph(const char* name)
{
    register int i;
    GraphPtr *lst = (GraphPtr*) graphs.base;

    for(i = 0; i < graphs.pos; i ++, lst++) {
        if(*lst && stringEqUp(name, (*lst)->nm->name))
            return *lst;
    }

    return NULL;
}

//void freeGraph(Graph* ptr) { return; } /* ??? */

Graph* memNextGraph(Graph* from)
{
    int i, get = 0;
    GraphPtr *lst = (GraphPtr*) graphs.base;

    if(!from) get = 1;

    for(i = 0; i < graphs.pos; i ++, lst ++) {
        if(*lst && (*lst)->nm) {
            if(get)
                return *lst;
            else if(*lst == from)
                get = 1;
    } }

    return NULL;
}


int variableAdd(char* nm)
{
    Var *elt;
    VarPtr* lst;
    int pos = -1;

    elt = (Var*) memAddNew(&vars, &pos, sizeof(Var), APPEND_VARS_SIZE);

    lst = (VarPtr*) vars.base;

    lst[pos] = elt;

    elt->nm = addName(nm);

    return pos;
}


void memVarFree(void)
{
    VarPtr *lst = (VarPtr*) vars.base;

    while(vars.pos --) {
        if(*lst) {
            if(!(*lst)->isPtr) {
                dataClear((*lst)->dat);
                free((*lst)->dat);
            }
            free(*lst);
        }
        lst ++;
    }

    free(vars.base);
}


void memEltDelete(Element *e)
{
    register int i;
    ElementPtr* lst = (ElementPtr*) elements.base;

    for(i = 0; i < elements.pos; i ++, lst++) {
        if(*lst == e) break;
    }

    if(*lst == e) {
        eltDelete(e);
        *lst = NULL;
    }
}

void* memAddNew(Memory* m, int *pos, int eltSize, int append)
{
    void *ptr = NULL;
    int aSize, eSize, i = 0;

    if(m->pos >= m->length) {
        ptr = m->base;
        for(i = 0; i < m->pos; i ++) {
            if(!ptr) break;
            ptr = (void*) ((char*) ptr + sizeof(void**));
        }

        if(i == m->pos) {
            aSize = sizeof(void**) * append;
            eSize = sizeof(void**) * m->length;

            _CRITICAL(ptr = realloc(m->base, eSize + aSize))
            m->base = ptr;

            ptr = (void*) ((char*) ptr + eSize);
            memset(ptr, 0, aSize);

            m->length += append;
    } }

    _CRITICAL(ptr = calloc(1, eltSize))

    *pos = i ? i : m->pos++;

    return ptr;
}

char* memTmpText(int sz)
{
    static int pos = 0;
    char *ptr = tmpTxtBuff;

    if(sz > (TMP_TEXT_BUFF >> 2)) return NULL; /* TMP_TEXT_BUFF/4 */

    if(TMP_TEXT_BUFF >= sz + pos) {
        ptr += pos;
        pos += sz;
    }
    else
        pos = 0;

    return ptr;
}
/* > sort
int memCompare(const void* a, const void *b)
{
    struct tmpCompare *ca = (struct tmpCompare*) a,
                        *cb = (struct tmpCompare*) b;

    if(!ca)
        return cb ? -1 : 0;
    else if(!cb)
        return 1;

    if(!ca->nm)
        return cb->nm ? -1 : 0;
    else if(!cb->nm)
        return 1;

    return strcmp(ca->nm->name, cb->nm->name);
}

void memSortName(Memory *m)
{
    qsort(m->base, m->pos, sizeof(struct tmpCompare *), memCompare);
}
*/
Data* getTmpData(u_char size)
{
    static u_char first = True;
    static u_char current = 0;
    int sz = size;
    Data* res = tmpDataBuff;

    if(size > (TMP_DATA_BUFF >> 2) || size == 0) return NULL;

    if(first) {
        memset(&tmpDataBuff, 0, sizeof(tmpDataBuff));
        first = False;
    }
    else {

        if(TMP_DATA_BUFF - current < size) current = 0;

        res = res + current;
        while(sz) {
            dataClear(res++);
            sz --;
        }

        res = tmpDataBuff + current;
    }

    current += size;

    return res;
}

void memDataBuffFree(void)
{
    register int i = TMP_DATA_BUFF;
    Data* d = tmpDataBuff;

    while(i) {
        dataClear(d++);
        i --;
    }
}
