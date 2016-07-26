/** @file vpb_connectors.h
~~~ Base Connectors ~~~
Structures for element connection and data exchange */

#ifndef VPB_CONNECTOR_H
#define VPB_CONNECTOR_H

#include "vpb_abstract.h"
#include "vpb_data.h"
#include "vpb_element.h"

typedef struct vpLink Link;

/** @enum ConnectType
    Direction of data stream */
enum ConnectType {C_IN, C_OUT};

/** @enum ConnectData
    Type and size of connector data, used in connector ticket */
enum ConnectData {DAT_ANYTYPE = -3, DAT_SIGNAL, DAT_STRING, DAT_ANYSIZE, DAT_ONE};

/** @struct tConnector_
    Connector ticket */
struct tConnector_
{
    Names* nm;
    //char *name;                 /**< Connector name */
    char *u8_descript;          /**< Connector description */

    //u_char direction;           /**< Data stream direction (enum ConnectType) */

    int dat;                    /**< Data type or size (enum ConnectData) */

    ticCon* next;               /**< Pointer to next connector ticket */

};

/** @struct vpConnect
    Main connector structure */
struct vpConnect
{
    ticCon* ticket;             /**< Pointer to ticket */
    Element* master;            /**< Pointer to master element */
    /** @union
        Collect structure types */
    union {
        /** @struct
            Used in block when *ticket == NULL */
        struct {
            Connector *next;    /**< Pointer to next connector */
            char *name;         /**< Connector name */
        } blk;
        /** @struct
            Used when element not a block (*ticket != NULL) */
        struct {
            Link *pin;          /**< Structure for linking */
            Data *data;         /**< Pointer to data */
            u_char pointer;     /**< Flag that have no dinamical data */
        } elt;
    };
};

/** @struct vpLink
    Structure for linking connectors */
struct vpLink
{
    Connector *base;            /**< Pointer to connector */

    Link *lnk;                  /**< Pointer to vpLink in second connector */
    Link *next;                 /**< Next linking of current connector */
};

/** Set connection between two elements
    @fn conSetConnection
    @param from - source connector
    @param to   - dst connector
    @return True if setup connection */
Bool conSetConnection(Connector* from, Connector *to);

/** Clear connector data
    @fn conClear
    @param connect - pointer to connector
    @param dir - data direction type */
void conClear(Connector *connect, u_char dir);
/** Get connector of linked element
    @fn getConnection
    @param base - initial connector
    @param pos - number of linkage
    @return pointer to second connector */
Connector* getConnection(Connector* base, int pos);
/** Get connector with data in block connector
    @fn conGetBegin
    @param from - block connector
    @return pointer to element connector */
Connector* conGetBegin(Connector *from);
/** Initialize data according data type
    @fn conInitData
    @param c - pointer to connector
    @return True if initializes seccessfully */
Bool conInitData(Connector* c, u_char dir);

Bool defaultConnectorPrepare(Element *elt, u_char onlyPointer);

char *conDataType(int marker);

Bool conDisconnect(Connector* from, Connector *to);

Element* getConnectedElt(Connector* c, int pos);

int conFindIndex(Connector* c, u_char dir);

#endif /* VPB_CONNECTOR_H */
