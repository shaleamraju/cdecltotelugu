

char cdeclsccsid[] = "@(#)cdecl.c	2.5 1/15/96";

int yylex(void);

#ifdef __EMSCRIPTEN__
static int isatty(int fd) {
  (void)fd;
  return 0;
}
static int fileno(void *file) {
  (void)file;
  return 0;
}
#endif

#include <ctype.h>
#include <stdio.h>
#if __STDC__ || defined(DOS)
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#else
#ifndef NOVARARGS
#include <varargs.h>
#endif
char *malloc();
void free(), exit(), perror();
#ifdef BSD
#include <strings.h>
extern int errno;
#define strrchr rindex
#define NOTMPFILE
#else
#include <errno.h>
#include <string.h>
#endif
#ifdef NOVOID
#define void int
#endif
#endif

#ifdef USE_READLINE
#include <readline/readline.h>

char *getline();
char **attempt_completion(char *, int, int);
char *keyword_completion(char *, int);
char *command_completion(char *, int);
#endif


#define MAX_NAME 32


char cdecl_prompt[MAX_NAME + 3];


char real_prompt[MAX_NAME + 3];

#define MB_SHORT 0001
#define MB_LONG 0002
#define MB_UNSIGNED 0004
#define MB_INT 0010
#define MB_CHAR 0020
#define MB_FLOAT 0040
#define MB_DOUBLE 0100
#define MB_VOID 0200
#define MB_SIGNED 0400

#define NullCP ((char *)NULL)
#ifdef dodebug
#define Debug(x)                                                               \
  do {                                                                         \
    if (DebugFlag)                                                             \
      (void)fprintf x;                                                         \
  } while (0)
#else
#define Debug(x)
#endif

#if __STDC__
char *ds(char *), *cat(char *, ...), *visible(int);
int main(int, char **);
int yywrap(void);
int dostdin(void);
void mbcheck(void), dohelp(void), usage(void);
void prompt(void), doprompt(void), noprompt(void);
void unsupp(char *, char *);
void notsupported(char *, char *, char *);
void yyerror(char *);
void doset(char *);
void dodeclare(char *, char *, char *, char *, char *);
void docast(char *, char *, char *, char *);
void dodexplain(char *, char *, char *, char *, char *);
void docexplain(char *, char *, char *, char *);
void cdecl_setprogname(char *);
int dotmpfile(int, char **), dofileargs(int, char **);
#else
char *ds(), *cat(), *visible();
int getopt();
void mbcheck(), dohelp(), usage();
void prompt(), doprompt(), noprompt();
void unsupp(), notsupported();
void yyerror();
void doset(), dodeclare(), docast(), dodexplain(), docexplain();
void cdecl_setprogname();
int dotmpfile(), dofileargs();
#endif
FILE *tmpfile();


unsigned modbits = 0;
int arbdims = 1;
char *savedname = 0;
char unknown_name[] = "unknown_name";
char prev = 0;
              
              
              
              
              
              
              
              
              
              


int RitchieFlag = 0;     
int MkProgramFlag = 0;   
int PreANSIFlag = 0;     
int CplusplusFlag = 0;   
int OnATty = 0;          
int Interactive = 0;     
int KeywordName = 0;     
char *progname = "cdecl";
int quiet = 0;           

#if dodebug
int DebugFlag = 0;
#endif

#ifdef doyydebug
#define YYDEBUG 1
#endif

#include "cdgram.c"
#include "cdlex.c"


#define ALWAYS 0
#define _ ALWAYS
#define NEVER 1
#define X NEVER
#define RITCHIE 2
#define R RITCHIE
#define PREANSI 3
#define P PREANSI
#define ANSI 4
#define A ANSI



char crosscheck[9][9] = {
   
    _,     _, _, _, _, _, _, _, _,
    _,      _, _, _, _, _, _, _, _,
    X,    _, _, _, _, _, _, _, _,
    X,     X, X, _, _, _, _, _, _,
    X,     X, X, X, _, _, _, _, _,
    R, _, R, R, X, _, _, _, _,
    P,   P, P, P, X, X, _, _, _,
    A,    X, X, X, X, X, X, _, _,
    P,   X, X, X, X, X, X, X, _};


