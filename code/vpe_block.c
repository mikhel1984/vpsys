/** @file vpe_block.c
~~~ Element Block ~~~
Functions for working with blocks */

/* ERROR FILE NUMBER - 12 */

#include <stdio.h>
#include <stdlib.h>

#include "vpe_block.h"
#include "vpb_ticket.h"
#include "vp_construct.h"
#include "vp_work.h"
#include "vpb_element.h"
#include "vpb_error.h"
#include "vpb_connectors.h"
#include "vpb_memory.h"

#define TMP_CONNECT_SIZE 256



/* Pairs of element ID for identification */
typedef struct idPair
{
    int src;
    int dst;
} Pairs;

Ticket* blockTicket = NULL;

/** Copy chain of elements and fill list of pairs
    @fn blkChainCopy
    @param src - graph to copy
    @param dst - where to copy
    @param p   - pointer to list of pairs
    @return successfull or failed */
Bool blkChainCopy(Graph* src, Graph* dst, Pairs** p);

/** Copy internal links
    @fn blkInternalCopy
    @param src - source graph
    @param p   - list of ID pairs
    @return successfull or failed */
Bool blkInternalCopy(Graph* src, Pairs* p);

/** Find index of second element in pair
    @fn getPair
    @param p   - list of pairs
    @param key - source ID
    @return dst ID */
int getPair(Pairs* p, int key);

/* Register block type */
Bool regBlock(void)
{
    int elt;

    elt = regElement("BLOCK",
        lngText("blk_tp", "Block is a container for other scheme elements"));

    if(elt == -1) return False;
    /* copy ticket to mark blocks */
    blockTicket = memGetTicket(elt);
    /* 1-st parameter - pointer to graph */
    regParameter(elt, NULL, NULL, dataFree(KEY_GRAPH, 0, 0), False);

    return True;
}

/* Add new block */
Element* blkCreate(char* name, Graph* base)
{
    Element* blk = NULL, *elt;
    Graph* set;
    Pairs* pTmp = NULL;
    Connector *arrCon[TMP_CONNECT_SIZE] = {NULL}, *cPtr, *groupPtr;
    int i, pos;
    u_char j, dir;
    char tmpName[8];

    /* registrate element */
    blk = eltCreate("BLOCK", name);
    if(!blk) return NULL;

    /* create graph */
    set = newGraph(NULL, base->nm->name);
    /* copy elements of chain and fill list of pairs */
    if(!blkChainCopy(base, set, &pTmp)) return NULL;
    /* set pointer to element */
    _DP(blk, 0)->freeVal.ptr = (void*) set;
    /* copy internal links */
    if(!blkInternalCopy(base, pTmp)) return NULL;

    /* create external connectors */
    for(dir = C_IN; dir <= C_OUT ; dir ++) {
        pos = 0;
        groupPtr = NULL;
        /* find free connectors */
        for(i = 0; pTmp[i].dst != -1; i ++) {
            elt = memGetElement(pTmp[i].dst);
            /* for every connector */
            for(j = 0; j < elt->out[dir].size; j ++) {
                cPtr = conGetBegin(_ptrC(elt, dir, j));
                if(cPtr->elt.pin)
                    continue;
                else
                    /* add to array */
                    arrCon[pos++] = _ptrC(elt, dir, j);
        } }
        /* create connector group */
        if(pos > 0) {
            _CRITICAL(groupPtr = (Connector*) malloc(sizeof(Connector) * pos))
            /* fill */
            for(i = 0; i < pos; i ++) {
                /* prepare name */
                sprintf(tmpName, "%s%d", dir == C_IN ? "in" : "out", i);
                groupPtr[i].blk.name = memName(tmpName);
                groupPtr[i].blk.next = arrCon[i];
        } }
        /* add to block */
        blk->out[dir].size = pos;
        blk->out[dir].group = groupPtr;
    }

    IFFREE(pTmp);

    return blk;
}

/* Copy chain of elements */
/* ERROR FUNCTION NUMBER - 3 */
Bool blkChainCopy(Graph* src, Graph* dst, Pairs** p)
{
    Chain* ptr;
    Element* elt;
    Pairs* idLst;

    if(!src->base)
        return setError(E_WRONG_ARG, 12.03, NULL);
    /* get memory for list of pairs */
    _CRITICAL(idLst = (Pairs*) calloc(graphSize(src) + 1, sizeof(Pairs)))

    *p = idLst;
    /* copy elements to new graph */
    for(ptr = src->base; ptr; ptr = ptr->next, idLst++) {
        /* get element copy */
        if((elt = eltCopy(NULL, ptr->elt)) == NULL) return False;
        /* add to graph */
        if(!eltToGraph(dst, elt)) return False;
        /* copy id-s */
        idLst->src = ptr->elt->id;
        idLst->dst = elt->id;
    }
    /* terminate list */
    idLst->dst = idLst->src = -1;

    return True;
}

/* Copy internal links */
Bool blkInternalCopy(Graph* src, Pairs* p)
{
    int i = 0, jDst, iCon, pos, jCon;
    Element* srcElt, *iElt, *jElt;
    Connector *conPtr, *lnkConPtr;

    for(i = 0; p[i].src != -1; i ++) {
        /* for every element in graph check connection */
        srcElt = memGetElement(p[i].src);
        /* only output connectors */
        for(iCon = 0; iCon < (int) srcElt->out[C_OUT].size; iCon ++) {
            /* find begining (if it is also block) */
            conPtr = conGetBegin(srcElt->out[C_OUT].group + iCon);
            /* check connected elements, create links */
            pos = 0;
            while((lnkConPtr = getConnection(conPtr, pos)) != NULL) {
                /* continue if not in graph */
                if((jDst = getPair(p, lnkConPtr->master->id)) == -1)
                    continue;
                /* find linked connector index */
                jCon = conFindIndex(lnkConPtr, C_IN);
                /* create linkage */
                iElt = memGetElement(p[i].dst);
                jElt = memGetElement(jDst);
                if(!conSetConnection(iElt->out[C_OUT].group+iCon,
                                      jElt->out[C_IN].group+jCon))
                        return False;
                pos ++;
                /* what about input connectors order ? */
    } } }

    return True;
}

/* Find dst ID */
int getPair(Pairs* p, int key)
{
    while(p->src != key && p->src != -1) p ++;

    return p->dst;
}

/* Copy block of elements */
Element* blkCopy(char *name, Element* blkSrc)
{
    Graph* src = NULL;

    if(blkSrc->tick != blockTicket) return NULL;

    src = blkGraph(blkSrc);
    if(!src) return NULL;

    return blkCreate(name, src);
}

/* Get begining of internal chain */
Element* blkInternalFirst(Element* elt)
{
    Graph* gr = NULL;
    Element* e = elt;

    if(elt->tick == blockTicket) {
        gr = blkGraph(elt);
        e = gr->base->elt;
    }

    return e;
}
