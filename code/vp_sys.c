
/* ERROR FILE NUMBER - 15 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "vpb_abstract.h"
#include "vp_work.h"
#include "vp_sys.h"
#include "vp_construct.h"
#include "vpb_data.h"
#include "vpb_memory.h"
#include "vpc_parser.h"
#include "vpb_error.h"
#include "vpb_lang.h"
#include "vpc_dialog.h"
#include "vpe_collect1.h"
#include "vpe_collect2.h"
#include "vpe_block.h"

#define VP_HEAD1 "*  * ***   (/)  "
#define VP_HEAD2 " * * *  *  _/   "
#define VP_HEAD3 "  ** ***    \\   "
#define VP_HEAD4 "   * *     (/)  "

/* #define INVITE ">> " */
#define NEXT_LINE "... "


#define FILE_SETTINGS "vpsys.conf"
#define FILE_SET_SIZE 1000

#define EMPTY_TIMES 3

#define CONF_LANG   "lang"
#define CONF_ULOG   "use_log"
#define CONF_NLOG   "new_log"
#define CONF_DT     "dt"
#define CONF_TOLL   "tol"
#define INTER_SIZE  512

/*jmp_buf ebuf;*/

jmp_buf registrateErr;
jmp_buf addNewErr;
Bool interactiveRunning = False;


char progBuf[PROG_BUF_SIZE];
char* progBufPtr = progBuf;
Bool execMode = False;

void clear_console(void);
void print_head(void);
void read_settings(void);
Bool parse_settings(void);

int interactive_mode(void);

int execute_mode(int argc, char* argv[]);


int vp_sys(int argc, char* argv[])
{
    int res = 0;
    extern Graph* thisGraph;

    if(setjmp(registrateErr)) {
        memFree();
        return -1;
    }

    numInitConst();   /* set constants */
    logDate();        /* begin new session in log file */
    read_settings();  /* set configuration */
    /* in interactive mode load language file */
    if(argc == 1)  loadLanguageFile();
    /* base elements */
    if(base_elements() != 0) {
        puts("Element registering error");
        printError();
        return 1;
    }
    /* init graph */
    thisGraph = newGraph("BASE", NULL);

    if(argc == 1) {
        res = interactive_mode();
    }
    else {
        res = execute_mode(argc, argv);
    }

    memFree();

    return res;
}

void clear_console(void)
{
#ifdef OS_LINUX
    system("clear");
#elif defined(OS_WINDOWS)
    system("cls");
#endif
}

void print_head(void)
{
    /* change puts to printf if have additional text */
    printf("%sVer. %.2f alpha\n", VP_HEAD1, VPSYS_VERS);
    printf(VP_HEAD2 "... Math -> is -> fun ...\n");
    //u_print(NULL, lngText("i_simp", "Simple dynamical systems modeling"));
    //printf("\n");
    //puts(VP_HEAD3 "    ... Math -> is -> fun ...");
    puts(VP_HEAD3 "E-mail: vpsys@yandex.ru");
    puts(VP_HEAD4 "(c) 2015, Stanislav Mikhel\n");

    u_print(NULL, lngText("i_w1", "Wellcome to"));
    printf(" VPSYS %.2f! ", VPSYS_VERS);
    u_print(NULL, lngText("i_simp", "Simple dynamical systems modeling"));
    printf("\n");
    u_print(NULL, lngText("i_w2", "To exit the programm enter 'end'"));
    //printf("\nE-mail: vpsys@yandex.ru\n");
    printf("\n\n");
}

void read_settings(void)
{
    FILE *confFile = NULL;

    if(parse_settings()) {
        return;
    }
    else if((confFile = fopen(FILE_SETTINGS, "w")) != NULL){
        /* try to write default settings to file */
        /* language */
        fprintf(confFile, "# === SETTINGS ===\n\n"
                "#   Language\n"
                "# en - english, ru - russian, eo - esperanto\n"
                CONF_LANG " = en\n");
        fprintf(confFile, "\n#   Log\n"
                "# Do you with to use log file\n"
                CONF_ULOG " = yes\n"
                "# Start new log for every session\n"
                CONF_NLOG " = yes\n");
        fprintf(confFile, "\n# Set default time stamp\n"
                CONF_DT " = 1e-1\n");
        fprintf(confFile, "\n# Set tolerance\n"
                CONF_TOLL " = 4\n");
        fprintf(confFile, "\n#   Constants\n"
                "# (allready exist: %%pi = 3.14.., %%e = 2.71.., %%i = sqrt(-1))\n"
                "%%phi = 1.6180339    # harmony:) \n");
        fclose(confFile);
    }
}

