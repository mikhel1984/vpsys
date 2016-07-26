/** @file vpb_lang.c
~~~ Base Language ~~~
Functions for working with interface language */

/* ERROR FILE NUMBER - 5 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "vpb_lang.h"

#ifdef OS_WINDOWS
#include <windows.h>
#endif /* OS_WINDOWS */

#include "vpb_error.h"
#include "vpb_data.h"
#include "vpc_parser.h"

#define LNG_BUFF_SIZE  5000     /* buffer for language file */
#define LNG_STR_NUMBER 100      /* number of translated strings */
#define LNG_DEFAULT    "en.lng" /* default language file name */
#define LNG_WCHAR_BUF  256      /* buffer size for wide characters */

/* ==== Code from Jeff Bezanson's UTF-8 library ==== */
static const unsigned int offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};
/* ==== Code from Jeff Bezanson's UTF-8 library ==== */
static const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/** @struct
    Collect pointers to translated strings */
struct
{
    unsigned int hash;      /**< Key string hash code */
    char *pos;               /**< Pointer to string in buffer */

} lngString[LNG_STR_NUMBER] = {{0, NULL}};

/* buffer for loading language file */
char lngBuffer[LNG_BUFF_SIZE] = {'\0'};


char lngFile[] = LNG_DEFAULT;
char *lngFilePtr = lngFile;

/* array for print */
wchar_t wBuff[LNG_WCHAR_BUF];

extern char *dlgErr[];

Bool useLngFile = True;
/** Parsing language file data and transform to list of string
    @fn lngParse
    @return True if translated successfully */
Bool lngParse(void);
/** Convert utf-8 strings to wide char (4 byte)
    @fn u8_toucs
    @param dest - dest array of wide characters
    @param sz - size of dest
    @param src - source text
    @param srcsr - source size
    @return length of converted string */
int u8_toucs(unsigned int *dest, int sz, char *src, int srcsz);
/** Switching between decoding functions
    @fn utf8_decode
    @param u8_text - text in utf-8 code
    @return length of converted string */
int utf8_decode(char* u8_txt);

/* Get string with given language */
char* lngText(char *key, char* def)
{
    /* ERROR: 01 */
    unsigned int keyHash;
    register int i = 0;
    FILE *lngPtr;

    if(!key || !useLngFile) return def;
    /* find string according key hash code */
    keyHash = stringHash(key);

    while(i < LNG_STR_NUMBER && lngString[i].pos) {
        if(lngString[i].hash == keyHash)
            return lngString[i].pos;
        i ++;
    }
    /* if not found in base file, add */
    if(strncmp(lngFile, LNG_DEFAULT, 2) == 0) {

        if((lngPtr = fopen(lngFile, "a")) != NULL) {
            /* write to file end */
            fprintf(lngPtr, "\n\"%s\", \"%s\"", key, def);
            fclose(lngPtr);
        }
        else
            setError(E_FOPEN_ERR, 5.01, LNG_DEFAULT);
    }
    /* return default string */
    return def;
}

/* Read data from language file */
Bool loadLanguageFile(void)
{
    /* ERROR: 03 */
    /* load file */
    if(!loadFile(lngFile, lngBuffer, LNG_BUFF_SIZE))
        return setError(E_FOPEN_ERR, 5.03, lngFile);

    /* set pointer for parser */
    setProgramm(lngBuffer);
    /* parse data from file */
    return lngParse();
}

/* Language data pars and modify */
Bool lngParse(void)
{
    /* ERROR: 02 */
    static int current = 0;
    char* lngPos = lngBuffer;
    /* file end */
    if(getToken() == T_END)
        return True;
    /* line end, comment or empty string */
    if(token_type == T_ENDLINE)
        return lngParse();
    /* check position in array - ? */
    if(current >= LNG_STR_NUMBER) return setError(E_MEM_OUT, 9.02, NULL);
    /* check key */
    if(token_type != T_STRING) return setError(E_SYNTAX, 9.02, "expected key");
    /* get hash */
    lngString[current].hash = stringHash(tokenPtr);
    /* check ',' */
    getToken();
    if(*tokenPtr != ',') return setError(E_SYNTAX, 9.02, dlgErr[10]);
    /* get pointer, manually parse */
    lngPos = setPoint().pos;
    /* find text */
    while(*lngPos && iswhite(*lngPos)) lngPos ++;
    /* check text */
    if(*lngPos != '\"') return setError(E_SYNTAX, 9.02, dlgErr[2]);
    /* set begin and get end of string */
    lngString[current].pos = ++lngPos;
    while(*lngPos && *lngPos != '\"') lngPos ++;
    /* check end */
    if(*lngPos == '\0') return setError(E_SYNTAX, 9.02, dlgErr[11]);

    *lngPos++ = '\0';
    current ++;
    setProgramm(lngPos);

    return lngParse();
}

/* ==== Code from Jeff Bezanson's UTF-8 library ==== */
int u8_toucs(unsigned int *dest, int sz, char *src, int srcsz)
{
    unsigned int ch;
    int nb, i=0;
    char *src_end = src + srcsz;

    while (i < sz-1) {
        nb = trailingBytesForUTF8[(unsigned char)*src];
        if (srcsz == -1) {
            if (*src == 0)
                goto done_toucs;
        }
        else {
            if (src + nb >= src_end)
                goto done_toucs;
        }
        ch = 0;
        switch (nb) {
            /* these fall through deliberately */
        case 3: ch += (unsigned char)*src++; ch <<= 6;
        case 2: ch += (unsigned char)*src++; ch <<= 6;
        case 1: ch += (unsigned char)*src++; ch <<= 6;
        case 0: ch += (unsigned char)*src++;
        }
        ch -= offsetsFromUTF8[nb];
        dest[i++] = ch;
    }
 done_toucs:
    dest[i] = 0;
    return i;
}

/* Print text in utf-8 */
void u_print(FILE* out, char* u8_txt)
{
    /* output */
    if(!out) out = stdout;
    /* decode and print */
    if(u8_txt && utf8_decode(u8_txt) > 0) {
        setlocale(LC_CTYPE, "");
        fprintf(out, "%ls", wBuff);
    }
    else
        fprintf(out, ".. error ..");
}

/* Get function for convertion */
int utf8_decode(char* u8_txt)
{
#ifdef OS_LINUX /* also if sizeof(wchar) == 4  */
    return u8_toucs((unsigned int*) wBuff, LNG_WCHAR_BUF, u8_txt, -1);
#elif defined(OS_WINDOWS)
    return MultiByteToWideChar(CP_UTF8, 0, u8_txt, -1, wBuff, LNG_WCHAR_BUF);
#else
    return 0;
#endif
}