struct {
  char *name;
  int bit;
} crosstypes[9] = {
    {"long", MB_LONG},     {"int", MB_INT},     {"short", MB_SHORT},
    {"char", MB_CHAR},     {"void", MB_VOID},   {"unsigned", MB_UNSIGNED},
    {"signed", MB_SIGNED}, {"float", MB_FLOAT}, {"double", MB_DOUBLE}};



void mbcheck() {
  register int i, j, restriction;
  char *t1, *t2;

 
 
  for (i = 1; i < 9; i++) {
   
    if ((modbits & crosstypes[i].bit) != 0) {
     
      for (j = 0; j < i; j++) {
       
        if (!(modbits & crosstypes[j].bit))
          continue;
       
        restriction = crosscheck[i][j];
        if (restriction == ALWAYS)
          continue;
        t1 = crosstypes[i].name;
        t2 = crosstypes[j].name;
        if (restriction == NEVER) {
          notsupported("", t1, t2);
        } else if (restriction == RITCHIE) {
          if (RitchieFlag)
            notsupported(" (Ritchie Compiler)", t1, t2);
        } else if (restriction == PREANSI) {
          if (PreANSIFlag || RitchieFlag)
            notsupported(" (Pre-ANSI Compiler)", t1, t2);
        } else if (restriction == ANSI) {
          if (!RitchieFlag && !PreANSIFlag)
            notsupported(" (ANSI Compiler)", t1, t2);
        } else {
          (void)fprintf(stderr, "%s: Internal error in crosscheck[%d,%d]=%d!\n",
                        progname, i, j, restriction);
          exit(1);
        }
      }
    }
  }
}


#undef _
#undef ALWAYS
#undef X
#undef NEVER
#undef R
#undef RITCHIE
#undef P
#undef PREANSI
#undef A
#undef ANSI

#ifdef USE_READLINE





char *commands[] = {"declare", "explain", "cast", "help",
                    "set",     "exit",    "quit", NULL};

char *keywords[] = {"function", "returning", "array",    "pointer", "reference",
                    "member",   "const",     "volatile", "noalias", "struct",
                    "union",    "enum",      "class",    "extern",  "static",
                    "auto",     "register",  "short",    "long",    "signed",
                    "unsigned", "char",      "float",    "double",  "void",
                    NULL};

char *options[] = {"options", "create",  "nocreate", "prompt",    "noprompt",
#if 0
  "interactive",
  "nointeractive",
#endif
                   "ritchie", "preansi", "ansi",     "cplusplus", NULL};


static char *line_read = NULL;


char *getline() {
 
  if (line_read != NULL) {
    free(line_read);
    line_read = NULL;
  }

 
  line_read = readline(cdecl_prompt);

 
  if (line_read && *line_read)
    add_history(line_read);

  return (line_read);
}

char **attempt_completion(char *text, int start, int end) {
  char **matches = NULL;

  if (start == 0)
    matches = completion_matches(text, command_completion);

  return matches;
}

char *command_completion(char *text, int flag) {
  static int index, len;
  char *command;

  if (!flag) {
    index = 0;
    len = strlen(text);
  }

  while (command = commands[index]) {
    index++;
    if (!strncmp(command, text, len))
      return strdup(command);
  }
  return NULL;
}

char *keyword_completion(char *text, int flag) {
  static int index, len, set, into;
  char *keyword, *option;

  if (!flag) {
    index = 0;
    len = strlen(text);
   
    set = !strncmp(rl_line_buffer, "set", 3);
    into = 0;
  }

  if (set) {
    while (option = options[index]) {
      index++;
      if (!strncmp(option, text, len))
        return strdup(option);
    }
  } else {
   
    if (!into) {
      into = 1;
      if (!strncmp(text, "into", len) && strncmp(text, "int", len))
        return strdup("into");
      if (strncmp(text, "int", len))
        return keyword_completion(text, into);
     
      if (!strncmp(rl_line_buffer, "cast", 4) &&
          !strstr(rl_line_buffer, "into"))
        return strdup("into");
      else
        return strdup("int");
    } else
      while (keyword = keywords[index]) {
        index++;
        if (!strncmp(keyword, text, len))
          return strdup(keyword);
      }
  }
  return NULL;
}
#endif



void unsupp(s, hint) char *s, *hint;
{
  notsupported("", s, NullCP);
  if (hint)
    (void)fprintf(stderr, "\t(maybe you mean \"%s\")\n", hint);
}



