
/* ERROR FILE NUMBER - 16 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "vpe_collect2.h"
#include "vp_construct.h"
#include "vpb_element.h"
#include "vpb_data.h"
#include "vpb_connectors.h"
#include "vpb_error.h"

#define INT_TRAPEZ "TRAPEZE"


/* SUM */
Bool regMatSumm(void);
/* CONST */
Bool regMatConst(void);
/* FUNC */
Bool regMatFunc(void);
/* XINT */
Bool regMatIntegrate(void);
/* XDIFF */
Bool regMatDerivative(void);



Bool regCollect2(void)
{
    regMatSumm();
    regMatConst();
    regMatFunc();
    regMatIntegrate();
    regMatDerivative();

    return True;
}

/*   ------------ Summator --------------- */

/* cannot copy now... */
Bool matSummPrep(Element *e, u_char u)
{
    return True;
}

Bool matSummExec(Element *e, Number t, Number dt)
{
    Data *out;
    Connector* c;
    int i = 1;

    out = getOutData(e, 0);

    c = getConnection(_ptrC(e,C_IN,0), 0);

    if(!dataCopy(out,*(c->elt.data))) return False;

    while((c = getConnection(_ptrC(e,C_IN,0), i++)) != NULL) {
        if(!dataSum(out, *(c->elt.data))) return False;
    }

    return True;
}


Bool regMatSumm(void)
{
    int elt;

    elt = regElement("SUM",
        lngText("msum_t", "Return summ of incomming data"));

    if(elt == -1) return False;

    regConnector(elt, "IN0", NULL, DAT_ANYTYPE, C_IN);

    regConnector(elt, "OUT0", NULL, DAT_ANYTYPE, C_OUT);

    regPrep(elt, matSummPrep);

    regExec(elt, matSummExec);

    return True;
}

/*  --------------- Constant operations ------------------- */

Bool matConstExec(Element *e, Number t, Number dt)
{
    Data *in, *out;
    Number k = _DP(e, 1)->numVal, *n;
    char op = _DP(e, 0)->txtVal.str[0];

    in = _DC(e, C_IN, 0);
    out = _DC(e, C_OUT, 0);

    if(!resizeData(out, *in)) return False;

    iReset(in);
    iReset(out);

    while((n = getNext(out)) != NULL) {
        switch(op) {
        case '+':
            *n = nSum(k, *getNext(in));
            break;
        case '-':
            *n = nSub(*getNext(in), k);
            break;
        case '*':
            if(k.iVal == -1) {
                *n = *getNext(in);
                numInvert(n);
            }
            else
                *n = nProd(*getNext(in), k);
            break;
        case '/':
            *n = nRat(*getNext(in), k);
            break;
        case '\\':
            *n = nRat(k, *getNext(in));
            break;
        default:
            return False;
        }
    }

    return True;
}

Bool regMatConst(void)
{
    int elt;

    elt = regElement("OP",
        lngText("mcst_t", "Chagne data on constant value"));

    if(elt == -1) return False;

    regParameter(elt, "OP", lngText("mcst_p0", "Operation"),
                    regDataText("*"), True);

    regParameter(elt, "K", lngText("mcst_p1", "Constang value"),
                    dataInt(1), True);

    regConnector(elt, "IN0", NULL, DAT_ANYSIZE, C_IN);

    regConnector(elt, "OUT0", NULL, DAT_ANYSIZE, C_OUT);

    regExec(elt, matConstExec);

    return True;
}

/* -------------------- Function operation -------------------- */

Bool matFuncExec(Element *e, Number t, Number dt)
{
    Data *in, *out;
    Number *n, arg[2];
    int fn;

    in = _DC(e, C_IN, 0);
    out = _DC(e, C_OUT, 0);

    if(!resizeData(out, *in)) return False;

    if(isZero(dt)) {
        _DP(e, 0)->numVal.iVal = getFunc(_DP(e,1)->txtVal.str);

        if(_DP(e, 0)->numVal.iVal == -1 && _DP(e,1)->txtVal.str[0] != '\0')
            return False;
    }

    fn = _DP(e, 0)->numVal.iVal;

    iReset(in);
    iReset(out);

    while((n = getNext(out)) != NULL) {
        if(fn == -1)
            *n = *getNext(in);
        else {
            switch(getFuncArg(fn)) {
            case 1:
                if(!evalFunc(fn, getNext(in), n)) return False;
                break;
            case 2:
                arg[0] = *getNext(in);
                arg[1] = _DP(e, 2)->numVal;
                if(!evalFunc(fn, arg, n)) return False;
                break;
            default:
                printf("DBG: Too many argumets\n");
                return False;
        } }
    }

    return True;
}

Bool regMatFunc(void)
{
    int elt;

    elt = regElement("FUNC",
        lngText("mfnc_t", "Apply function to data stream"));

    if(elt == -1) return False;

    /* internal representation as number */
    regParameter(elt, NULL, NULL, dataInt(-1), False);
    /* 1 */
    regParameter(elt, "F", lngText("mfnc_p0", "Function name"),
                    regDataText(""), True);
    /* 2 !!! change to array when it will be writen */
    regParameter(elt, "ARG1", lngText("mfnc_p1", "First additional argument"),
                    dataInt(1), True);

    regConnector(elt, "IN0", NULL, DAT_ANYSIZE, C_IN);

    regConnector(elt, "OUT0", NULL, DAT_ANYSIZE, C_OUT);

    regExec(elt, matFuncExec);

    return True;
}

/* ------------------ Integral -------------------- */

