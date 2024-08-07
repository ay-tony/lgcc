grammar lgcc;

program: function_definition EOF;

function_definition: function_type IDENTIFIER '(' ')' block;

function_type: 'int' # KeyInt;

block: '{' statement '}' # Statements;

statement: 'return' LITERAL_INTEGER ';' # ReturnStatement;

expression: (op = '+' | op = '-') expression								# UnaryExpression
	| lhs = expression (op = '*' | op = '/' | op = '%') rhs = expression	# BinaryExpression
	| lhs = expression (op = '+' | op = '-') rhs = expression				# BinaryExpression
	| '(' expression ')'													# BraceExpression
	| LITERAL_INTEGER														# IntegerExpression;

IDENTIFIER: 'main';

LITERAL_INTEGER:
	'0'
	| ([1-9][0-9]*)
	| ('0' [1-7][0-7]*)
	| ('0' [xX][0-9a-fA-F]*);

WHITESPACE: [ \n\t\r] -> skip;