void notsupported(compiler, type1, type2) char *compiler, *type1, *type2;
{
  if (type2)
    (void)fprintf(stderr, "Warning: Unsupported in%s C%s -- '%s' with '%s'\n",
                  compiler, CplusplusFlag ? "++" : "", type1, type2);
  else
    (void)fprintf(stderr, "Warning: Unsupported in%s C%s -- '%s'\n", compiler,
                  CplusplusFlag ? "++" : "", type1);
}


void yyerror(s) char *s;
{
  (void)printf("%s\n", s);
  Debug((stdout, "yychar=%d\n", yychar));
}


int yywrap() { return 1; }


#if __STDC__
#define VA_DCL(type, var) (type var, ...)
#define VA_START(list, var, type) ((va_start(list, var)), (var))
#else
#if defined(DOS)
#define VA_DCL(type, var) (var, ...) type var;
#define VA_START(list, var, type) ((va_start(list, var)), (var))
#else
#ifndef NOVARARGS
#define VA_DCL(type, var) (va_alist) va_dcl
#define VA_START(list, var, type) ((va_start(list)), va_arg(list, type))
#else

#define VA_DCL(type, var) (var) type var;
#define VA_START(list, var, type) (list = (va_list) & (var), (var))
#define va_arg(list, type) ((type *)(list += sizeof(type)))[-1]
#define va_end(p)
typedef char *va_list;
#endif           
#endif           
#endif           


char *cat VA_DCL(char *, s1) {
  register char *newstr;
  register unsigned len = 1;
  char *str;
  va_list args;

 
  str = VA_START(args, s1, char *);
  for (; str; str = va_arg(args, char *))
    len += strlen(str);
  va_end(args);

 
  newstr = malloc(len);
  if (newstr == 0) {
    (void)fprintf(stderr, "%s: out of malloc space within cat()!\n", progname);
    exit(1);
  }
  newstr[0] = '\0';

 
  str = VA_START(args, s1, char *);
  for (; str; str = va_arg(args, char *)) {
    (void)strcat(newstr, str);
    free(str);
  }
  va_end(args);

  Debug((stderr, "\tcat created '%s'\n", newstr));
  return newstr;
}


char *ds(s)
char *s;
{
  register char *p = malloc((unsigned)(strlen(s) + 1));

  if (p)
    (void)strcpy(p, s);
  else {
    (void)fprintf(stderr, "%s: malloc() failed!\n", progname);
    exit(1);
  }
  return p;
}


char *visible(c)
int c;
{
  static char buf[5];

  c &= 0377;
  if (isprint(c)) {
    buf[0] = c;
    buf[1] = '\0';
  } else
    (void)sprintf(buf, "\\%03o", c);
  return buf;
}

#ifdef NOTMPFILE









static char *file4tmpfile = 0;

FILE *tmpfile() {
  static char *listtmpfiles[] = {"/usr/tmp/cdeclXXXXXX", "/tmp/cdeclXXXXXX",
                                 "/cdeclXXXXXX", "cdeclXXXXXX", 0};

  char **listp = listtmpfiles;
  for (; *listp; listp++) {
    FILE *retfp;
    (void)mktemp(*listp);
    retfp = fopen(*listp, "w+");
    if (!retfp)
      continue;
    file4tmpfile = *listp;
    return retfp;
  }

  return 0;
}

void rmtmpfile() {
  if (file4tmpfile)
    (void)unlink(file4tmpfile);
}
#else

#define rmtmpfile()
#endif             

#ifndef NOGETOPT
extern int optind;
#else




int optind = 1;

int getopt(argc, argv, optstring)
char **argv;
char *optstring;
{
  int ret;
  char *p;

  if ((argv[optind][0] != '-')
#ifdef DOS
      && (argv[optind][0] != '/')
#endif
  )
    return EOF;

  ret = argv[optind][1];
  optind++;

  for (p = optstring; *p; p++)
    if (*p == ret)
      return ret;

  (void)fprintf(stderr, "%s: illegal option -- %s\n", progname, visible(ret));

  return '?';
}
#endif


