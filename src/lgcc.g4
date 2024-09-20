grammar lgcc;

program: function_definition EOF;

declaration:
    specifiers_and_qualifiers declarators_and_initializers
    | attribute_specifier_sequence specifiers_and_qualifiers declarators_and_initializers
    | attribute_specifier_sequence;

specifiers_and_qualifiers:
    (type_specifier | storage_class_specifier | type_qualifier | function_specifier | alignment_specifier)*;

type_specifier: 'void' | arithmetic_type;

arithmetic_type: 'int' | 'float';

storage_class_specifier: 'typedef' | 'constexpr' | 'auto' | 'register' | 'static' | 'extern' | '_Thread_local';

type_qualifier: 'const' | 'volatile' | 'restrict' | '_Atomic';

function_specifier: 'inline' | '_Noreturn';

alignment_specifier: '_Alignas';

declarators_and_initializers:
    declarator_and_initializer (',' declarator_and_initializer);

declarator_and_initializer:
    declarator initializer?;

declarator:
    noptr_declarator
    | '*' attribute_specifier_sequence? qualifiers? declarator;

noptr_declarator:
    IDENTIFIER attribute_specifier_sequence?
    | '(' declarator ')'
    | noptr_declarator '[' 'static'? qualifiers? expression? ']' attribute_specifier_sequence
    | noptr_declarator '[' qualifiers? 'static'? expression? ']' attribute_specifier_sequence
    | noptr_declarator '[' qualifiers? '*' ']' attribute_specifier_sequence
    | noptr_declarator '(' parameter_list ')' attribute_specifier_sequence
    | noptr_declarator '(' ')' attribute_specifier_sequence;

qualifiers: qualifier+;

qualifier: 'const' | 'volatile' | 'restrict';
    
attribute_specifier_sequence: '[[' attribute_list ']]';

attribute_list: attribute_token (',' attribute_token)*;

attribute_token:
    standard_attribute
    | attribute_prefix '::' IDENTIFIER
    | standard_attribute '(' argument_list? ')'
    | attribute_prefix '::' IDENTIFIER '(' argument_list? ')';

standard_attribute:
    'deprecated'
    | 'fallthrough'
    | 'nodiscard'
    | 'maybe_unused'
    | 'noreturn'
    | 'unsequenced'
    | 'reproducible';

attribute_prefix: 'lgcc';

argument_list: balanced_token_sequence;

balanced_token_sequence:
    balanced_token 
    | balanced_token_sequence balanced_token;

balanced_token:
    '(' balanced_token_sequence ')'
    | '[' balanced_token_sequence ']'
    | '{' balanced_token_sequence '}';
    // TODO: 此处需要添加其它 C 标准中的 tokens

parameter_list:; // TODO: 添加形参列表

initializer:; // TODO: 添加初始化器

function_definition: function_type IDENTIFIER '(' ')' block;

function_type: 'int' | 'void';

block: '{' statement* '}';

statement:
    return_statement                      # ReturnStatement
    | variable_definition_statement       # VariableDefinitionStatement
    | const_variable_definition_statement # ConstVariableDefinitionStatement
    | assignment_statement                # AssignmentStatement
    | block                               # BlockStatement;

return_statement:
    'return' const_expression ';' # ReturnConstExpressionStatement
    | 'return' expression ';'     # ReturnExpressionStatement;

variable_definition_statement:
    arithmetic_type single_variable_definition (',' single_variable_definition)* ';';

single_variable_definition:
    IDENTIFIER                        # NoInitializeVariableDefinition
    | IDENTIFIER '=' const_expression # ConstExpressionInitializeVariableDefinition
    | IDENTIFIER '=' expression       # ExpressionInitializeVariableDefinition;

const_variable_definition_statement:
    (('const' arithmetic_type) | (arithmetic_type 'const')) single_const_variable_definition (',' single_const_variable_definition)* ';';

single_const_variable_definition:
    IDENTIFIER '=' const_expression   # ConstExpressionInitializeConstVariableDefinition
    | IDENTIFIER '=' expression       # ExpressionInitializeConstVariableDefinition;

assignment_statement:
    left_value '=' expression ';';

left_value:
    IDENTIFIER;

const_expression:
    (op = '+' | op = '-') const_expression                                           # UnaryConstExpression
    | lhs = const_expression (op = '*' | op = '/' | op = '%') rhs = const_expression # BinaryConstExpression
    | lhs = const_expression (op = '+' | op = '-') rhs = const_expression            # BinaryConstExpression
    | lhs = const_expression (op = '<' | op = '<=' | op = '>' | op = '>=') rhs = const_expression # BinaryConstExpression
    | lhs = const_expression (op = '==' | op = '!=') rhs = const_expression          # BinaryConstExpression
    | '(' const_expression ')'                                                       # BraceConstExpression
    | LITERAL_INTEGER                                                                # IntegerConstExpression
    | LITERAL_FLOAT                                                                  # FloatConstExpression;

expression:
    (op = '+' | op = '-') expression                                     # UnaryExpression
    | lhs = expression (op = '*' | op = '/' | op = '%') rhs = expression # BinaryExpression
    | lhs = expression (op = '+' | op = '-') rhs = expression            # BinaryExpression
    | lhs = expression (op = '<' | op = '<=' | op = '>' | op = '>=') rhs = expression # BinaryExpression
    | lhs = expression (op = '==' | op = '!=') rhs = expression          # BinaryExpression
    | '(' expression ')'                                                 # BraceExpression
    | const_expression                                                   # ConstExpressionExpression
    | IDENTIFIER                                                         # IdentifierExpression;

IDENTIFIER: [a-zA-Z_][0-9a-zA-Z_]*;

LITERAL_INTEGER:
    '0'
    | ([1-9][0-9]*)
    | ('0' [1-7][0-7]*)
    | ('0' [xX][0-9a-fA-F]*);

LITERAL_FLOAT:
    (([0-9]* '.' [0-9]+) | [0-9]+ '.') ([eE] [+-]? [0-9]+)?;

WHITESPACE: [ \n\t\r] -> skip;
COMMENT_SINGLE_LINE: '//' .*? '\n' -> skip;
COMMENT_MULTI_LINE: '/*' .*? '*/' -> skip;

