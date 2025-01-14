%skeleton "lalr1.cc" /* -*- c++ -*- */
%require "3.0"
%defines
//%define parser_class_name {SysYFParser}
%define api.parser.class {SysYFParser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
#include <string>
#include "SyntaxTree.h"
class SysYFDriver;
}

// The parsing context.
%param { SysYFDriver& driver }

// Location tracking
%locations
%initial-action
{
// Initialize the initial location.
@$.begin.filename = @$.end.filename = &driver.file;
};

// Enable tracing and verbose errors (which may be wrong!)
%define parse.trace
%define parse.error verbose

// Parser needs to know about the driver:
%code
{
#include "SysYFDriver.h"
#define yylex driver.lexer.yylex
}

// Tokens:
%define api.token.prefix {TOK_}

%token END
%token ERROR 258 PLUS 259 MINUS 260 MULTIPLY 261 DIVIDE 262 MODULO 263 LTE 264
%token GT 265 GTE 266 EQ 267 NEQ 268 ASSIGN 269 SEMICOLON 270 
%token COMMA 271 LPARENTHESE 272 RPARENTHESE 273 LBRACKET 274 
%token RBRACKET 275 LBRACE 276 RBRACE 277 ELSE 278 IF 279
%token INT 280 RETURN 281 VOID 282 WHILE 283 
%token <std::string>IDENTIFIER 284
%token <float>FLOATCONST 285
%token <int>INTCONST 286
%token LETTER 287 EOL 288 COMMENT 289 
%token BLANK 290 CONST 291 BREAK 292 CONTINUE 293 NOT 294
%token AND 295 OR 296 MOD 297
%token FLOAT 298
%token LOGICAND 299
%token LOGICOR 300
%token LT 301
%token <std::string>STRINGCONST 302
%token LRBRACKET 303
// Use variant-based semantic values: %type and %token expect genuine types

%type <SysYF::Ptr<SysYF::SyntaxTree::Assembly>>CompUnit
%type <SysYF::PtrVec<SysYF::SyntaxTree::GlobalDef>>GlobalDecl
%type <SysYF::PtrVec<SysYF::SyntaxTree::VarDef>>ConstDecl
%type <SysYF::PtrVec<SysYF::SyntaxTree::VarDef>>ConstDefList
%type <SysYF::SyntaxTree::Type>BType
%type <SysYF::Ptr<SysYF::SyntaxTree::VarDef>>ConstDef
%type <SysYF::PtrVec<SysYF::SyntaxTree::VarDef>>VarDecl
%type <SysYF::PtrVec<SysYF::SyntaxTree::VarDef>>VarDefList
%type <SysYF::Ptr<SysYF::SyntaxTree::VarDef>>VarDef
%type <SysYF::PtrVec<SysYF::SyntaxTree::Expr>>ArrayExpList
%type <SysYF::Ptr<SysYF::SyntaxTree::InitVal>>InitVal
%type <SysYF::Ptr<SysYF::SyntaxTree::InitVal>>InitValList
%type <SysYF::Ptr<SysYF::SyntaxTree::InitVal>>CommaInitValList
%type <SysYF::PtrVec<SysYF::SyntaxTree::Expr>>ExpList
%type <SysYF::PtrVec<SysYF::SyntaxTree::Expr>>CommaExpList
%type <SysYF::Ptr<SysYF::SyntaxTree::FuncDef>>FuncDef
%type <SysYF::Ptr<SysYF::SyntaxTree::BlockStmt>>Block
%type <SysYF::PtrVec<SysYF::SyntaxTree::Stmt>>BlockItemList
%type <SysYF::PtrVec<SysYF::SyntaxTree::Stmt>>BlockItem
%type <SysYF::Ptr<SysYF::SyntaxTree::Stmt>>Stmt
%type <SysYF::Ptr<SysYF::SyntaxTree::Expr>>OptionRet
%type <SysYF::Ptr<SysYF::SyntaxTree::LVal>>LVal
%type <SysYF::Ptr<SysYF::SyntaxTree::Expr>>Exp
%type <SysYF::Ptr<SysYF::SyntaxTree::Literal>>Number