struct helpstruct {
  char *text;   
  char *cpptext;
} helptext[] =
    {/* up-to 23 lines of help text so it fits on (24x80) screens */
     {
         "[] means optional; {} means 1 or more; <> means defined elsewhere",
         0},
     {"  commands are separated by ';' and newlines", 0},
     {"command:", 0},
     {"  declare <name> as <english>", 0},
     {"  cast <name> into <english>", 0},
     {"  explain <gibberish>", 0},
     {"  set or set options", 0},
     {"  help, ?", 0},
     {"  quit or exit", 0},
     {"english:", 0},
     {"  function [( <decl-list> )] returning <english>", 0},
     {"  block [( <decl-list> )] returning <english>", 0},
     {"  array [<number>] of <english>", 0},
    
     {"  [{ const | volatile | noalias }] pointer to <english>",
      "  [{const|volatile}] {pointer|reference} to [member of class <name>] "
      "<english>"},
     {"  <type>", 0},
     {"type:", 0},
     {"  {[<storage-class>] [{<modifier>}] [<C-type>]}", 0},
    
     {"  { struct | union | enum } <name>",
      "  {struct|class|union|enum} <name>"},
    
     {"decllist: a comma separated list of <name>, <english> or <name> as "
      "<english>",
      0},
     {"name: a C identifier", 0},
    
     {"gibberish: a C declaration, like 'int *x', or cast, like '(int *)x'", 0},
     {"storage-class: extern, static, auto, register", 0},
     {"C-type: int, char, float, double, or void", 0},
    
     {"modifier: short, long, signed, unsigned, const, volatile, or noalias",
      "modifier: short, long, signed, unsigned, const, or volatile"},
     {0, 0}};


void dohelp() {
  register struct helpstruct *p;
  register char *fmt = CplusplusFlag ? " %s\n" : "  %s\n";

  for (p = helptext; p->text; p++)
    if (CplusplusFlag && p->cpptext)
      (void)printf(fmt, p->cpptext);
    else
      (void)printf(fmt, p->text);
}


void usage() {
  (void)fprintf(stderr, "Usage: %s [-r|-p|-a|-+] [-ciq%s%s] [files...]\n",
                progname,
#ifdef dodebug
                "d",
#else
                "",
#endif
#ifdef doyydebug
                "D"
#else
                ""
#endif
  );
  (void)fprintf(stderr, "\t-r Check against Ritchie PDP C Compiler\n");
  (void)fprintf(stderr, "\t-p Check against Pre-ANSI C Compiler\n");
  (void)fprintf(stderr, "\t-a Check against ANSI C Compiler%s\n",
                CplusplusFlag ? "" : " (the default)");
  (void)fprintf(stderr, "\t-+ Check against C++ Compiler%s\n",
                CplusplusFlag ? " (the default)" : "");
  (void)fprintf(stderr, "\t-c Create compilable output (include ; and {})\n");
  (void)fprintf(stderr, "\t-i Force interactive mode\n");
  (void)fprintf(stderr, "\t-q Quiet prompt\n");
#ifdef dodebug
  (void)fprintf(stderr, "\t-d Turn on debugging mode\n");
#endif
#ifdef doyydebug
  (void)fprintf(stderr, "\t-D Turn on YACC debugging mode\n");
#endif
  exit(1);
 
}


static int prompting;

void doprompt() { prompting = 1; }
void noprompt() { prompting = 0; }

void prompt() {
#ifndef USE_READLINE
  if ((OnATty || Interactive) && prompting) {
    (void)printf("%s", cdecl_prompt);
#if 0
	(void) printf("%s> ", progname);
#endif
    (void)fflush(stdout);
  }
#endif
}


void cdecl_setprogname(char *argv0) {
#ifdef DOS
  char *dot;
#endif

  progname = strrchr(argv0, '/');

#ifdef DOS
  if (!progname)
    progname = strrchr(argv0, '\\');
#endif

  if (progname)
    progname++;
  else
    progname = argv0;

#ifdef DOS
  dot = strchr(progname, '.');
  if (dot)
    *dot = '\0';
  for (dot = progname; *dot; dot++)
    *dot = tolower(*dot);
#endif
 
  {
    int len;

    len = strlen(progname);
    if (len > MAX_NAME)
      len = MAX_NAME;
    strncpy(real_prompt, progname, len);
    real_prompt[len] = '>';
    real_prompt[len + 1] = ' ';
    real_prompt[len + 2] = '\0';
  }
}




int namedkeyword(argn)
char *argn;
{
  static char *cmdlist[] = {"explain", "declare", "cast", "help",
                            "?",       "set",     0};

 
  char **cmdptr = cmdlist;
  for (; *cmdptr; cmdptr++)
    if (strcmp(*cmdptr, progname) == 0) {
      KeywordName = 1;
      return 1;
    }

 
  for (cmdptr = cmdlist; *cmdptr; cmdptr++)
    if (strcmp(*cmdptr, argn) == 0)
      return 1;

 
  return 0;
}



