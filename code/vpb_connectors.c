/** @file vpb_connectors.c
~~~ Base Connectors ~~~
Functions for working with connectors */

/* ERROR FILE NUMBER - 1 */

#include <stdio.h>
#include <stdlib.h>

#include "vpb_connectors.h"
#include "vpb_data.h"
#include "vpb_error.h"
#include "vpb_element.h"


/** Add new linking structure
    @fn lnkAdd
    @param base - pointer to connector
    @return pointer to new link structure */
Link* lnkAdd(Connector *base);
/** Linking two connectors
    @fn linking
    @param a - first link structure
    @param b - second link structure
    @return True if linked successfully */
Bool linking(Link* a, Link* b);
/** Check possibility of connection
    @fn conIsEquial
    @param c_from - src connector
    @param c_to - dst connector
    @return True if possible to connect */
Bool conIsEquial(Connector *c_from, Connector *c_to);

void conRemoveSecond(Link* l);

/* Add link structure */
/* ERROR FUNCTION NUMBER - 1 */
Link* lnkAdd(Connector *base)
{
    Link *ptr = base->elt.pin;
    Link* newLink = (Link*) malloc(sizeof(Link));

    if(newLink) {
        /* printf("C: %s\n", base->master->name); */
        /* set values */
        newLink->base = base;
        newLink->lnk  = NULL;
        newLink->next = NULL;
        /* set to end */
        if(ptr) {
            while(ptr->next) ptr = ptr->next;
            ptr->next = newLink;
        }
        else
            base->elt.pin = newLink;
    }
    else { setError(E_NO_MEMORY, 1.01, NULL); }

    return newLink;
}

/* Linking two elements */
/* ERROR FUNCTION NUMBER - 2 */
Bool linking(Link *a, Link *b)
{
    if(a && b) {
        a->lnk = b;
        b->lnk = a;
    }
    else
        return setError(E_WRONG_ARG, 1.02, NULL);

    return True;
}

/* Find first connector */
/* ERROR FUNCTION NUMBER - 3 */
Connector* conGetBegin(Connector *from)
{
    Connector *ptr = from;

    if(ptr) {
        /* in block *ticket == NULL */
        while(!ptr->ticket)  ptr = ptr->blk.next;
    }
    else setError(E_WRONG_ARG, 1.03, NULL);

    return ptr;
}

/* Check possibility to set connection */
Bool conIsEquial(Connector *c_from, Connector *c_to)
{
    /* ??? */
    if(c_from->ticket->dat == c_to->ticket->dat)
        /* equial type of size */
        return True;
    else if(c_to->ticket->dat == DAT_ANYTYPE)
        /* dst connector type not defined */
        return True;
    else if(c_from->ticket->dat >= 0 && c_to->ticket->dat >= 0)
        /* src is array or number and dst size not defined */
        return True;

    return False;
}

/* Connect two elements */
/* ERROR FUNCTION NUMBER - 4 */
Bool conSetConnection(Connector *from, Connector *to)
{
    int i = 0;
    Connector *ptr;
    /* check arguments */
    if(!from || !to)
        return setError(E_WRONG_ARG, 1.04, NULL);

    /* find begining */
    from = conGetBegin(from);
    to   = conGetBegin(to);

    /* check direction - ?*/
    //if(from->ticket->direction == to->ticket->direction)
    //    return setError(E_WRONG_ARG, 1.04, NULL);

    /* check connection possibility */
    if(!conIsEquial(from, to))
        return setError(E_DIFF_TYPE, 1.04, NULL);
    /* check if allready linking */
    while((ptr = getConnection(to, i++)) != NULL) {
        if(ptr == from) return True;
    }
    /* link elements */
    if(!linking(lnkAdd(from),lnkAdd(to)))
        return setError(E_NO_MEMORY, 1.04, NULL);

    return True;
}
/* ERROR FUNCTION NUMBER - 7 */
Bool conDisconnect(Connector* from, Connector *to)
{
    u_char find = False;
    Link *lnkToBase = NULL, *lnkPtr;

    if(!from || !to)
        return setError(E_WRONG_ARG, 1.07, NULL);

    from = conGetBegin(from);
    to = conGetBegin(to);
    /* check if connected, get second link */
    lnkPtr = to->elt.pin;
    while(lnkPtr) {
        if(lnkPtr->lnk->base == from) {
            find = True;
            break;
        }
        lnkToBase = lnkPtr;
        lnkPtr = lnkPtr->next;
    }

    if(!find) return True; /* if not connected, exit finction */

    if(lnkToBase)
        lnkToBase->next = lnkPtr->next;
    else
        to->elt.pin = lnkPtr->next;

    conRemoveSecond(lnkPtr);
    free(lnkPtr);

    return True;
}

int disconnectAll(Element *e)
{
    int dir, i;
    Connector *eCon = NULL;
    Link *lnk;

    if(!e) return False;

    for(dir = 0; dir <= 1; dir ++) {
        for(i = 0; i < e->out[dir].size; i ++) {
            eCon = conGetBegin(e->out[dir].group + i);

            while(eCon->elt.pin) {
                lnk = eCon->elt.pin;
                eCon->elt.pin = lnk->next;

                conRemoveSecond(lnk);
                free(lnk);
    } } }

    return True;
}