Bool matFuncIntegrate(Element *e, Number t, Number dt)
{
    Data *datA;
    Number *nIn, *nOut, *nAcc;

    nIn = _DC(e, C_IN, 0)->arrVal.numb;
    nOut = _DC(e, C_OUT, 0)->arrVal.numb;
    nAcc = _DP(e, 3)->arrVal.numb;

    /* check for complex !!! */
    if(nIn->type == nCMP) return False;

    if(isZero(dt)) {
        datA = _DP(e, 1);

        if(stringEqUp(datA->txtVal.str, INT_TRAPEZ))
            _DP(e, 0)->numVal = constZero;
        else if(stringEqUp(datA->txtVal.str, "SQUARE"))
            _DP(e, 0)->numVal = intToNumber(1);
        /* else ... */

        nAcc[1] = nAcc[2] = constZero;
        nAcc[0] = *nIn;

        *nOut = _DP(e, 2)->numVal;
        return True;
    }

    datA = _DP(e, 2);

    switch(_DP(e, 0)->numVal.iVal) {
    case 0: /* trapez */
        nAcc[1] = nSum(nAcc[1], nAcc[2]);
        nAcc[2] = *nIn;
        /* (x0 + xN)/2 */
        *nOut = nMult(nSum(nAcc[0], nAcc[2]), 0.5);
        /* ... + sum(x) */
        *nOut = nSum(*nOut, nAcc[1]);
        /* ... * h */
        *nOut = nProd(*nOut, dt);
        /* ... + I0 */
        //out->arrVal.numb[0] = nSum(out->arrVal.numb[0], datA->numVal);
        break;
    case 1: /* square */
        /* get integral value */
        *nOut = nProd(nAcc[0], dt);
        /* get summ for next */
        nAcc[0] = nSum(nAcc[0], nAcc[1]);
        /* save input */
        nAcc[1] = *nIn;
        //out->arrVal.numb[0] = nSum(out->arrVal.numb[0], datA->numVal);
        break;
    default:
        return setError(E_WTF, 0, NULL);
    }
    *nOut = nSum(*nOut, datA->numVal);

    return True;
}

Bool regMatIntegrate(void)
{
    int elt;
    Data dTmp;

    elt = regElement("XINT",
            lngText("xint_t", "Integrate incoming data"));
    if(elt == -1) return False;

    /* 0 - internal representation */
    regParameter(elt, NULL, NULL, dataDbl(0), False);
    /* 1 - integral type */
    regParameter(elt, "ALG", lngText("xint_p0", "Type of algorithm"),
                    regDataText(INT_TRAPEZ), True);
    /* 2 - initial value */
    regParameter(elt, "X0", lngText("xint_p1", "Initial value"),
                    dataInt(0), True);
    dataInitArray(&dTmp, 3); /* x0, sum, xN */
    /* 3 - accumulator */
    regParameter(elt, NULL, NULL, dTmp, False);
    /* input */
    regConnector(elt, "IN0", NULL, DAT_ONE, C_IN);
    /* output */
    regConnector(elt, "OUT0", NULL, DAT_ONE, C_OUT);

    regExec(elt, matFuncIntegrate);

    dataClear(&dTmp);

    return True;
}

/* -------------------- First Derivative ---------------- */
Bool matFuncDiff(Element *e, Number t, Number dt)
{
    Number *nAcc, *nIn, *nOut;

    nIn = _DC(e, C_IN, 0)->arrVal.numb;
    nOut = _DC(e, C_OUT, 0)->arrVal.numb;
    nAcc = _DP(e, 1)->arrVal.numb;


    if(isZero(dt)) {
        *_DP(e, 2) = dataInt(2);
        //nAcc[1] = nAcc[2] = constZero;
        nAcc[0] = *nIn;

        *nOut = _DP(e, 0)->numVal;
        return True;
    }

    switch(_DP(e,2)->numVal.iVal) {
    case 2:
        nAcc[1] = *nIn;
        *nOut = nSub(nAcc[1], nAcc[0]);
        *nOut = nRat(*nOut, dt);
        _DP(e,2)->numVal.iVal = 1;
        return True;
    case 1:
        nAcc[2] = *nIn;
        _DP(e,2)->numVal.iVal = 0;
        break;
    case 0:
        nAcc[0] = nAcc[1];
        nAcc[1] = nAcc[2];
        nAcc[2] = *nIn;
        break;
    default:
        return False;
    }

    *nOut = nSum(nAcc[0], nMult(nAcc[1], -4.0));
    *nOut = nSum(*nOut, nMult(nAcc[2], 3.0));
    *nOut = nRat(*nOut, nMult(dt, 2.0));

    return True;
}

Bool regMatDerivative(void)
{
    int elt;
    Data dTmp;

    elt = regElement("XDIFF",
                lngText("xdif_t", "Find incoming data derivative"));
    if(elt == -1) return False;

    /* 0 - initial value */
    regParameter(elt, "X0", lngText("xdif_p0", "Initial value"),
                    dataDbl(0), True);
    /* 1 - accumulator */
    dataInitArray(&dTmp, 3);
    regParameter(elt, NULL, NULL, dTmp, False);
    /* 2 - steps */
    regParameter(elt, NULL, NULL, dataInt(0), False);
    /* input */
    regConnector(elt, "IN0", NULL, DAT_ONE, C_IN);
    /* output */
    regConnector(elt, "OUT0", NULL, DAT_ONE, C_OUT);

    regExec(elt, matFuncDiff);

    dataClear(&dTmp);
    return True;
}
