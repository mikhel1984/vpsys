/** @file vpc_parser.h
~~~ Code Parser ~~~
Analize text and get tokens */

#ifndef VPC_PARSER_H
#define VPC_PARSER_H

#include "vpb_abstract.h"

#define COMMENT '#'    /* define character for string comments */
#define ENDLINE (token_type == T_END || token_type == T_ENDLINE)

/** @enum TokenType
    List of token types */
typedef enum TokenType Token;
enum TokenType {T_NONE, T_WORD, T_DELIMETER, T_INT, T_FLOAT, T_BIN, T_HEX, T_STRING,
                T_ENDLINE, T_END, T_ERR, T_KEYCHAR};

/** @enum SpecialToken
    List of special delimeter types */
typedef enum SpecialToken Special;
enum SpecialToken {S_NONE, S_LINK,  S_RANGE, S_REF,
                    /* from */ S_NE, S_LT, S_LE, S_EQ, S_GE, S_GT /* to */ };

typedef struct ParserState
{
    u_char type;
    u_char spec;
    char*  pos;
} LastState;

/* use external references */
extern Token token_type;
extern Special token_spec;
extern char *tokenPtr;
extern char *leftTokenPtr;

/** Check number type
    @fn getNumberType
    @param lingth - return number of digits
    @return type of number */
Token getNumberType(int *length);
/** Set pointer to the begining of programm
    @fn setProgramm
    @param code - pointer to code text */
void setProgramm(char* code);
/** Get next token from text
    @fn getToken
    @return token type */
Token getToken(void);
/** Return last token to stream
    @fn putback */
void putback(void);
/** Evaluate number (and call getToken()!)
    @fn eval_num
    @param n - result number
    @return true if evaluate successfully */
Bool eval_num(Number *n);  /* !!! use putback after function !!! */
/** Load file to buffer
    @fn loadFile
    @param fName - file name
    @param b - pointer to buffer
    @param buff - buffer size
    @return true if file opened */
Bool loadFile(const char* fName, char* b, int buff);
/** Find construction 'a = b'
    @fn getSetTokens
    @return true if get no errors */
Bool getSetTokens(void);
/** Check character to whitespace
    @fn iswhite
    @param c - character
    @return true if whitespace */
Bool iswhite(char c);
/** Remember current position in stream
    @fn setPoint */
LastState setPoint(void);
/** Return to position in memory
    @fn goToPosition */
void goToPoint(LastState* ls);

Bool eval_args(Number *n, u_char *val, u_char max);

Number* eval_index(Data* d);

Bool eval_concat(Data* res, Data* dat);

Bool eval_data(Data *d, int expec);

#endif /* VPC_PARSER_H */
