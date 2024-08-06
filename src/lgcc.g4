grammar lgcc;

program: function_definition;

function_definition: function_type identifier '(' ')' block;

function_type: 'int';

identifier: 'main';

block: '{' statement '}';

statement: 'return' expression ';';

expression: ('+' | '-') expression
	| expression ('*' | '/' | '%') expression
	| expression ('+' | '-') expression
	| '(' expression ')'
	| literal_integer;

literal_integer: '0';

Whitespace: [ \n\t\r] -> skip;