int dostdin() {
  int ret;
  if (OnATty || Interactive) {
#ifndef USE_READLINE
    if (!quiet)
      (void)printf("Type `help' or `?' for help\n");
    prompt();
#else
    char *line, *oldline;
    int len, newline;

    if (!quiet)
      (void)printf("Type `help' or `?' for help\n");
    ret = 0;
    while ((line = getline())) {
      if (!strcmp(line, "quit") || !strcmp(line, "exit")) {
        free(line);
        return ret;
      }
      newline = 0;
     
      len = strlen(line);
      if (len && line[len - 1] != '\n' && line[len - 1] != ';') {
        newline = 1;
        oldline = line;
        line = malloc(len + 2);
        strcpy(line, oldline);
        line[len] = ';';
        line[len + 1] = '\0';
      }
      if (len)
        ret = dotmpfile_from_string(line);
      if (newline)
        free(line);
    }
    puts("");
    return ret;
#endif
  }

  yyin = stdin;
  ret = yyparse();
  OnATty = 0;
  return ret;
}


int dotmpfile_from_string(s) const char *s;
{
  int ret = 0;
  FILE *tmpfp = tmpfile();
  if (!tmpfp) {
    int sverrno = errno;
    (void)fprintf(stderr, "%s: cannot open temp file\n", progname);
    errno = sverrno;
    perror(progname);
    return 1;
  }

  if (fputs(s, tmpfp) == EOF) {
    int sverrno;
    sverrno = errno;
    (void)fprintf(stderr, "%s: error writing to temp file\n", progname);
    errno = sverrno;
    perror(progname);
    (void)fclose(tmpfp);
    rmtmpfile();
    return 1;
  }

  rewind(tmpfp);
  yyin = tmpfp;
  ret += yyparse();
  (void)fclose(tmpfp);
  rmtmpfile();

  return ret;
}



int dotmpfile(argc, argv)
int argc;
char **argv;
{
  int ret = 0;
  FILE *tmpfp = tmpfile();
  if (!tmpfp) {
    int sverrno = errno;
    (void)fprintf(stderr, "%s: cannot open temp file\n", progname);
    errno = sverrno;
    perror(progname);
    return 1;
  }

  if (KeywordName)
    if (fputs(progname, tmpfp) == EOF) {
      int sverrno;
    errwrite:
      sverrno = errno;
      (void)fprintf(stderr, "%s: error writing to temp file\n", progname);
      errno = sverrno;
      perror(progname);
      (void)fclose(tmpfp);
      rmtmpfile();
      return 1;
    }

  for (; optind < argc; optind++)
    if (fprintf(tmpfp, " %s", argv[optind]) == EOF)
      goto errwrite;

  if (putc('\n', tmpfp) == EOF)
    goto errwrite;

  rewind(tmpfp);
  yyin = tmpfp;
  ret += yyparse();
  (void)fclose(tmpfp);
  rmtmpfile();

  return ret;
}


int dofileargs(argc, argv)
int argc;
char **argv;
{
  FILE *ifp;
  int ret = 0;

  for (; optind < argc; optind++)
    if (strcmp(argv[optind], "-") == 0)
      ret += dostdin();

    else if ((ifp = fopen(argv[optind], "r")) == NULL) {
      int sverrno = errno;
      (void)fprintf(stderr, "%s: cannot open %s\n", progname, argv[optind]);
      errno = sverrno;
      perror(argv[optind]);
      ret++;
    }

    else {
      yyin = ifp;
      ret += yyparse();
    }

  return ret;
}


void docast(name, left, right, type) char *name, *left, *right, *type;
{
  int lenl = strlen(left), lenr = strlen(right);

  if (prev == 'f')
    unsupp("Cast into function", "cast into pointer to function");
  else if (prev == 'A' || prev == 'a')
    unsupp("Cast into array", "cast into pointer");
  (void)printf("(%s%*s%s)%s\n", type, lenl + lenr ? lenl + 1 : 0, left, right,
               name ? name : "expression");
  free(left);
  free(right);
  free(type);
  if (name)
    free(name);
}


