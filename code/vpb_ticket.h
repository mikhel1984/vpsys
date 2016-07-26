/** @file vpb_ticket.h
~~~ Base Ticket ~~~
Structures and functions for element initialization */

#ifndef VPB_TICKET_H
#define VPB_TICKET_H

#include "vpb_abstract.h"
#include "vpb_element.h"

/** @struct vpTicket
    Main element ticket */
struct vpTicket
{
    Names* type;
    //char *type;                       /**< Element type */
    char* u8_descript;                /**< Type description */

    //unsigned int hash;              /**< Type name hash */

    ticPar *lstPar;                   /**< Pointer to first parameter ticket */
    ticPar *parLast;                  /**< Pointer to last parameter ticket */
    ticCon *lstIn;                    /**< Pointer to first input connector */
    ticCon *inLast;                   /**< Pointer to last input connector */
    ticCon *lstOut;                   /**< Pointer to first output connector */
    ticCon *outLast;                  /**< Pointer to last output connector */

    u_char parNum;                 /**< Parameter number */
    u_char cinNum;                 /**< Input connector number */
    u_char coutNum;                /**< Output connector number */

    Bool (*exec)(Element*, Number, Number);    /**< Function for execution */
    Bool (*prep)(Element*, u_char);           /**< Function for data correction - ? */
};

/** Delete ticket data
    @fn ticketDelete
    @param t - pointer to ticket */
void ticketDelete(Ticket* t);
/** Get input connector index
    @fn cinPos
    @param elt - index of element
    @param cinName - name of connector
    @return connector index or -1 */
int cinPos(int elt, const char* cinName);
/** Get output connector index
    @fn coutPos
    @param elt - index of element
    @param coutName - name of connector
    @return connector index or -1 */
int coutPos(int elt, const char* coutName);
/** Get parameter position
    @fn parPos
    @param elt - index of element
    @param parName - name of parameter
    @return parameter index or -1 */
int parPos(int elt, const char* parName);
/** Get parameter data from ticket
    @fn getParameterData
    @param elt - element index
    @param pos - parameter index
    @return pointer to data structure */
Data* getParameterData(int elt, int pos);

/** Check if parameter public
    @fn isParPablic
    @par elt - element index
    @par pos - position index
    @return true if public */
Bool isParPablic(int elt, int pos);

ticPar* parameterFind(int elt, int pos);

#endif /* VPB_TICKET_H */
