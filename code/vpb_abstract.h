#ifndef VPB_ABSTRACT_H
#define VPB_ABSTRACT_H

/* current version */
#define VPSYS_VERS 0.51

/* used for readint utf-8, threads */
#define OS_LINUX
//#define OS_WINDOWS

typedef struct vpNumber Number;
typedef struct vpData Data;
typedef struct vpDataArray Array;
typedef struct vpConnect Connector;
typedef struct vpElement Element;
typedef struct vpParam Param;
typedef struct vpParamGroup ParamGroup;
typedef struct vpConnectGroup ConnectGroup;

typedef struct eltChain Chain;
typedef struct eltGraph Graph;

typedef struct tConnector_ ticCon;
typedef struct tParam_ ticPar;
typedef struct vpTicket Ticket;

typedef struct vpNames Names;
typedef struct vpVars Var;

typedef unsigned char u_char;

/* numbers */
typedef double    dnumber;
typedef float     cnumber;
typedef long int  inumber;
typedef int       rnumber;

typedef enum Bool_ {False=0, True} Bool;

#define KEY_FILE  10
#define KEY_CHAIN 20
#define KEY_CODE  30
#define KEY_GRAPH 40

#define MAXNAME  32
#define PROG_BUF_SIZE  5000

#define IFFREE(X) if(X) free(X)

#endif /* VPB_ABSTRACT_H */