void dodeclare(name, storage, left, right, type) char *name, *storage, *left,
    *right, *type;
{
  if (prev == 'v')
    unsupp("Variable of type void", "variable of type pointer to void");

  if (*storage == 'r')
    switch (prev) {
    case 'f':
      unsupp("Register function", NullCP);
      break;
    case 'A':
    case 'a':
      unsupp("Register array", NullCP);
      break;
    case 's':
      unsupp("Register struct/class", NullCP);
      break;
    }

  if (*storage)
    (void)printf("%s ", storage);
  (void)printf("%s %s%s%s", type, left,
               name            ? name
               : (prev == 'f') ? "f"
                               : "var",
               right);
  if (MkProgramFlag) {
    if ((prev == 'f') && (*storage != 'e'))
      (void)printf(" { }\n");
    else
      (void)printf(";\n");
  } else {
    (void)printf("\n");
  }
  free(storage);
  free(left);
  free(right);
  free(type);
  if (name)
    free(name);
}

void dodexplain(storage, constvol1, constvol2, type, decl) char *storage,
    *constvol1, *constvol2, *type, *decl;
{
  if (type && (strcmp(type, "void") == 0)) {
    if (prev == 'n')
      unsupp("Variable of type void", "variable of type pointer to void");
    else if (prev == 'a')
      unsupp("array of type void", "array of type pointer to void");
    else if (prev == 'r')
      unsupp("reference to type void", "pointer to void");
  }

  if (*storage == 'r')
    switch (prev) {
    case 'f':
      unsupp("Register function", NullCP);
      break;
    case 'A':
    case 'a':
      unsupp("Register array", NullCP);
      break;
    case 's':
      unsupp("Register struct/union/enum/class", NullCP);
      break;
    }

  (void)printf("%s as1 ^", savedname);
  if (*storage)
    (void)printf("%s2 ", storage);
  if (*constvol1)
    (void)printf("%s4 ", constvol1);
  if (*constvol2)
    (void)printf("%s5 ", constvol2);
  (void)printf("%s$ ", type ? type : "int");
  (void)printf("%s3 ", decl);
  (void)printf("declare\n");
}

void docexplain(constvol, type, cast, name) char *constvol, *type, *cast, *name;
{
  if (strcmp(type, "void") == 0) {
    if (prev == 'a')
      unsupp("array of type void", "array of type pointer to void");
    else if (prev == 'r')
      unsupp("reference to type void", "pointer to void");
  }
  if (strlen(constvol) > 0)
    (void)printf("%s1 ", constvol);
  (void)printf("%s$ ", type);
  (void)printf("%s7 ", cast);
  (void)printf("%s 0into4 ", name);
  (void)printf("cast\n");
}