/* Clear connector data */
void conClear(Connector *connect, u_char dir)
{
    Link* lnkPtr = NULL;
    /* element connector */
    if(connect->ticket) {

        if(dir == C_IN) {
            if(connect->elt.pointer) {
                IFFREE(connect->elt.data); }
            else {
                dataClear(connect->elt.data);
                free(connect->elt.data);
                //dataDelete(connect->elt.data);
            }
        }
        else {
            /* if get data, delete it */
            if(connect->elt.data) {
                dataClear(connect->elt.data);
                free(connect->elt.data);
                //dataDelete(connect->elt.data);
            }
        }
        /* delete link structures */
        while(connect->elt.pin) {
            lnkPtr = connect->elt.pin;
            connect->elt.pin = connect->elt.pin->next;
            free(lnkPtr);
        }
    }
    /* block connector */
    else {
        IFFREE(connect->blk.name);
    }
}

/* Get connector of linked element */
/* ERROR FUNCTION NUMBER - 5 */
Connector* getConnection(Connector* base, int pos)
{
    Link* ptr;
    Connector* res = NULL;

    if(base && pos >= 0) {
        /* find link with given position */
        ptr = base->elt.pin;
        while(ptr && pos--) ptr = ptr->next;
        /* pointer not null */
        if(ptr) {
            if(ptr->lnk)
                res = ptr->lnk->base;
            else /* link structure have to exist only when elements are connected */
                setError(E_WTF, 1.05, "no linking");
        } }
    else setError(E_WRONG_ARG, 1.05, NULL);

    return res;
}

Element* getConnectedElt(Connector* c, int pos)
{
    Connector* ans = getConnection(c, pos);
    return ans ? ans->master : NULL;
}

/* initialise data */
/* ERROR FUNCTION NUMBER - 6 */
Bool conInitData(Connector *c, u_char dir)
{
    if(!c) return setError(E_WRONG_ARG, 1.06, NULL);

    c->elt.pointer = True;

    switch(c->ticket->dat) {
    case DAT_ANYTYPE: /* does it realy need? */
        *(c->elt.data) = dataFree(0, 1, 0);
        break;
    case DAT_SIGNAL:
        *(c->elt.data) = dataBool(True);
        break;
    case DAT_STRING:
        *(c->elt.data) = regDataText(NULL);
        break;
    default:
        if(dir == C_OUT)
            /* array with given size */
            return dataInitArray(c->elt.data, c->ticket->dat);
        else
            *(c->elt.data) = dataArrayNull();
    }

    return True;
}

Bool defaultConnectorPrepare(Element *elt, u_char onlyPointer)
{
    Data *thisData, *opositData;
    Connector *cPtr;
    int i;

    for(i = 0; i < (int) elt->out[C_IN].size; i ++) {
        thisData = _DC(elt, C_IN, i);
        cPtr = getConnection(elt->out[C_IN].group + i, 0);
        if(cPtr) {
            opositData = cPtr->elt.data;

            if(!thisData || !opositData) return False;
            if(onlyPointer)
                *thisData = *opositData;
            else if(!dataCopy(thisData, *opositData))
                return False;

            _ptrC(elt, C_IN, i)->elt.pointer = onlyPointer;
        }
        else
            return False;
    }
    return True;
}

char* conDataType(int marker)
{
    char *res;

    switch(marker) {
    case DAT_ANYTYPE:
        res = "any type"; break;
    case DAT_SIGNAL:
        res = "signal"; break;
    case DAT_STRING:
        res = "text"; break;
    default:
        res = "array"; break;
    }

    return res;
}

void conRemoveSecond(Link* l)
{
    Connector *cPtr = NULL;
    Link* from = NULL, *lPtr;

    if(/* !l ||*/ !l->lnk) return;

    cPtr = l->lnk->base;

    lPtr = cPtr->elt.pin;
    while(lPtr) {
        if(lPtr->lnk == l)
            break;
        from = lPtr;
        lPtr = lPtr->next;
    }

    if(from) {
        lPtr = from->next;
        from->next = lPtr->next;
    }
    else {
        lPtr = cPtr->elt.pin;
        cPtr->elt.pin = lPtr->next;
    }

    free(lPtr);
}

int connect(Element *e1, char *c1, Element *e2, char *c2)
{
    Connector *from, *to;

    from = eltConnector(e1, c1, C_OUT);
    to = eltConnector(e2, c2, C_IN);

    return conSetConnection(from, to);
}

int disconnect(Element *e1, char *c1, Element *e2, char *c2)
{
    Connector *from, *to;

    from = eltConnector(e1, c1, C_OUT);
    to = eltConnector(e2, c2, C_IN);

    return conDisconnect(from, to);
}

int conFindIndex(Connector* c, u_char dir)
{
    int i;
    Connector *ptr = c->master->out[dir].group;

    for(i = 0; i < (int) c->master->out[dir].size; i ++, ptr++) {
        if(ptr == c) return i;
    }

    return -1;
}
