/** @file vpb_ticket.c
~~~ Base Ticket ~~~
Functions for working with tickets */

/* ERROR FILE NUMBER - 8 */

#include <stdio.h>
#include <stdlib.h>

#include "vpb_ticket.h"
#include "vpb_error.h"
#include "vpb_connectors.h"
#include "vpb_memory.h"
#include "vpb_data.h"
#include "vpb_element.h"
#include "vp_construct.h"

/** Delete parameter ticket
    @fn ticParDelete
    @param tp - pointer to parameter ticket */
void ticParDelete(ticPar* tp);
/** Delete connector ticket
    @fn ticConDelete
    @param tc - pointer to connector ticket */
void ticConDelete(ticCon* tc);
/* set pointer when create type */
Bool defaultExec(Element* e, Number t, Number dt) { return False; }
/* Registration of new type */
int  regElement(const char *type, char *descript)
{
    /* ERROR: 01 */
    int pos;
    Ticket *ptr;

    if(!type) {
        setError(E_WRONG_ARG, 8.01, NULL);
        return -1;
    }
    /* check existing */
    pos = findTicket(type);
    if(pos != -1) {
        /* such type allready exist */
        setError(E_WTF, 8.01, "type exist");
        return -1;
    }
    /* get memory */
    if((pos = memAddTicket()) != -1) {
        /* get pointer */
        ptr = memGetTicket(pos);
        /* set */
        ptr->type = addName(type);
        ptr->u8_descript = descript;
        ptr->prep = defaultConnectorPrepare;
        ptr->exec = defaultExec;
        /* other set to 0 in memAddTicket() */
    }
    else
        return -1;

    return pos;
}

/* Add parameter */
int regParameter(int elt, const char *name, char* descript,
                  Data val, Bool isExternal)
{
    /* ERROR: 02 */
    Ticket* tick;
    ticPar* par;
    /* get pointer to main ticket */
    tick = memGetTicket(elt);
    if(tick) {
        /* new */
        par = (ticPar*) malloc(sizeof(ticPar));
        if(!par)  return setError(E_NO_MEMORY, 8.02, NULL);
        /* fill */
        par->next = NULL;
        /* set name to list of names */
        par->nm = name ? addName(name) : NULL;
        par->u8_descript = descript; /* only pointer! */
        par->isExternal = isExternal;
        /* copy data */
        par->dat = dataInt(0);  /* initialize */
        dataCopy(&(par->dat), val);
        /* add to end of list*/
        if(tick->lstPar)
            tick->parLast->next = par;
        else
            tick->lstPar = par;
        /* set pointer to last element and increase */
        tick->parLast = par;
    }
    else { return setError(E_WRONG_ARG, 8.02, NULL) - 1; }

    return (int) tick->parNum ++;
}

/* Add connector */
int regConnector(int elt, const char *name, char* descript,
                  int type, int dir)
{
    /* ERROR: 03 */
    Ticket* tick;
    ticCon* con, **start, **last;
    int ind = 0;

    /* get pointer to main ticket */
    tick = memGetTicket(elt);
    if(tick) {
        /* new */
        con = (ticCon*) malloc(sizeof(ticCon));
        if(!con) { return setError(E_NO_MEMORY, 8.03, NULL) - 1; }
        /* fill */
        con->next = NULL;
        con->nm = name ? addName(name) : NULL;
        con->u8_descript = descript;
        con->dat = type;
        /* get pointers */
        start = (dir == C_IN) ? &(tick->lstIn) : &(tick->lstOut);
        last  = (dir == C_IN) ? &(tick->inLast) : &(tick->outLast);
        ind   = (dir == C_IN) ? ++ tick->cinNum : ++ tick->coutNum;

        if(*start)
            (*last)->next = con;
        else
            *start = con;

        *last = con;
    }
    else { return setError(E_WRONG_ARG, 8.03, NULL) - 1; }

    return ind; /* or ind-1 ? */
}

/* Function for execution */
Bool regExec(int elt, Bool (*execf)(Element*, Number, Number))
{
    /* ERROR: 04 */
    Ticket* tick = memGetTicket(elt);
    /* set function */
    if(tick && execf) {
        tick->exec = execf;
    }
    else { return setError(E_WRONG_ARG, 8.04, NULL); }

    return True;
}

/* Preparing data function */
Bool regPrep(int elt, Bool (*prepf)(Element*, u_char))
{
    /* ERROR: 05 */
    Ticket* tick = memGetTicket(elt);

    if(tick && prepf) {
        tick->prep = prepf;
    }
    else { return setError(E_WRONG_ARG, 8.05, NULL); }

    return True;
}

/* Delete ticket data */
void ticketDelete(Ticket* t)
{
    ticCon *tc;
    ticPar *tp;

    if(t) {
        /* parameters */
        while(t->lstPar) {
            tp = t->lstPar;
            t->lstPar = tp->next;
            ticParDelete(tp);
        }
        /* input connectors */
        while(t->lstIn) {
            tc = t->lstIn;
            t->lstIn = tc->next;
            ticConDelete(tc);
        }
        /* output connectors */
        while(t->lstOut) {
            tc = t->lstOut;
            t->lstOut = tc->next;
            ticConDelete(tc);
        }
        /* ticket */
        free(t);
    }
}

/* Delete parameter ticket */
void ticParDelete(ticPar *tp)
{
    dataClear(&(tp->dat));

    free(tp);
}

/* Delete connector ticket */
void ticConDelete(ticCon *tc)
{
    free(tc);
}

int cPosFind(int elt, const char* name, u_char dir)
{
    int i = 0;
    unsigned int hs;
    Ticket *tick;
    ticCon *ptr;


    if((tick = memGetTicket(elt)) != NULL && name) {
        /* looking for */
        hs = stringHashUp(name);
        for(ptr = (dir == C_IN) ? tick->lstIn : tick->lstOut;
                                        ptr; ptr = ptr->next, i ++) {
            if(hs == ptr->nm->name_hash)
                return i;
    } }

    return -1;
}

/* Find input connector position */
int cinPos(int elt, const char *cinName)
{
    return cPosFind(elt, cinName, C_IN);
}

/* Find output connector position */
int coutPos(int elt, const char* coutName)
{
    return cPosFind(elt, coutName, C_OUT);
}

/* Find parameter position */
int parPos(int elt, const char* parName)
{
    int i = 0;
    Ticket *tick;
    ticPar *ptr;
    /* get ticket */
    tick = memGetTicket(elt);

    if(tick && parName) {
        /* find name */
        for(ptr = tick->lstPar; ptr; ptr = ptr->next, i ++) {
            if(ptr->nm && stringEqUp(parName, ptr->nm->name))
                return i;
    } }

    return -1;
}

ticPar* parameterFind(int elt, int pos)
{
    Ticket* tick;
    ticPar *ptr = NULL;
    /* get ticket */
    tick = memGetTicket(elt);
    /* find parameter */
    if(tick && pos >= 0) {
        ptr = tick->lstPar;
        while(pos-- > 0 && ptr) ptr = ptr->next;
    }

    return ptr;
}

/* Get pointer to data structure in ticket */
Data* getParameterData(int elt, int pos)
{
    ticPar *ptr = parameterFind(elt, pos);

    return ptr ? &(ptr->dat) : NULL;
}
/* Get parameter public flag */
Bool isParPablic(int elt, int pos)
{
    ticPar *ptr = parameterFind(elt, pos);

    return ptr ? ptr->isExternal : False;
}