%type <SysYF::Ptr<SysYF::SyntaxTree::FuncParam>>FuncFParam
%type <SysYF::PtrVec<SysYF::SyntaxTree::FuncParam>>FParamList
%type <SysYF::PtrVec<SysYF::SyntaxTree::FuncParam>>CommaFParamList
%type <SysYF::Ptr<SysYF::SyntaxTree::Expr>>RelExp
%type <SysYF::Ptr<SysYF::SyntaxTree::Expr>>EqExp
%type <SysYF::Ptr<SysYF::SyntaxTree::Expr>>LAndExp
%type <SysYF::Ptr<SysYF::SyntaxTree::Expr>>LOrExp
%type <SysYF::Ptr<SysYF::SyntaxTree::Expr>>CondExp
%type <SysYF::Ptr<SysYF::SyntaxTree::Stmt>>IfStmt

// No %destructors are needed, since memory will be reclaimed by the
// regular destructors.
/* %printer { yyoutput << $$; } <*>; */   

// Grammar:
%start Begin 

%%
Begin: CompUnit END {
    $1->loc = @$;
    driver.root = $1;
    return 0;
  }
  ;

CompUnit:CompUnit GlobalDecl{
		$1->global_defs.insert($1->global_defs.end(), $2.begin(), $2.end());
		$$=$1;
	} 
	| GlobalDecl{
		$$=SysYF::Ptr<SysYF::SyntaxTree::Assembly>(new SysYF::SyntaxTree::Assembly());
		$$->global_defs.insert($$->global_defs.end(), $1.begin(), $1.end());
  }
	;

GlobalDecl:ConstDecl{
    $$=SysYF::PtrVec<SysYF::SyntaxTree::GlobalDef>();
    $$.insert($$.end(), $1.begin(), $1.end());
  }
	| VarDecl{
    $$=SysYF::PtrVec<SysYF::SyntaxTree::GlobalDef>();
    $$.insert($$.end(), $1.begin(), $1.end());
  }
  | FuncDef{
    $$=SysYF::PtrVec<SysYF::SyntaxTree::GlobalDef>();
    $$.push_back(SysYF::Ptr<SysYF::SyntaxTree::GlobalDef>($1));
  }
	;

ConstDecl:CONST BType ConstDefList SEMICOLON{
    $$=$3;
    for (auto &node : $$) {
      node->btype = $2;
    }
  }
	;
ConstDefList:ConstDefList COMMA ConstDef{
    $1.push_back(SysYF::Ptr<SysYF::SyntaxTree::VarDef>($3));
    $$=$1;
  }
	| ConstDef{
    $$=SysYF::PtrVec<SysYF::SyntaxTree::VarDef>();
    $$.push_back(SysYF::Ptr<SysYF::SyntaxTree::VarDef>($1));
  }
	;

BType:INT{
  $$=SysYF::SyntaxTree::Type::INT;
  }
  | FLOAT {
  $$=SysYF::SyntaxTree::Type::FLOAT;
  }
;


ConstDef:IDENTIFIER ArrayExpList ASSIGN InitVal{
    $$=SysYF::Ptr<SysYF::SyntaxTree::VarDef>(new SysYF::SyntaxTree::VarDef());
    $$->is_constant = true;
    $$->is_inited = true;
    $$->name=$1;
    $$->array_length = $2;
    $$->initializers = SysYF::Ptr<SysYF::SyntaxTree::InitVal>($4);
    $$->loc = @$;
  }
	;

VarDecl:BType VarDefList SEMICOLON{
    $$=$2;
    for (auto &node : $$) {
      node->btype = $1;
    }
  }
	;

VarDefList:VarDefList COMMA VarDef{
    $1.push_back(SysYF::Ptr<SysYF::SyntaxTree::VarDef>($3));
    $$=$1;
  }
	| VarDef{
    $$=SysYF::PtrVec<SysYF::SyntaxTree::VarDef>();
    $$.push_back(SysYF::Ptr<SysYF::SyntaxTree::VarDef>($1));
  }
	;

VarDef:IDENTIFIER ArrayExpList{
    $$=SysYF::Ptr<SysYF::SyntaxTree::VarDef>(new SysYF::SyntaxTree::VarDef());
    $$->name=$1;
    $$->array_length = $2;
    $$->is_inited = false;
    $$->loc = @$;
  }
	| IDENTIFIER ArrayExpList ASSIGN InitVal{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::VarDef>(new SysYF::SyntaxTree::VarDef());
    $$->name = $1;
    $$->array_length = $2;
    $$->initializers = SysYF::Ptr<SysYF::SyntaxTree::InitVal>($4);
    $$->is_inited = true;
    $$->loc = @$;
  }
	;

