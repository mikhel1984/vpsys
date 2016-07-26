
/* ERROR FILE NUMBER - 13 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpe_collect1.h"
#include "vp_construct.h"
#include "vpb_element.h"
#include "vpb_data.h"
#include "vpb_connectors.h"
#include "vpb_error.h"

/* SRC */
Bool regSrcElt(void);
/* SYNC */
Bool regSyncFile(void);

/* ASRC */
Bool regArrSrc(void);

Bool regCollect1(void)
{
    regSrcElt();

    regSyncFile();

    /*regArrSrc();*/

    return True;
}

/* ------------- Elementary data source --------- */

/* source functions */
/* ERROR FUNCTION NUMBER - 1 */
Bool srcElementarExec(Element *e, Number t, Number dt)
{
    Data *datA, *datB, *datC;

    /* set function representation */
    if(dt.dVal == 0) {
        /* check output data */
        if(!_DC(e, C_OUT, 0) || !_DC(e, C_OUT,0)->arrVal.numb)
            return setError(E_WTF, 13.01, NULL);
        datA = _DP(e, 0);  /* string type */
        datB = _DP(e, 3);  /* number type */

        if(stringEqUp("CONST", datA->txtVal.str))
            datB->numVal.iVal = 0;
        else if(stringEqUp("LIN", datA->txtVal.str))
            datB->numVal.iVal = 1;
        else if(stringEqUp("SIN", datA->txtVal.str))
            datB->numVal.iVal = 2;
        else if(stringEqUp("RAND", datA->txtVal.str))
            datB->numVal.iVal = 3;
        else
            return setError(E_WTF, 13.01, datA->txtVal.str);
    }

    /* get value */
    datA = _DP(e, 1);      /* k   */
    datB = _DP(e, 2);      /* t0  */
    datC = _DC(e,C_OUT,0); /* output data (array[1]) */

    switch(_DP(e,3)->numVal.iVal) {
    case 0:
        datC->arrVal.numb[0] = datA->numVal;
        break;
    case 1:
        /* k*(t-t0) */
        datC->arrVal.numb[0] = nProd(datA->numVal, nSub(t, datB->numVal));
        break;
    case 2:
        /* sin(k*(t-t0)) */
/*        printf("k = %s\n", numAsText(datA->numVal));
        printf("t0 = %s\n", numAsText(datB->numVal));
        printf("t = %s\n", numAsText(t)); */
        datC->arrVal.numb[0] = nSin( nProd(datA->numVal, nSub(t, datB->numVal)) );
/*        printf("res = %s\n", numAsText(datC->arrVal.numb[0]));*/
        break;
    case 3:
        /* random */
        datC->arrVal.numb[0] = nRand(datA->numVal);
        break;
    default:
        return setError(E_WRONG_TYPE, 13.1, NULL);
    }

    return True;
}

Bool regSrcElt(void)
{
    int elt;

    elt = regElement("SRC",
        lngText("scom_t", "Generate elementary signals: const, k*t, sin(kt), random"));

    if(elt == -1) return False;
    /* 0 - function type */
    regParameter(elt, "FUNC",
        lngText("scom_p0", "'const' - constant, 'lin' - linear, 'sin' - sinusoid, 'rand' - random"),
                regDataText("const"), True);
    /* 1 - koefficient */
    regParameter(elt, "K",
        lngText("scom_p1",
                "Parameter: value for 'const', angle for 'lin', frequency for 'sin', threshold for 'rand'"),
                dataDbl(1), True);
    /* 2 - time shift */
    regParameter(elt, "T0",
        lngText("tau0", "Initial time shift"), dataDbl(0), True);
    /* 3 - internal represeintation of function type */
    regParameter(elt, "N_FUNC", NULL, dataInt(0), False);
    /* output connector */
    regConnector(elt, "OUT0", NULL, 1, C_OUT);
    /* function */
    regExec(elt, srcElementarExec);

    return True;
}

/*  --------------- SYNC DATA ---------------  */

