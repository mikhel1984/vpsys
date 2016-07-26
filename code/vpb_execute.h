/** @file vpb_execute.h
~~~ Base Execution ~~~
Execute functions from chain */

#ifndef VPB_EXECUTE_H
#define VPB_EXECUTE_H

#include "vpb_abstract.h"

/** Recursive function for chain creating
    @fn chainForExecution
    @param base - pointer to element base
    @param elt - one of elements
    @param controlElt - element for control avoiding loops
    @return True if chain created */
Bool chainForExecution(Chain **base, Chain** last, Element *elt, Element *controlElt);




#endif /* VPB_EXECUTE_H */
