/** @file vpb_execute.c
~~~ Base Execution ~~~
Work with chain */

/* ERROR FILE NUMBER - 4 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vpb_execute.h"
#include "vpb_error.h"
#include "vpb_element.h"
#include "vpb_connectors.h"
#include "vp_work.h"
#include "vpb_memory.h"
#include "vpb_ticket.h"
#include "vpe_block.h"

#define MAX_STREAM_NUM   10          /* maximun number of threads */
/* simplify execution and preparing code */
#define EXEC(X,T,P) ((X)->tick->exec(X,T,P))
#define PREP(X,F)   ((X)->tick->prep(X,F))

/* signal from elements */
u_char haltEngine = False;
/* desired number of threads */
u_char threadNumber = 1;

/* for future multitread realisation */
/** @struct
    Parts of chain in threads */
struct
{
    Chain *part;              /**< Fragment of chain */
    int   count;              /**< Iteration number */

} threadChain[MAX_STREAM_NUM];


/** Preparing chain for evaluating in threads
    @fn chainBreak
    @param c - pointer to chain
    @param period - time period for breaking */
void chainBreak(Chain *c, int period);
/** Set thread number (if program can do it in this OS)
    @fn setThreads
    @param n - desired number of threads */
void setThreads(u_char n);

/* Default time step */
double tstep = 0.001;
/* dt = 0 - first iteration !!! */
/* time in engine */
Number tau;

/* Set right sequence for processing */
Bool chainForExecution(Chain **base, Chain** last, Element *elt, Element *controlElt)
{
    int i, k;
    Element *ptr = NULL;
    Connector *conPtr = NULL;
    /* in case of block */
    elt = blkInternalFirst(elt);
    controlElt = blkInternalFirst(controlElt);

    /* for all input connectors of this element */
    for(i = 0; i < (int) elt->out[C_IN].size; i ++) {
        k = 0; /* number of link for this connector */
        while ((conPtr = getConnection(elt->out[C_IN].group + i, k ++)) != NULL) {
            /* base element */
            ptr = conPtr->master;
            /* check list of elements */
            if(!isInChain(*base, ptr)) {
                /* if not in list, go here */
                if(ptr != controlElt)
                    return chainForExecution(base, last, ptr, controlElt);
    } } }

    /* add element */
    addToChain(base, last, elt);
    /* debug
    printf("%s  ", elt->name); */

    /* for all output connectors */
    for(i = 0; i < (int) elt->out[C_OUT].size; i++) {
        k = 0; /* link number */
        while((conPtr = getConnection(elt->out[C_OUT].group + i, k++)) != NULL) {
            /* base element */
            ptr = conPtr->master;
            /* if not in list */
            if(!isInChain(*base, ptr)) {
                /* return if false */
                if(chainForExecution(base, last, ptr, ptr) != True)
                    return False;
    } } }

    return True;
}

/* Single elecution */
int singleExecute(Chain *c, Number t, Number dt)
{
    /* ERROR: 01 */
    u_char flag = 0;
    clock_t dur, sum = 0;

    while(c && !haltEngine) {
        /* execute and time measure */
        dur = clock();
        /* in frist element copy data, then only pointers */
        if(!PREP(c->elt, flag)) return -1;  /* flar ~ (sum > 0) ? */
        /* execute */
        if(!EXEC(c->elt, t, dt)) return -1;
        /* get time (in ms) */
        dur = clock() - dur;
        /* statistic */
        /* eliminate first iteration (first time is constZero - inumber) */
        if(dt.iVal != 0) {
            /* add to element statistic */
            c->elt->workTime += dur;
            c->elt->circle ++;
        }
        /* next */
        sum += dur;
        flag = 1;
        c = c->next;
    }

    return (int) sum;
}

/* Set number of threads */
void setThreads(u_char n)
{
#if defined(OS_LINUX) || defined (OS_WINDOWS)
    threadNumber = (n > 0) && (n <= MAX_STREAM_NUM) ? n : 1;
#endif /* OS_LINUX or OS_WINDOWS*/
}

/* Prepare chin for multithread working (not used now) */
void chainBreak(Chain *c, int period)
{
    int i = 0;
    unsigned int sum = 0;
    Chain *tmp;

    threadChain[i++].part = c;

    if(threadNumber > 1) {

        period /= threadNumber;

        while(c) {
            sum += c->elt->workTime;

            if(sum > (unsigned int) period && c->next) {
                threadChain[i++].part = c->next;
                tmp = c;
                c = c->next;
                tmp->next = NULL;

                sum = 0;
            }
            else
                c = c->next;
        }
    }
}

/* Execute computations */
int engine(Chain *c, Number start, Number stop, Number step)
{
    /* ERROR: 03 */
    clock_t tBeg;
    int iter, i;
    /* convert to double all numbers */
    transformDbl(&start, real_part(start));
    transformDbl(&stop, real_part(stop));
    transformDbl(&step, real_part(step));
    /* use iter for working with positive and negative time step */
    iter = (int) ( (stop.dVal - start.dVal) / step.dVal );
    /* check arguments */
    if(!c || iter < 0) {
        setError(E_WRONG_ARG, 4.03, NULL);
        return -1;
    }

    tau = start;  /* not for multithread version!!! */
    haltEngine = False;
    tBeg = clock();
    /* run */
    for(i = 0; i <= iter; i++, tau.dVal += step.dVal) {
        /* iterate */
        if(singleExecute(c, tau, (i > 0) ? step : constZero) == -1)
            return -1;
        if(haltEngine) break;
    }

    return (int) (clock() - tBeg) * 1000 / CLOCKS_PER_SEC;
}

/* Prepare chain for execution in graph */
Chain* prepExecChain(Graph *gr)
{
    /* ERROR: 04 */
    if(!gr || !gr->base || !gr->base->elt)
        return (void*) setError(E_WRONG_ARG, 4.04, NULL);
    /* delete old chain */
    if(gr->exec) chainClear(&(gr->exec));
    /* create new */
    if(!chainForExecution(&(gr->exec), &(gr->execLast),
                            gr->base->elt, gr->base->elt))
    {   return NULL;  }

    return gr->exec;
}

/* Set halt flag */
void halt(void) { haltEngine = True; }