ArrayExpList:ArrayExpList LBRACKET Exp RBRACKET{
    $1.push_back(SysYF::Ptr<SysYF::SyntaxTree::Expr>($3));
    $$=$1;
  }
	| %empty{
    $$=SysYF::PtrVec<SysYF::SyntaxTree::Expr>();
  }
  ;

InitVal: Exp{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::InitVal>(new SysYF::SyntaxTree::InitVal());
    $$->isExp = true;
    $$->elementList = SysYF::PtrVec<SysYF::SyntaxTree::InitVal>();
    $$->expr = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    $$->loc = @$;
  }
  | LBRACE InitValList RBRACE{
    $$ = $2;
  }
  ;

InitValList: CommaInitValList InitVal{
    $1->elementList.push_back(SysYF::Ptr<SysYF::SyntaxTree::InitVal>($2));
    $$ = $1;
  }
  | %empty{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::InitVal>(new SysYF::SyntaxTree::InitVal());
    $$->isExp = false;
    $$->elementList = SysYF::PtrVec<SysYF::SyntaxTree::InitVal>();
    //$$->elementList = std::vector<SysYF::Ptr<SysYF::SyntaxTree::InitVal>>();
    $$->expr = nullptr;
    $$->loc = @$;
  }
  ;

CommaInitValList: CommaInitValList InitVal COMMA{
    $1->elementList.push_back(SysYF::Ptr<SysYF::SyntaxTree::InitVal>($2));
    $$ = $1;
  }
  | %empty{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::InitVal>(new SysYF::SyntaxTree::InitVal());
    $$->isExp = false;
    $$->elementList = SysYF::PtrVec<SysYF::SyntaxTree::InitVal>();
    //$$->elementList = std::vector<SysYF::Ptr<SysYF::SyntaxTree::InitVal>>();
    $$->expr = nullptr;
    $$->loc = @$;
  }
  ;

ExpList:CommaExpList Exp{
    $1.push_back(SysYF::Ptr<SysYF::SyntaxTree::Expr>($2));
    $$ = $1;
  }
  | %empty{
    $$ = SysYF::PtrVec<SysYF::SyntaxTree::Expr>();
  }
	;

CommaExpList:CommaExpList Exp COMMA{
    $1.push_back(SysYF::Ptr<SysYF::SyntaxTree::Expr>($2));
    $$ = $1;
  }
  | %empty{
    $$ = SysYF::PtrVec<SysYF::SyntaxTree::Expr>();
  }
  ;


FuncFParam:BType IDENTIFIER ArrayExpList{
  $$ = SysYF::Ptr<SysYF::SyntaxTree::FuncParam>(new SysYF::SyntaxTree::FuncParam());
  $$->param_type = $1;
  $$->name = $2;
  $$->array_index = $3;
  $$->loc = @$;
}
| BType IDENTIFIER LRBRACKET ArrayExpList{
   $$ = SysYF::Ptr<SysYF::SyntaxTree::FuncParam>(new SysYF::SyntaxTree::FuncParam());
   $$->param_type = $1;
   $$->name = $2;
   $$->array_index = $4;
   $$->array_index.insert($$->array_index.begin(),nullptr);
   $$->loc = @$;
}
;

FParamList: CommaFParamList FuncFParam{
  $1.push_back(SysYF::Ptr<SysYF::SyntaxTree::FuncParam>($2));
  $$ = $1;
}
| %empty{
  $$ = SysYF::PtrVec<SysYF::SyntaxTree::FuncParam>();
}
;

CommaFParamList:CommaFParamList FuncFParam COMMA{
  $1.push_back(SysYF::Ptr<SysYF::SyntaxTree::FuncParam>($2));
  $$ = $1;
}
| %empty{
  $$ = SysYF::PtrVec<SysYF::SyntaxTree::FuncParam>();
}
;
FuncDef:BType IDENTIFIER LPARENTHESE FParamList RPARENTHESE Block{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::FuncDef>(new SysYF::SyntaxTree::FuncDef());
    $$->ret_type = $1;
    $$->name = $2;
    auto tmp = SysYF::Ptr<SysYF::SyntaxTree::FuncFParamList>(new SysYF::SyntaxTree::FuncFParamList());
    tmp->params = $4;
    $$->param_list = SysYF::Ptr<SysYF::SyntaxTree::FuncFParamList>(tmp);
    $$->body = SysYF::Ptr<SysYF::SyntaxTree::BlockStmt>($6);
    $$->loc = @$;
  }
  | VOID IDENTIFIER LPARENTHESE FParamList RPARENTHESE Block{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::FuncDef>(new SysYF::SyntaxTree::FuncDef());
    $$->ret_type = SysYF::SyntaxTree::Type::VOID;
    $$->name = $2;
    auto tmp = SysYF::Ptr<SysYF::SyntaxTree::FuncFParamList>(new SysYF::SyntaxTree::FuncFParamList());
    tmp->params = $4;
    $$->param_list = SysYF::Ptr<SysYF::SyntaxTree::FuncFParamList>(tmp);
    $$->body = SysYF::Ptr<SysYF::SyntaxTree::BlockStmt>($6);
    $$->loc = @$;
  }
  ;

