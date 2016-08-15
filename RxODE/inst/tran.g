//loop

statement_list : (statement)+ ;

statement 
  : assignment ';'
  | derivative ';'
  | jac ';'
  | compound_statement
  | selection_statement
  | printf_statement ';'
  | ';' ;


compound_statement : '{' statement_list? '}' ;

selection_statement
  : 'if' '(' logical_or_expression ')' statement ('else' statement)?;

printf_statement
  : printf_command '(' string (',' identifier)* ')';

printf_command
  : 'printf' | 'Rprintf' | 'print';

derivative : 'd/dt' '(' identifier_no_output ')' '=' additive_expression;
der_rhs    : 'd/dt' '(' identifier_no_output ')';
jac        : 'jac' '(' identifier_no_output ',' identifier_no_output ')' '=' additive_expression;
jac_rhs    : 'jac' '(' identifier_no_output ',' identifier_no_output ')';

assignment : identifier '=' additive_expression;



logical_or_expression :	logical_and_expression 
  ('||' logical_and_expression)* ;

logical_and_expression : equality_expression 
  ('&&' equality_expression)* ;

equality_expression : relational_expression 
  (('!=' | '==') relational_expression)* ;

relational_expression : additive_expression
 (('<' | '>' | '<=' | '>=') additive_expression)* ;

additive_expression : multiplicative_expression
  (('+' | '-') multiplicative_expression)* ;

multiplicative_expression : unary_expression 
  (('*' | '/') unary_expression)* ;

unary_expression : ('+' | '-')? (primary_expression | power_expression);

power_expression : primary_expression '^' primary_expression ;

primary_expression 
  : identifier
  | der_rhs
  | jac_rhs
  | constant
  | function
  | '(' additive_expression ')'
  ;

function : identifier '(' additive_expression (',' additive_expression)* ')' ;

constant : decimalint | float1 | float2;


decimalint: "0|([1-9][0-9]*)" $term -1;
string: "\"([^\"\\]|\\[^])*\"";
float1: "([0-9]+.[0-9]*|[0-9]*.[0-9]+)([eE][\-\+]?[0-9]+)?" $term -2;
float2: "[0-9]+[eE][\-\+]?[0-9]+" $term -3;
identifier: "[a-zA-Z_][a-zA-Z0-9_]*" $term -4;
identifier_no_output: "[a-zA-Z_][a-zA-Z0-9_]*" $term -4;
whitespace: ( "[ \t\r\n]+" | singleLineComment )*;
singleLineComment: '#' "[^\n]*" '\n';