/* get settings from file */
/* ERROR FUNCTION NUMBER - 1 */
Bool parse_settings(void)
{
    char setBuff[FILE_SET_SIZE];
    Number n;

    extern char* lngFilePtr;
    extern Bool writeLog;
    extern Bool appendLog;
    extern double tstep;
    extern unsigned int toll;

    if(!loadFile(FILE_SETTINGS, setBuff, FILE_SET_SIZE))
        return False;

    /* parse data */
    setProgramm(setBuff);
    /* set language */
    if(!getSetTokens()) return setError(E_SYNTAX, 15.01, "language");

    if(strcmp(leftTokenPtr, CONF_LANG) == 0) {
        //strncpy(lngFilePtr, tokenPtr, 2);
        /* copy 2 symbols */
        lngFilePtr[0] = tokenPtr[0];
        lngFilePtr[1] = tokenPtr[1];
    }
    else return setError(E_SYNTAX, 15.01, NULL);

    /* write log to file */
    if(!getSetTokens()) return setError(E_SYNTAX, 15.01, "log");

    if(strcmp(leftTokenPtr, CONF_ULOG) == 0) {
        if(strcmp(tokenPtr, "yes") == 0)
            writeLog = True;
        else if(strcmp(tokenPtr, "no") == 0)
            writeLog = False;
        else
            return setError(E_SYMANTIC, 15.01, "expected yes/no");
    }
    else return False;

    /* rewrite log */
    if(!getSetTokens()) return setError(E_SYNTAX, 15.01, "log");

    if(strcmp(leftTokenPtr, CONF_NLOG) == 0) {
        if(strcmp(tokenPtr, "no") == 0)
            appendLog = True;
        else if(strcmp(tokenPtr, "yes") == 0)
            appendLog = False;
        else
            return setError(E_SYMANTIC, 15.01, "expected yes/no");;
    }
    else return False;

    /* default time stamp */
    if(!getSetTokens()) return setError(E_SYNTAX, 15.01, "time");

    if(strcmp(leftTokenPtr, CONF_DT) == 0) {
        if(token_type == T_FLOAT)
            tstep = atof(tokenPtr);
        else if(token_type == T_INT)
            tstep = (double) atoi(tokenPtr);
        else
            return setError(E_SYMANTIC, 15.01, "unexpected type");
    }
    else return False;

    /* tolerance */
    if(!getSetTokens()) return setError(E_SYNTAX, 15.01, "tolerance");
    if(strcmp(leftTokenPtr, CONF_TOLL) == 0) {
        if(token_type == T_INT)
            toll = (unsigned int) atoi(tokenPtr);
        else
            return setError(E_SYMANTIC, 15.01, "unexpected type");
    }
    else return False;

    /* constants */
    while(getSetTokens()) {
        if(eval_num(&n)) {
            memAddConst(leftTokenPtr, n, True);
        }
    }

    return True;
}

/* register base elements */
int base_elements(void)
{
    regBlock();

    regCollect1();

    regCollect2();

    return 0;
}
#if 0
/* if last is coma */
char* notFinished(char* p)
{
    char *start = p, *end;
    /* find end */
    while(*p) p++;
    end = p;
    /* check last sign */
    p--;
    while(iswhite(*p)) p--;

    if(*p == ',')
        return end;

    return start;
}
#endif
int interactive_mode(void)
{
    int ans = 0, emptyRest = EMPTY_TIMES;
    char *pos;
    char interBuf[INTER_SIZE] = "";

    clear_console();
    print_head();
    execMode = False;
    interactiveRunning = True;

    if(setjmp(addNewErr)) {
        ans = 0;
        emptyRest = 1;
    }

    while(ans != 1 && emptyRest) {
        printf("%s>%c%c ", *interBuf ? "\n" : "",
                        emptyRest > 1 ? '>' : ' ', emptyRest > 2 ? '>' : ' ');
        //gets(interBuf);
        fgets(interBuf, INTER_SIZE, stdin);

        /* check enter complete */
        while((pos = isFinish(interBuf)) != interBuf) {
            //*pos++ = '\n';
            printf(NEXT_LINE);
            //gets(pos);
            fgets(pos, INTER_SIZE - ((int) (pos - interBuf)), stdin);
        }

        /* check exit */
        if(*interBuf)
            emptyRest = EMPTY_TIMES;
        else {
            emptyRest --;
            continue;
        }
        /* logging */
        logText(interBuf);

        ans = dlg_exec_code(interBuf);
    }

    printf("\n");
    u_print(stdout, lngText("i_bye", "Bye!"));
    printf("\n");

    return ans != -1 ? 0 : -1;
}

/* ERROR FUNCTION NUMBER - 2 */
int execute_mode(int argc, char* argv[])
{

    if(!loadFile(argv[1], progBuf, PROG_BUF_SIZE)) {
        setError(E_FOPEN_ERR, 15.02, argv[1]);
        return -1;
    }

    execMode = True;

    if(argc == 2)
        return dlg_exec_code(progBuf);

    return -1;
}

char* isFinish(char* line)
{
    u_char flOpen = 0;
    char *ptr = line, *back;
    /* zero string */
    if(*ptr == '\n') goto correct;
    /* check string closed */
    while(*ptr) {
        if(*ptr == '"')  flOpen = !flOpen;
        ptr ++;
    }
    if(flOpen) return ptr;
    /* check last is coma */
    back = ptr - 2;
    while(iswhite(*back)) back--;

    if(*back == ',' || *back == ';') return ptr;
    ptr --;

correct:
    *ptr = '\0';
    return line;

    //return (*back == ',' || *back == ';') ? ptr : line;
}
