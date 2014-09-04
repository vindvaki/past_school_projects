%{

#include <cstdio>
#include "CodeGen.h"

extern int yylex();
void yyerror(char *);

%}

%union {
    std::string *str;
    std::vector<std::string *> *strvec;
    std::vector<EDefFun *> *defunvec;
    std::vector<EDefVar *> *defvarvec;
    std::vector<EExpr *> *exprvec;
    EDefFun *defun;
    EDefVar *defvar;
    BodyData *body;
    EExpr *expr;
    int token;
}

%token <token> IN SHOW DEF IF LAMBDA 
%token <str> NAME VALUE ERR

%type <defun> defun
%type <defvar> defvar
%type <strvec> args
%type <body> body
%type <expr> expr cexpr vexpr
%type <exprvec> exprs
%type <defunvec> defuns
%type <defvarvec> defvars

%error-verbose

%start program
%%

program 
    : /**/
    | program defun { $2->generateFun(std::cout); delete $2; }
    ; 

defun
    : '(' DEF '(' NAME args ')' body ')' { $$ = new EDefFun($4, $5, $7); delete $4,$5,$7; }
    ;
defvar
    : '(' DEF NAME expr ')' { $$ = new EDefVar($3, $4); delete $3; }
    ;

args 
    : /**/        { $$ = new std::vector<std::string *>; }
    | args NAME   { $1->push_back($2); $$ = $1; }
    ;

body /* |body| er flóknara en ég hefði viljað en svona fæst a.m.k. ótvíræðni */
	: expr { $$ = new BodyData(); $$->funvec=0; $$->varvec=0; $$->expr=$1; }
	| defuns expr  { $$ = new BodyData(); $$->funvec = $1; $$->varvec = 0; $$->expr = $2; }
	| defvars expr { $$ = new BodyData(); $$->funvec = 0; $$->varvec = $1; $$->expr = $2; }
	| defuns defvars expr { $$ = new BodyData(); $$->funvec = $1; $$->varvec = $2; $$->expr = $3; }
	;

defuns
      : defun        { $$ = new std::vector<EDefFun *>(); $$->push_back($1); }
      | defuns defun { $1->push_back($2); $$ = $1; }
      ;

defvars
       : defvar         { $$ = new std::vector<EDefVar *>(); $$->push_back($1); }
       | defvars defvar { $1->push_back($2); $$ = $1; }
       ;

exprs
    : /**/       { $$ = new std::vector<EExpr *>; }
    | exprs expr { $1->push_back($2); $$ = $1; }
    ;

expr
    : vexpr
    | cexpr
    ;

cexpr
    : '(' NAME exprs ')'   { $$ = new ENameCall($2, $3); delete $2,$3; }
    | '(' cexpr exprs ')'  { $$ = new EExprCall($2, $3); delete $3; }
    | '(' IF expr expr expr ')' { $$ = new EIf($3, $4, $5); }
    ;

vexpr
    : NAME  { $$ = new EVar($1); delete $1; }
    | VALUE { $$ = new EVal($1); delete $1; }
    ;

%%

void yyerror(char *s) { printf("ERROR: %s\n", s); }

int main(int argc, char **argv) {
  switch(argc) {
  case 2: // venjuleg eining
    {
      std::string mmod (argv[1]);
      std::cout << "\"" << mmod << ".mmod\" = " << std::endl;
      std::cout << " ! {{" << std::endl;
      yyparse();
      std::cout << "}} * BASIS" << std::endl;
      break;
    }
  case 3: // keyrslueining
    {
      std::string mexe (argv[1]); // nafnið á keyrslueiningunni
      std::string start(argv[2]); // fallið sem keyrt er
      std::cout << "\"" << mexe << ".mexe\" = " << start << " in " << std::endl;
      std::cout << "! {{" << std::endl;
      yyparse();
      std::cout << "}} * BASIS ; " << std::endl;
      break;
    }
  default: // hrá þula
    yyparse();
    break;
  }
  return 0;
}