Block:LBRACE BlockItemList RBRACE{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::BlockStmt>(new SysYF::SyntaxTree::BlockStmt());
    $$->body = $2;
    $$->loc = @$;
  }
  ;

BlockItemList:BlockItemList BlockItem{
    $1.insert($1.end(), $2.begin(), $2.end());
    $$ = std::move($1);
  }
  | %empty{
    $$ = SysYF::PtrVec<SysYF::SyntaxTree::Stmt>();
  }
  ;

BlockItem:VarDecl{
    $$ = SysYF::PtrVec<SysYF::SyntaxTree::Stmt>();
    $$.insert($$.end(), $1.begin(), $1.end());
  }
  | ConstDecl{
    $$ = SysYF::PtrVec<SysYF::SyntaxTree::Stmt>();
    $$.insert($$.end(), $1.begin(), $1.end());
  }
  | Stmt{
    $$ = SysYF::PtrVec<SysYF::SyntaxTree::Stmt>();
    $$.push_back(SysYF::Ptr<SysYF::SyntaxTree::Stmt>($1));
  }
  ;

Stmt:LVal ASSIGN Exp SEMICOLON{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::AssignStmt>(new SysYF::SyntaxTree::AssignStmt());
    temp->target = SysYF::Ptr<SysYF::SyntaxTree::LVal>($1);
    temp->value = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  | Exp SEMICOLON{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::ExprStmt>(new SysYF::SyntaxTree::ExprStmt());
    temp->exp = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    $$ = temp;
    $$->loc = @$;
  }
  | RETURN OptionRet SEMICOLON{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::ReturnStmt>(new SysYF::SyntaxTree::ReturnStmt());
    temp->ret = SysYF::Ptr<SysYF::SyntaxTree::Expr>($2);
    $$ = temp;
    $$->loc = @$;
  }
  | Block{
    $$ = $1;
  }
  | WHILE LPARENTHESE CondExp RPARENTHESE Stmt{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::WhileStmt>(new SysYF::SyntaxTree::WhileStmt());
    temp->cond_exp = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    temp->statement = SysYF::Ptr<SysYF::SyntaxTree::Stmt>($5);
    $$ = temp;
    $$->loc = @$;
  }
  | IfStmt {
    $$ = $1;
  }
  | BREAK SEMICOLON {
    $$ = SysYF::Ptr<SysYF::SyntaxTree::BreakStmt>(new SysYF::SyntaxTree::BreakStmt());
    $$->loc = @$;
  }
  | CONTINUE SEMICOLON {
    $$ = SysYF::Ptr<SysYF::SyntaxTree::ContinueStmt>(new SysYF::SyntaxTree::ContinueStmt());
    $$->loc = @$;
  }
  | SEMICOLON{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::EmptyStmt>(new SysYF::SyntaxTree::EmptyStmt());
    $$->loc = @$;
  }
  ;

IfStmt:IF LPARENTHESE CondExp RPARENTHESE Stmt {
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::IfStmt>(new SysYF::SyntaxTree::IfStmt());
    temp->cond_exp = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    temp->if_statement = SysYF::Ptr<SysYF::SyntaxTree::Stmt>($5);
    temp->else_statement = nullptr;
    $$ = temp;
    $$->loc = @$;
  }
  | IF LPARENTHESE CondExp RPARENTHESE Stmt ELSE Stmt {
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::IfStmt>(new SysYF::SyntaxTree::IfStmt());
    temp->cond_exp = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    temp->if_statement = SysYF::Ptr<SysYF::SyntaxTree::Stmt>($5);
    temp->else_statement = SysYF::Ptr<SysYF::SyntaxTree::Stmt>($7);
    $$ = temp;
    $$->loc = @$;
  }
  ;

OptionRet:Exp{
    $$ = $1;
  }
  | %empty{
    $$ = nullptr;
  }
  ;

LVal:IDENTIFIER ArrayExpList{
    $$ = SysYF::Ptr<SysYF::SyntaxTree::LVal>(new SysYF::SyntaxTree::LVal());
    $$->name = $1;
    $$->array_index = $2;
    $$->loc = @$;
  }
  ;

%left PLUS MINUS;
%left MULTIPLY DIVIDE MODULO;
%precedence UPLUS UMINUS UNOT;

Exp:PLUS Exp %prec UPLUS{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::UnaryExpr>(new SysYF::SyntaxTree::UnaryExpr());
    temp->op = SysYF::SyntaxTree::UnaryOp::PLUS;
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($2);
    $$ = temp;
    $$->loc = @$;
  }
  | MINUS Exp %prec UMINUS{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::UnaryExpr>(new SysYF::SyntaxTree::UnaryExpr());
    temp->op = SysYF::SyntaxTree::UnaryOp::MINUS;
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($2);
    $$ = temp;
    $$->loc = @$;
  }
  | NOT Exp %prec UNOT{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::UnaryCondExpr>(new SysYF::SyntaxTree::UnaryCondExpr());
    temp->op = SysYF::SyntaxTree::UnaryCondOp::NOT;
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($2);
    $$ = temp;
    $$->loc = @$;
  }
  | Exp PLUS Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryExpr>(new SysYF::SyntaxTree::BinaryExpr());
    temp->op = SysYF::SyntaxTree::BinOp::PLUS;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  | Exp MINUS Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryExpr>(new SysYF::SyntaxTree::BinaryExpr());
    temp->op = SysYF::SyntaxTree::BinOp::MINUS;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  | Exp MULTIPLY Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryExpr>(new SysYF::SyntaxTree::BinaryExpr());
    temp->op = SysYF::SyntaxTree::BinOp::MULTIPLY;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  | Exp DIVIDE Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryExpr>(new SysYF::SyntaxTree::BinaryExpr());
    temp->op = SysYF::SyntaxTree::BinOp::DIVIDE;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  | Exp MODULO Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryExpr>(new SysYF::SyntaxTree::BinaryExpr());
    temp->op = SysYF::SyntaxTree::BinOp::MODULO;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  | LPARENTHESE Exp RPARENTHESE{
    $$ = $2;
  }
  | IDENTIFIER LPARENTHESE ExpList RPARENTHESE {
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::FuncCallStmt>(new SysYF::SyntaxTree::FuncCallStmt());
    temp->name = $1;
    temp->params = $3;
    $$ = temp;
    $$->loc = @$;
  }
  | LVal{
    $$ = $1;
  }
  | Number{
    $$ = $1;
  }
  ;

RelExp:RelExp LT Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::LT;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |RelExp LTE Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::LTE;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |RelExp GT Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::GT;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |RelExp GTE Exp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::GTE;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |Exp {
    $$ = $1;
  }
  ;

EqExp:EqExp EQ RelExp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::EQ;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |EqExp NEQ RelExp{
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::NEQ;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |RelExp {
    $$ = $1;
  }
  ;

LAndExp:LAndExp LOGICAND EqExp {
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::LAND;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |EqExp{
    $$ = $1;
  }
  ;

LOrExp:LOrExp LOGICOR LAndExp {
    auto temp = SysYF::Ptr<SysYF::SyntaxTree::BinaryCondExpr>(new SysYF::SyntaxTree::BinaryCondExpr());
    temp->op = SysYF::SyntaxTree::BinaryCondOp::LOR;
    temp->lhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($1);
    temp->rhs = SysYF::Ptr<SysYF::SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
  }
  |LAndExp{
    $$ = $1;
  }
  ;

CondExp:LOrExp{
    $$ = $1;
  }
  ;

Number: INTCONST {
    $$ = SysYF::Ptr<SysYF::SyntaxTree::Literal>(new SysYF::SyntaxTree::Literal());
    $$->literal_type = SysYF::SyntaxTree::Type::INT;
    $$->int_const = $1;
    $$->loc = @$;
  }
  | FLOATCONST {
    $$ = SysYF::Ptr<SysYF::SyntaxTree::Literal>(new SysYF::SyntaxTree::Literal());
    $$->literal_type = SysYF::SyntaxTree::Type::FLOAT;
    $$->float_const = $1;
    $$->loc = @$;
  }
  ;

%%

// Register errors to the driver:
void yy::SysYFParser::error (const location_type& l,
                          const std::string& m)
{
    driver.error(l, m);
}
