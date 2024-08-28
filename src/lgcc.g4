grammar lgcc;

program: function_definition EOF;

function_definition: function_type IDENTIFIER '(' ')' block;

function_type: 'int' | 'void';

block: '{' statement* '}';

statement:
    return_statement                # ReturnStatement
    | variable_definition_statement # VariableDefinitionStatement;

return_statement:
    'return' const_expression ';' # ReturnConstExpressionStatement
    | 'return' expression ';'     # ReturnExpressionStatement;

variable_definition_statement:
    variable_type single_variable_definition (',' single_variable_definition)* ';';

variable_type: 'int' | 'float' ;

single_variable_definition:
    IDENTIFIER                        # NoInitializeVariableDefinition
    | IDENTIFIER '=' const_expression # ConstExpressionInitializeVariableDefinition
    | IDENTIFIER '=' expression       # ExpressionInitializeVariableDefinition;

const_expression:
    (op = '+' | op = '-') const_expression                                           # UnaryConstExpression
    | lhs = const_expression (op = '*' | op = '/' | op = '%') rhs = const_expression # BinaryConstExpression
    | lhs = const_expression (op = '+' | op = '-') rhs = const_expression            # BinaryConstExpression
    | '(' const_expression ')'                                                       # BraceConstExpression
    | LITERAL_INTEGER                                                                # IntegerConstExpression;

expression:
    (op = '+' | op = '-') expression                                     # UnaryExpression
    | lhs = expression (op = '*' | op = '/' | op = '%') rhs = expression # BinaryExpression
    | lhs = expression (op = '+' | op = '-') rhs = expression            # BinaryExpression
    | '(' expression ')'                                                 # BraceExpression
    | const_expression                                                   # ConstExpressionExpression
    | IDENTIFIER                                                         # IdentifierExpression;

IDENTIFIER: [a-zA-Z_][0-9a-zA-Z_]*;

LITERAL_INTEGER:
    '0'
    | ([1-9][0-9]*)
    | ('0' [1-7][0-7]*)
    | ('0' [xX][0-9a-fA-F]*);

WHITESPACE: [ \n\t\r] -> skip;
COMMENT_SINGLE_LINE: '//' .*? '\n' -> skip;
COMMENT_MULTI_LINE: '/*' .*? '*/' -> skip;

