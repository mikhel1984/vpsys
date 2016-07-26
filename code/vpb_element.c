/** @file vpb_element.c
~~~ Base Element ~~~
Functions for working with element */

/* ERROR FILE NUMBER - 3 */

#include <stdio.h>
#include <stdlib.h>

#include "vpb_element.h"
#include "vpb_connectors.h"
#include "vpb_memory.h"
#include "vpb_ticket.h"
#include "vpb_data.h"
#include "vpb_error.h"
#include "vp_construct.h"
#include "vpe_block.h"

Element* eltPrepare(Ticket *t, const char* name, u_char copyParam)
{
    /* ERROR: 10 */
    Element* newElt = NULL;
    Connector* conPtr = NULL;
    Param *parPtr = NULL;
    ticCon* con;
    ticPar* par;
    int count, direct;

    if(!t) return (void*) setError(E_WRONG_ARG, 3.10, NULL);

    /* get memory for element */
    count = memAddElement();
    newElt = memGetElement(count);

    /* copy element name */
    if(name) newElt->nm = addName(name);
    /* set ticket */
    newElt->tick = t;
    newElt->id = count;
    /* set connectors */
    for(direct = C_IN; direct <= C_OUT; direct ++) {
        /* get connector number */
        count = (direct == C_IN) ? t->cinNum : t->coutNum;
        /* set size */
        newElt->out[direct].size = count;
        /* create connectors */
        if(count > 0) {
            _CRITICAL(conPtr = calloc(count, sizeof(Connector)))

            newElt->out[direct].group = conPtr;
            /* fill each connector in group */
            con = (direct == C_IN) ? t->lstIn : t->lstOut;
            for( ;con; con = con->next, conPtr++) {
                conPtr->ticket = con;
                conPtr->master = newElt;
                /* data */
                conPtr->elt.data = (Data*) malloc(sizeof(Data));
                /* what about C_IN connectors ? */
                conInitData(conPtr, direct);
    } } }

    /* get parameter number */
    count = t->parNum;
    /* set parameters */
    newElt->state.size = count;
    if(count > 0) {
        /* get memroy */
        _CRITICAL(parPtr = calloc(count, sizeof(Param)))
        newElt->state.group = parPtr;
        if(copyParam) {
            /* fill parameters */
            for(par = t->lstPar; par; par = par->next, parPtr++) {
                parPtr->ticket = par;
                /* calloc - other fill 0 */
                dataCopy(&(parPtr->dat), par->dat);
    } } }

    newElt->circle = 0;
    newElt->workTime = 0;

    return newElt;
}

/* Create element with given type */
Element* eltCreate(const char* type, const char* name)
{
    /* ERROR: 01 */
    /* find ticket */
    Ticket* tickPtr = memGetTicket(findTicket(type));
    if(!tickPtr)
        return (void*) setError(E_NOT_FOUND, 3.01, type);
    /* add new */
    return eltPrepare(tickPtr, name, True);
}


Element* eltCopy(char* name, Element* base)
{
    /* ERROR: 09 */
    Element *newElt = NULL;
    Param *from, *to;
    int i;

    if(!base)
        return (void*) setError(E_WRONG_ARG, 3.09, NULL);

    /* in case of block */
    if(base->tick == blockTicket)
        return blkCopy(name, base);
    /* create new */
    if((newElt = eltPrepare(base->tick, name, False)) == NULL)
        return NULL;
    /* copy parameters */
    from = base->state.group;
    to = newElt->state.group;

    for(i = 0; i < (int) base->state.size; i ++, from++, to++) {
        to->ticket = from->ticket;
        dataCopy(&(to->dat), from->dat);
    }

    return newElt;
}

/* Delete element data */
void eltDelete(Element* elt)
{
    int i, n;

    if(elt) {
        /* connectors */
        for(n = C_IN; n < C_OUT; n++) {
            if(elt->out[n].size > 0 && elt->out[n].group) {
                for(i = 0; i < elt->out[n].size; i ++) {
                    conClear(_ptrC(elt,n,i), n);
                }
                free(elt->out[n].group);
        } }
        /* parameters */
        if(elt->state.size > 0 && elt->state.group) {
            for(i = 0; i < elt->state.size; i++) {
                dataClear(_DP(elt,i));
            }
            free(elt->state.group);
        }
        /* delete element */
        free(elt);
    }
}

/* Add element to chain */
Bool addToChain(Chain **base, Chain **last, Element *e)
{
    /* ERROR: 02 */
    Chain *ptr;
    /* new part */
    _CRITICAL(ptr = (Chain*) malloc(sizeof(Chain)))

    ptr->elt = e;
    ptr->next = NULL;
    /* add to chain */
    if(*base)
        (*last)->next = ptr;
    else
        (*base) = ptr;

    *last = ptr;

    return True;
}

