%{
#include <stdint.h>
#include "lex.yy.h"

void yyerror(const char *s);
void p(const char *s, ...);
void pl(const char *s, ...);
#define p(s, ...) printf(s, ## __VA_ARGS__)
#define pl(s, ...) pind(); printf(s, ## __VA_ARGS__)
void pind();

uint32_t indent = 0;
uint32_t reg_no = 0;
%}

%union {
  uint32_t reg;
  int32_t inum;
  char *name;
}

%token T_KEY_RETURN T_KEY_INT
%token <name> T_IDENTIFIER
%token <inum> T_INTEGER_LITERAL
%type <inum> UnaryOp
%type <reg> PrimaryExp Exp AddExp MulExp UnaryExp

%%

CompUnit   
: FuncDef

FuncDef    
: { p("define "); reg_no = 0; }
  FuncType { p(" @"); }
  Ident { p("() "); }
  '(' ')' Block

FuncType   
: T_KEY_INT { p("i32"); }

Ident      
: T_IDENTIFIER { p($1); }

Block      
: '{' { p("{\n"); indent++; }
  Stmt { indent--; }
  '}' { p("}\n"); }

Stmt       
: T_KEY_RETURN Exp ';' { pl("ret i32 %%%d\n", $2); }

Exp        
: AddExp { $$ = $1; }

AddExp     
: MulExp { $$ = $1; }
| AddExp '+' MulExp {
    $$ = ++reg_no;
    pl("%%%d = add i32 %%%d, %%%d\n", $$, $1, $3);
  }
| AddExp '-' MulExp {
    $$ = ++reg_no;
    pl("%%%d = sub i32 %%%d, %%%d\n", $$, $1, $3);
  }

MulExp     
: UnaryExp { $$ = $1; }
| MulExp '*' UnaryExp {
    $$ = ++reg_no;
    pl("%%%d = mul i32 %%%d, %%%d\n", $$, $1, $3);
  }
| MulExp '/' UnaryExp {
    $$ = ++reg_no;
    pl("%%%d = sdiv i32 %%%d, %%%d\n", $$, $1, $3);
  }
| MulExp '%' UnaryExp {
    $$ = ++reg_no;
    pl("%%%d = srem i32 %%%d, %%%d\n", $$, $1, $3);
  }

UnaryExp   
: PrimaryExp { $$ = $1; }
| UnaryOp UnaryExp {
    if ($1 == 1) $$ = $2;
    else {
      $$ = ++reg_no;
      pl("%%%d = sub i32 0, %%%d\n", $$, $2);
    }
  }

PrimaryExp 
: '(' Exp ')' { $$ = $2; }
| T_INTEGER_LITERAL { pl("%%%d = add i32 0, %d\n", $$ = ++reg_no, $1); }

UnaryOp    
: '+' { $$ = 1; }
| '-' { $$ = -1; }

%%

void yyerror(const char *s) {
  fprintf(stderr, "%s\n", s);
}

void pind() {
  for (uint32_t i = 0; i < indent; i++) p("    ");
}