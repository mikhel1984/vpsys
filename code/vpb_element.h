/** @file vpb_element.h
~~~ Base Element ~~~
Structures of element, chain, graph etc. */

#ifndef VPB_ELEMENT_H
#define VPB_ELEMENT_H

#include "vpb_abstract.h"
#include "vpb_data.h"

#define _DP(E,N)   (&(E->state.group[N].dat))      /* pointer to parameter data */
#define _DC(E,D,N) (E->out[D].group[N].elt.data)   /* pointer to connector data */

#define _ptrP(E,N)   (E->state.group+(N))        /* pointer to parameter struct */
#define _ptrC(E,D,N) (E->out[D].group+(N))      /* pointer to connector struct */

/** @struct vpNames
    Save common names to list */
struct vpNames
{
    char* name;                 /**< Name string */
    unsigned int name_hash;    /**< Name hash code */
    struct vpNames *next;      /**< Pointer to next name */

};


/** @struct tParam_
    Ticket for parameter structure */
struct tParam_
{
    Names* nm;
    //char *name;                    /**< Parameter name */
    char* u8_descript;             /**< Parameter description */

    Data dat;                      /**< Parameter initial data */
    Bool isExternal;               /**< Possibility to modify by user */

    ticPar* next;                  /**< Pointer to next parameter ticket */
};

/** @struct vpParam
    Main parameter structure */
struct vpParam
{
    ticPar* ticket;               /**< Pointer to ticket, flag for union */
    u_char isVar;                 /**< Use parameter for optimize (future option) */
    Data dat;
};

/** @struct vpParamGroup
    Representation of parameters in element */
struct vpParamGroup
{
    u_char size;                  /**< Group size */
    Param* group;                 /**< Group array */
};

/** @struct vpConnectGroup
    Representation of connectors in element */
struct vpConnectGroup
{
    u_char size;                  /**< Group size */
    Connector *group;             /**< Group array */
};

/** @struct vpElement
    Structure of element */
struct vpElement
{
    Names* nm;
    //char* name;                   /**< Element name */
    char* u8_descript;            /**< Element description */

    Ticket* tick;                 /**< Pointer to ticket */

    ConnectGroup out[2];          /**< Connectors */

    ParamGroup state;             /**< Parameters */

    clock_t workTime;             /**< Working time, in takts */
    unsigned int circle;         /**< Execution times number */

    int id;
};

/** @struct eltGraph
    'Namespace' structure */
struct eltGraph
{
    Names* nm;
    //char* name;                   /**< Graph name */
    char* u8_descript;            /**< Graph description */

    Chain* base;                  /**< Chain of elements */
    Chain* baseLast;              /**< Pointer to last base chain element */
    Chain* exec;                  /**< Chain of execution */
    Chain* execLast;              /**< Pointer to last exec chain element */
};

/** @struct eltChain
    Structure for setting order of elements */
struct eltChain
{
    Element *elt;                 /**< Pointer to element */
    Chain *next;                  /**< Pointer to next chain element */
};




/*
Bool eltRename(Element *e, Text *name); */
/** Add element to chain
    @fn addToChain
    @param base - pointer to chain base pointer
    @param e - pointer to element
    @return True if done */
Bool addToChain(Chain **base, Chain **last, Element *e);
/** Check if element in chain
    @fn isInChain
    @param base - pointer to chain base
    @param e - pointer to element
    @return True if find */
Bool isInChain(Chain *base, Element *e);
/** Delete element
    @fn eltDelete
    @param elt - pointer to element */
void eltDelete(Element* elt);
/** Get data from parameter with given name
    @fn eltParamData
    @param elt - pointer to element
    @param par - parameter name
    @return pointer to data */
Data* eltParamData(Element *elt, const char* par);

/** Get pointer to connector with given name
    @fn eltConnector
    @param elt - pointer to element
    @param con - connector name
    @param dir - data stream direction
    @return pointer to connector */
Connector* eltConnector(Element *elt, const char *con, int dir);

int graphSize(Graph *g);

void chainClear(Chain** c);

Element* eltCopy(char* name, Element *base);

//int isInGraph(Graph* g, Element *e);

Names* addName(const char* name);







#endif /* VPB_ELEMENT_H */
