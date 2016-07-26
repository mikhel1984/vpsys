/** @file vpc_translate.h
~~~ Code Translator ~~~
Translate script to internal instructions */

#ifndef VPC_TRANSLATE_H
#define VPC_TRANSLATE_H

#include "vpb_abstract.h"

/* --------------------------
    List of inctruction codes
   -------------------------- */

/* pointers to data */
#define PTR_CIN  1   /* set pointer to input data */
#define PTR_COT  2   /* set pointer to output data */
#define PTR_PAR  3   /* set pointer to parameter */
/* get number from accumulator */
#define SET_NUM  10  /* write number from accumulator */
#define SET_ARG  12  /* write number from accumulator to agrument array */
#define GET_LEN  13  /* get array length */
/* write number to accumulator */
#define GET_NUM  20  /* write number to accumulator */
#define CST_ARG  23  /* write number from constant to argument */
/* evaluate functions */
#define FNC_ARG  30  /* evaluate function, write result to accumulator */
#define FNC_SUM  31  /* summ accumulator vith number, write result to accumulator */
#define FNC_SUB  32
#define FNC_MLT  33
#define FNC_DIV  34
#define FNC_INV  35  /* invert sign of number in accumulator */
#define FNC_ORL  36  /* get logic 'OR', set result to accumulator */
#define FNC_AND  37  /* get logic 'AND' */
#define FNC_NOT  38  /* logical invertion */
#define FNC_IND  39  /* get index from argument list to accumulator */
/* jump */
#define BRN_POS  50  /* go to line */
#define BRN_ZER  51  /* go to line if accumulator is zero */
#define BRN_BRK  52  /* break circle */
#define BRN_CON  53  /* continue circle */
/* additional */
#define ACC_CLR  60  /* clear accumulator - ? */
#define ACC_SWP  61  /* swap values between ceil and accumulator */
/* evaluate and write result to temporary memory */
#define TMP_SET  70  /* write to temporary memory value */
#define TMP_SUM  71  /* write result to vars */
#define TMP_SUB  72
#define TMP_MLT  73
#define TMP_DIV  74
#define TMP_ORL  75
#define TMP_AND  76
/* compare */
#define CMP_EQL  80
#define CMP_NEQ  81
#define CMP_LTE  82
#define CMP_LEQ  83
#define CMP_GTE  84
#define CMP_GEQ  85
/* halt */
#define HLT_EVL  90  /* halt computation */
#define HLT_ERR  91  /* halt with error result */
/* information about code */
#define INF_WRD  255


#define IND_VAR  0
#define IND_CONST 1

/** @struct codeWord
    Representation of command */
typedef struct codeWord
{
    u_char command; /**< Code of command */
    u_char value;   /**< Parameter of command */
    int  position;  /**< Additional value */
} Word;

/** Translate script to code
    @param elt - ID of current type position
    @return true if translated successfully */
Bool trans_code(int elt);
/** Execute translated code
    @param e  - element
    @param t  - current time
    @param dt - last time shift
    @return true if no errors */
Bool transCodeExec(Element *e, Number t, Number dt);
/** Write translated code to type description
    @param elt - type ID
    @return true if written successfully */
Bool transWriteCode(int elt);


#endif /* VPC_TRANSLATE_H */