/* sync file */
/* ERROR FUNCTION NUMBER - 2 */
Bool syncFileExec(Element* e, Number t, Number dt)
{
    Number *in;
    FILE *fPtr = NULL;
    int arrMax = 10;

    Data *datA, *datB, *datC;

    datC = _DP(e, 4);  /* next moment for writing */

    if(dt.dVal == 0) {
        datA = _DP(e, 0);  /* file name */
        datB = _DP(e, 3);  /* pointer to file */
        /* get stream */
        if(datA->txtVal.str[0] == '\0')
            fPtr = stdout;
        else
            fPtr = fopen(datA->txtVal.str, "w");

        if(fPtr)
            datB->freeVal.ptr = (void*) fPtr;
        else
            return setError(E_FOPEN_ERR, 13.02, NULL);

        datC->numVal.dVal = 0;
    }

    /* check time */
    if(t.dVal < datC->numVal.dVal)
        return True;

    fPtr = (FILE*) _DP(e,3)->freeVal.ptr;
    datA = _DC(e,C_IN,0);  /* input data */
    datB = _DP(e, 1);      /* delimeter */
    //datC = _DP(e, 2);      /* time step */
    fprintf(fPtr, "%lf", t.dVal);

    iReset(datA);

    switch(datA->type) {
    case tArray:
        while((in = getNext(datA)) != NULL) {
            fprintf(fPtr,"%s%s", datB->txtVal.str, numAsText(*in));
            if(arrMax-- == 0) break;
        }
        break;
    default:
        fprintf(fPtr, "%s%s", datB->txtVal.str, dataAsText(*datA));
    }
    fprintf(fPtr, "\n");
    /* next time stamp */
    datC->numVal = nSum(datC->numVal, _DP(e, 2)->numVal);

    return True;
}

Bool regSyncFile(void)
{
    int elt;

    elt = regElement("SYNC",
        lngText("dfile_t", "Write input data to file"));

    if(elt == -1) return False;
    /* 0 - file name */
    regParameter(elt, "FNAME",
        lngText("dfile_p0", "Output file name (screen if empty)"),
            regDataText(""), True);
    /* 1 - data delimeter */
    regParameter(elt, "DELIM",
        lngText("dfile_p1", "Data delimeter"), regDataText(" "), True);
    /* 2 - time scale */
    regParameter(elt, "SCALE",
        lngText("dfile_p2", "Time scale step"), dataDbl(0.1), True);
    /* 3 - pointer to file stream */
    regParameter(elt, "FPTR", NULL, dataFree(KEY_FILE, 0, 1), False);
    /* 4 - time step controle */
    regParameter(elt, "TNEXT", NULL, dataDbl(0), False);
    /* input connector */
    regConnector(elt, "IN0", NULL, DAT_ANYTYPE, C_IN);

    regExec(elt, syncFileExec);

    return True;
}

/*   -----  Amplifier ---------- */
#if 0 /* Just as sample */
Bool matAmplExec(Element *e, Number t, Number dt)
{
    Data *in, *out;
    Number *nOut, nAmp;
    /* get data */
    in = getInData(e, 0);
    out = getOutData(e, 0);
    /* resize */
    if(!resizeData(out, *in)) return False;
    /* amplify */
    nAmp = *_n(getParData(e, 0));
    iReset(in);
    iReset(out);

    while((nOut = getNext(out)) != NULL) {
        *nOut = nProd(nAmp, *getNext(in));
    }

    return True;
}
#endif



/* ---------------------- Array Source ------------------ */

Bool arrSrcExec(Element *e, Number t, Number dt)
{
    Data *src, *out;
    Number *n;

    if(isZero(dt)) {
        src = _DP(e, 0);
        out = _DC(e, C_OUT, 0);

        if(!resizeData(out, *src)) return False;

        iReset(src);
        iReset(out);

        while((n = getNext(out)) != NULL) *n = *getNext(src);
    }

    return True;
}

Bool regArrSrc(void)
{
    int elt;

    elt = regElement("ASRC",
        lngText("sarr_t", "Source of array data"));

    if(elt == -1) return False;

    regParameter(elt, "DIM", lngText("sarr_p0", "Array dimention"),
                    dataArrayNull(), True);

    regConnector(elt, "OUT0", NULL, DAT_ANYSIZE, C_OUT);

    regExec(elt, arrSrcExec);

    return True;
}
