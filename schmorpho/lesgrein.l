%{

#include "CodeGen.h"
#include "thattari.hpp"

int line = 1;

static void copy() {
    yylval.str = new std::string(yytext, yyleng);
}

%}

  /* reglulegar skilgreiningar */

OPCHAR  [\+\|\-\*\/%\$!\_\:\<\>=&\?\#\^~@]
ALPHA   [a-zA-ZþæöðáéýúíóÞÆÖÐÁÉÝÚÍÓ]
DIGIT   [0-9]
INT     {DIGIT}+
FLOAT   {INT}\.{INT}|{INT}\.|\.{INT}
STRING  \"([^\"\\]|\\b|\\t|\\n|\\f|\\r|\\\"|\\\'|\\\\|(\\[0-3][0-7][0-7])|\\[0-7][0-7]|\\[0-7])*\"
CHAR    \'([^\'\\]|\\b|\\t|\\n|\\f|\\r|\\\"|\\\'|\\\\|(\\[0-3][0-7][0-7])|(\\[0-7][0-7])|(\\[0-7]))\'

VALUE {INT}|{FLOAT}|{STRING}|{CHAR}

NAME  ({ALPHA}|{OPCHAR})({DIGIT}|{ALPHA}|{OPCHAR})*

DELIM [\(\)\[\]{},.]

COMMENT ;.*

NEWLINE \r|\n|\r\n
WHITESPACE  [\n\r\ \t\b\012]

%%

"if"                      { copy(); return(IF); }
"define"                  { copy(); return(DEF); }
"null"|"true"|"false"     { copy(); return(VALUE); }

{DELIM}                   { copy(); return(yytext[0]); }
{NAME}                    { copy(); return(NAME); }
{VALUE}                   { copy(); return(VALUE); }
{NEWLINE}                 { line++; /* hunsa newline, en telja línur */}
{WHITESPACE}|{COMMENT}    { /* hunsa athugasemdir og whitespace */ }

.                         { copy(); return(ERR); }

%%
