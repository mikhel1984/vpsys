/** @file vpe_block.h
~~~ Element Block ~~~
Working with group of elements like with single element */

#ifndef VPE_BLOCK_H
#define VPE_BLOCK_H

#include "vpb_abstract.h"

#define blkGraph(elt) ((Graph*) _DP(elt,0)->freeVal.ptr)

/* ticket pointer for block type */
extern Ticket* blockTicket;

/** Register type 'block'
    @fn regBlock
    @return True if registered successfully */
Bool regBlock(void);
/** Create new block
    @fn blkCreate
    @param name - block name
    @param base - initial graph
    @return pointer to new block */
Element* blkCreate(char* name, Graph* base);
/** Create copy of block
    @fn blkCopy
    @param name - copy block name
    @param blkSrc - source block
    @return pointer to copy of block */
Element* blkCopy(char* name, Element *blkSrc);
/** Get firs element of internal graph
    @fn blkInternalFirst
    @param elt - block
    @return pointer to first element */
Element* blkInternalFirst(Element* elt);



#endif /* VPE_BLOCK_H */
