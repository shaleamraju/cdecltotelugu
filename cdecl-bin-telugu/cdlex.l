%{
/* Lexical analyzer description for ANSI and C++ cdecl. */
/* The output of this file is included */
/* into the C file cdecl.c. */
char cdlexsccsid[] = "@(#)cdlex.l	2.2 3/30/88";
%}

N	[0-9]
A	[A-Z_a-z]
AN	[0-9A-Z_a-z]

%%
declare		return DECLARE;
exit		return 0;
explain		return EXPLAIN;
quit		return 0;
::		return DOUBLECOLON;
[?]		return HELP;
[,]		return COMMA;

auto		{ yylval.dynstr = ds(yytext);	return AUTO; }
char		{ yylval.dynstr = ds(yytext);	return CHAR; }
class		{ yylval.dynstr = ds(yytext);	return CLASS; }
const		{ yylval.dynstr = ds(yytext);	return CONSTVOLATILE; }
double		{ yylval.dynstr = ds(yytext);	return DOUBLE; }
enum		{ yylval.dynstr = ds(yytext);	return ENUM; }
extern		{ yylval.dynstr = ds(yytext);	return EXTERN; }
float		{ yylval.dynstr = ds(yytext);	return FLOAT; }
int		    { yylval.dynstr = ds(yytext);	return INT; }
long		{ yylval.dynstr = ds(yytext);	return LONG; }
noalias		{ yylval.dynstr = ds(yytext);	return CONSTVOLATILE; }
register	{ yylval.dynstr = ds(yytext);	return REGISTER; }
short		{ yylval.dynstr = ds(yytext);	return SHORT; }
signed		{ yylval.dynstr = ds(yytext);	return SIGNED; }
static		{ yylval.dynstr = ds(yytext);	return STATIC; }
struct		{ yylval.dynstr = ds(yytext);	return STRUCT; }
union		{ yylval.dynstr = ds(yytext);	return UNION; }
unsigned	{ yylval.dynstr = ds(yytext);	return UNSIGNED; }
void		{ yylval.dynstr = ds(yytext);	return VOID; }
volatile	{ yylval.dynstr = ds(yytext);	return CONSTVOLATILE; }

{A}{AN}*	{ yylval.dynstr = ds(yytext);	return NAME; }
{N}+		{ yylval.dynstr = ds(yytext);	return NUMBER; }

[\t ]		;
[&*[\]();^\n]	return *yytext;
.		{
			(void) printf("bad character '%s'\n",visible(*yytext));
			return *yytext;
		}
%%