void chainClear(Chain **c)
{
    Chain* ptr = *c, *del;

    while(ptr) {
        del = ptr;
        ptr = ptr->next;
        free(del);
    }

    *c = NULL;
}

/* Try to find element in chaint */
Bool isInChain(Chain *base, Element *e)
{
    if(e) {
        while(base) {
            if(base->elt == e) return True;
            base = base->next;
    } }

    return False;
}
/*
int isInGraph(Graph* g, Element *e)
{
    Chain* ptr = g->base;
    int pos = 0;

    while(ptr) {
        if(ptr->elt == e) return pos;
        pos ++;
        ptr = ptr->next;
    }

    return -1;
}
*/
/*
Element* findInGraph(Graph* g, int pos)
{
    Chain *ptr = g->base;

    while(ptr && pos) {
        pos --;
        ptr = ptr->next;
    }

    return ptr ? ptr->elt : NULL;
}
*/
/* Get parameter data from name */
Data* eltParamData(Element *elt, const char* par)
{
    /* ERROR: 03 */
    int i;
    Param* ptr;

    if(!elt || !par)
        return (void*) setError(E_WRONG_ARG, 3.03, NULL);

    for(i = 0, ptr = elt->state.group; i < (int) elt->state.size; ptr++, i++) {
        /* compare name and possibility to modify */
        if(ptr->ticket->isExternal && ptr->ticket->nm
                                    &&stringEqUp(par, ptr->ticket->nm->name))
            return &(ptr->dat);
    }

    return NULL;
}

/* Get connector from name */
Connector* eltConnector(Element *elt, const char *con, int dir)
{
    /* ERROR: 04 */
    int i = 0;
    unsigned int hs;
    Connector *ptr;

    if(!elt)
        return (void*) setError(E_WRONG_ARG, 3.04, NULL);

    if(elt->out[dir].size == 0) return NULL;

    ptr = elt->out[dir].group;
    /* if no name, return first connector in group */
    if(!con) return ptr;

    hs = stringHashUp(con);
    /* looking for */
    while(i < (int) elt->out[dir].size) {
        if(ptr->ticket->nm && ptr->ticket->nm->name_hash == hs)
            return ptr;
        ptr ++; i ++;
    }

    return NULL;
}

/* Find data in element from it's name */
Data* getNameData(Element *elt, const char* var)
{
    Data *res = NULL;
    Connector *c = NULL;
    /* check parameters */
    if((res = eltParamData(elt, var)) != NULL)
        return res;
    /* check input connector */
    if((c = eltConnector(elt, var, C_IN)) != NULL)
        return c->elt.data;
    /* check output connector */
    if((c = eltConnector(elt, var, C_OUT)) != NULL)
        return c->elt.data;

    return NULL;
}

/* Get data from input connector */
Data* getInData(Element *elt, unsigned int pos)
{
    /* ERROR: 05 */
    if(!elt || pos >= elt->out[C_IN].size)
        return (void*) setError(E_WRONG_ARG, 3.05, NULL);

    return _DC(elt, C_IN, pos);
}

/* Get data from output connector */
Data* getOutData(Element *elt, unsigned int pos)
{
    /* ERROR: 06 */
    if(!elt || pos >= elt->out[C_OUT].size)
        return (void*) setError(E_WRONG_ARG, 3.6, NULL);

    return _DC(elt, C_OUT, pos);
}

/* Get data from parameter */
/* ERROR FUNCTION NUMBER - 7 */
Data *getParData(Element* elt, unsigned int pos)
{
    if(!elt || pos >= elt->state.size)
        return (void*) setError(E_WRONG_ARG, 3.07, NULL);

    return _DP(elt, pos);
}

/* Get number of input connectors */
int inNum(Element *elt)
{
    return elt ? (int) elt->out[C_IN].size : -1;
}

/* Get number of output connectors */
int outNum(Element *elt)
{
    return elt ? (int) elt->out[C_OUT].size : -1;
}

/* Get number of parameters */
int parNum(Element *elt)
{
    return elt ? (int) elt->state.size : -1;
}

Graph* newGraph(const char *name, const char *descr)
{
    Graph* ptr = memGetGraph();

    ptr->nm = addName(name);
    ptr->u8_descript = memDescript(descr);

    return ptr;
}


int eltToGraph(Graph *g, Element *e)
{
    /* ERROR: 08 */
    if(!g || !e) {
        setError(E_WRONG_ARG, 3.08, NULL);
        return 0;
    }

    return addToChain(&(g->base), &(g->baseLast), e);
}

int graphSize(Graph *g)
{
    Chain *ptr;
    register int size = 0;

    if(!g) return -1;

    ptr = g->base;

    while(ptr) {
        size ++;
        ptr = ptr->next;
    }

    return size;
}

int setParameter(Element *e, const char* param, Data val)
{
    Data *dat;

    if((dat = eltParamData(e, param)) == NULL)
        return 0;

    return dataCopy(dat, val);
}