void doset(opt) char *opt;
{
  if (strcmp(opt, "create") == 0) {
    MkProgramFlag = 1;
  } else if (strcmp(opt, "nocreate") == 0) {
    MkProgramFlag = 0;
  } else if (strcmp(opt, "prompt") == 0) {
    prompting = 1;
    strcpy(cdecl_prompt, real_prompt);
  } else if (strcmp(opt, "noprompt") == 0) {
    prompting = 0;
    cdecl_prompt[0] = '\0';
  }
#ifndef USE_READLINE
 
  else if (strcmp(opt, "interactive") == 0) {
    Interactive = 1;
  } else if (strcmp(opt, "nointeractive") == 0) {
    Interactive = 0;
    OnATty = 0;
  }
#endif
  else if (strcmp(opt, "ritchie") == 0) {
    CplusplusFlag = 0;
    RitchieFlag = 1;
    PreANSIFlag = 0;
  } else if (strcmp(opt, "preansi") == 0) {
    CplusplusFlag = 0;
    RitchieFlag = 0;
    PreANSIFlag = 1;
  } else if (strcmp(opt, "ansi") == 0) {
    CplusplusFlag = 0;
    RitchieFlag = 0;
    PreANSIFlag = 0;
  } else if (strcmp(opt, "cplusplus") == 0) {
    CplusplusFlag = 1;
    RitchieFlag = 0;
    PreANSIFlag = 0;
  }
#ifdef dodebug
  else if (strcmp(opt, "debug") == 0) {
    DebugFlag = 1;
  } else if (strcmp(opt, "nodebug") == 0) {
    DebugFlag = 0;
  }
#endif
#ifdef doyydebug
  else if (strcmp(opt, "yydebug") == 0) {
    yydebug = 1;
  } else if (strcmp(opt, "noyydebug") == 0) {
    yydebug = 0;
  }
#endif
  else {
    if ((strcmp(opt, unknown_name) != 0) && (strcmp(opt, "options") != 0))
      (void)printf("Unknown set option: '%s'\n", opt);

    (void)printf("Valid set options (and command line equivalents) are:\n");
    (void)printf("\toptions\n");
    (void)printf("\tcreate (-c), nocreate\n");
    (void)printf("\tprompt, noprompt (-q)\n");
#ifndef USE_READLINE
    (void)printf("\tinteractive (-i), nointeractive\n");
#endif
    (void)printf("\tritchie (-r), preansi (-p), ansi (-a) or cplusplus (-+)\n");
#ifdef dodebug
    (void)printf("\tdebug (-d), nodebug\n");
#endif
#ifdef doyydebug
    (void)printf("\tyydebug (-D), noyydebug\n");
#endif

    (void)printf("\nCurrent set values are:\n");
    (void)printf("\t%screate\n", MkProgramFlag ? "   " : " no");
    (void)printf("\t%sprompt\n", cdecl_prompt[0] ? "   " : " no");
    (void)printf("\t%sinteractive\n", (OnATty || Interactive) ? "   " : " no");
    if (RitchieFlag)
      (void)printf("\t   ritchie\n");
    else
      (void)printf("\t(noritchie)\n");
    if (PreANSIFlag)
      (void)printf("\t   preansi\n");
    else
      (void)printf("\t(nopreansi)\n");
    if (!RitchieFlag && !PreANSIFlag && !CplusplusFlag)
      (void)printf("\t   ansi\n");
    else
      (void)printf("\t(noansi)\n");
    if (CplusplusFlag)
      (void)printf("\t   cplusplus\n");
    else
      (void)printf("\t(nocplusplus)\n");
#ifdef dodebug
    (void)printf("\t%sdebug\n", DebugFlag ? "   " : " no");
#endif
#ifdef doyydebug
    (void)printf("\t%syydebug\n", yydebug ? "   " : " no");
#endif
  }
}

void versions() {
  (void)printf("Version:\n\t%s\n\t%s\n\t%s\n", cdeclsccsid, cdgramsccsid,
               cdlexsccsid);
  exit(0);
}

#ifdef __EMSCRIPTEN__
int run_from_js(const char *txt) { return dotmpfile_from_string(txt); }
#else
int main(argc, argv)
char **argv;
{
  int c, ret = 0;

#ifdef USE_READLINE
 
  rl_attempted_completion_function = (CPPFunction *)attempt_completion;
  rl_completion_entry_function = (Function *)keyword_completion;
#endif

  cdecl_setprogname(argv[0]);
#ifdef DOS
  if (strcmp(progname, "cppdecl") == 0)
#else
  if (strcmp(progname, "c++decl") == 0)
#endif
    CplusplusFlag = 1;

  prompting = OnATty = isatty(0);
  while ((c = getopt(argc, argv, "cipqrpa+dDV")) != EOF)
    switch (c) {
    case 'c':
      MkProgramFlag = 1;
      break;
    case 'i':
      Interactive = 1;
      doprompt();
      break;
    case 'q':
      quiet = 1;
      noprompt();
      break;

   
   
    case 'r':
      CplusplusFlag = 0;
      RitchieFlag = 1;
      PreANSIFlag = 0;
      break;
    case 'p':
      CplusplusFlag = 0;
      RitchieFlag = 0;
      PreANSIFlag = 1;
      break;
    case 'a':
      CplusplusFlag = 0;
      RitchieFlag = 0;
      PreANSIFlag = 0;
      break;
    case '+':
      CplusplusFlag = 1;
      RitchieFlag = 0;
      PreANSIFlag = 0;
      break;

#ifdef dodebug
    case 'd':
      DebugFlag = 1;
      break;
#endif
#ifdef doyydebug
    case 'D':
      yydebug = 1;
      break;
#endif
    case 'V':
      versions();
      break;
    case '?':
      usage();
      break;
    }

 
  if (prompting)
    strcpy(cdecl_prompt, real_prompt);
  else
    cdecl_prompt[0] = '\0';

 

 
  if (optind == argc)
    ret += dostdin();

 
 
 
  else if (namedkeyword(argv[optind]))
    ret += dotmpfile(argc, argv);
  else
    ret += dofileargs(argc, argv);

  exit(ret);
 
}
#endif