grammar c23;

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
    identifier attribute_specifier_sequence?
    | '(' declarator ')'
    | noptr_declarator '[' 'static'? qualifiers? expression? ']' attribute_specifier_sequence?
    | noptr_declarator '[' qualifiers? 'static'? expression? ']' attribute_specifier_sequence?
    | noptr_declarator '[' qualifiers? '*' ']' attribute_specifier_sequence?
    | noptr_declarator '(' parameter_list ')' attribute_specifier_sequence?
    | noptr_declarator '(' ')' attribute_specifier_sequence?;

qualifiers: qualifier+;

qualifier: 'const' | 'volatile' | 'restrict';
    
attribute_specifier_sequence: '[[' attribute_list ']]';

attribute_list: attribute_token (',' attribute_token)*;

attribute_token:
    standard_attribute
    | attribute_prefix '::' identifier
    | standard_attribute '(' argument_list? ')'
    | attribute_prefix '::' identifier '(' argument_list? ')';

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

parameter_list:
    'void'; // TODO: 添加形参列表

initializer:; // TODO: 添加初始化器

statement: label* unlabeled_statement;

label:
    attribute_specifier_sequence? identifier ':'
    | attribute_specifier_sequence? 'case' const_expression ':'
    | attribute_specifier_sequence? 'default' ':';

unlabeled_statement:
    unlabeled_compound_statement
    | unlabeled_expression_statement
    | unlabeled_selection_statement
    | unlabeled_iteration_statement
    | unlabeled_jump_statement;

unlabeled_compound_statement:
    attribute_specifier_sequence? '{' (unlabeled_statement | label | declaration)* '}';

unlabeled_expression_statement:
    attribute_specifier_sequence? expression ';';

unlabeled_selection_statement:
    attribute_specifier_sequence? 'if' '(' expression ')' statement 'else' statement
    | attribute_specifier_sequence? 'if' '(' expresion ')' statement
    | attribute_specifier_sequence? 'switch' '(' expresion ')' statement;

unlabeled_iteration_statement:
    attribute_specifier_sequence? 'while' '(' expression ')' statement
    | attribute_specifier_sequence? 'do' statement 'while' '(' expresion ')' ';'
    | attribute_specifier_sequence? 'for' '(' init_clause ';' expression? ';' expression? ')' statement;

init_clause: expression | declaration;

unlabeled_jump_statement:
    attribute_specifier_sequence? 'break' ';'
    | attribute_specifier_sequence? 'continue' ';'
    | attribute_specifier_sequence? 'return' expression? ';'
    | attribute_specifier_sequence? 'goto' identifier ';';

token:
    KEYWORD
    | IDENTIFIER
    | constant
    | string_literal
    | punctuator;

KEYWORD:
    'alignof' | 'auto' | 'bool' | 'break' | 'case' | 'char' | 'const' | 'constexpr' | 'continue' | 'default' | 'do' | 'double' | 'else'
    | 'enum' | 'extern' | 'false' | 'float' | 'for' | 'goto' | 'if' | 'inline' | 'int' | 'long' | 'nullptr' | 'register' | 'restrict' 
    | 'return' | 'short' | 'signed' | 'sizeof' | 'static' | 'static_assert' | 'struct' | 'switch' | 'thread_local' | 'true' | 'typedef'
    | 'typeof' | 'typeof_unqual' | 'union' | 'unsigned' | 'void' | 'volatile' | 'while' | '_Atomic' | '_BitInt' | '_Complex' | '_Decimal128'
    | '_Decimal32' | '_Decimal64' | '_Generic' | '_Imaginary' | '_Noreturn';

IDENTIFIER: IDENTIFIER_START 
            | IDENTIFIER IDENTIFIER_CONTINUE;

IDENTIFIER_START: NONDIGIT;

IDENTIFIER_CONTINUE: DIGIT | NONDIGIT;

NONDIGIT: [a-zA-Z] | '_';

DIGIT: [0-9];

CONSTANT: INTEGER_CONSTANT | FLOATING_CONSTANT | ENUMERATION_CONSTANT | CHARACTER_CONSTANT | PREDEFINED_CONSTANT;

INTEGER_CONSTANT:
    DECIMAL_CONSTANT INTEGER_SUFFIX?
    | OCTAL_CONSTANT INTEGER_SUFFIX?
    | HEXADECIMAL_CONSTANT INTEGER_SUFFIX?
    | BINARY_CONSTANT INTEGER_SUFFIX?;

DECIMAL_CONSTANT: NONZERO_DIGIT | DECIMAL_CONSTANT '\''? DIGIT;

NONZERO_DIGIT: [1-9];

OCTAL_CONSTANT: '0' | OCTAL_CONSTANT '\''? OCTAL_DIGIT;

OCTAL_DIGIT: [0-7];

HEXADECIMAL_CONSTANT: HEXADECIMAL_PREFIX HEXADECIMAL_DIGIT_SEQUENCE;

HEXADECIMAL_PREFIX: '0x' | '0X';

HEXADECIMAL_DIGIT_SEQUENCE: HEXADECIMAL_DIGIT | HEXADECIMAL_DIGIT_SEQUENCE '\''? HEXADECIMAL_DIGIT;

HEXADECIMAL_DIGIT: [0-9a-fA-F];

BINARY_CONSTANT: BINARY_PREFIX BINARY_DIGIT | BINARY_CONSTANT '\''? BINARY_DIGIT;

BINARY_PREFIX: '0b' | '0B';

BINARY_DIGIT: [01];

INTEGER_SUFFIX:
    UNSIGNED_SUFFIX LONG_SUFFIX?
    | UNSIGNED_SUFFIX LONG_LONG_SUFFIX
    | UNSIGNED_SUFFIX BIT_PRECISE_INT_SUFFIX
    | LONG_SUFFIX UNSIGNED_SUFFIX?
    | LONG_LONG_SUFFIX UNSIGNED_SUFFIX?
    | BIT_PRECISE_INT_SUFFIX UNSIGNED_SUFFIX?;

UNSIGNED_SUFFIX: [uU];

LONG_SUFFIX: 'l' | 'L';

LONG_LONG_SUFFIX: 'll' | 'LL';

BIT_PRECISE_INT_SUFFIX: 'wb' | 'WB';

FLOATING_CONSTANT: DECIMAL_FLOATING_CONSTANT | HEXADECIMAL_FLOATING_CONSTANT;

DECIMAL_FLOATING_CONSTANT:
    FRACTIONAL_CONSTANT EXPONENT_PART? FLOATING_SUFFIX?
    | DIGIT_SEQUENCE EXPONENT_PART FLOATING_SUFFIX?;

FRACTIONAL_CONSTANT:
    DIGIT_SEQUENCE? '.' DIGIT_SEQUENCE
    | DIGIT_SEQUENCE '.';

EXPONENT_PART: [eE] SIGN? DIGIT_SEQUENCE;

SIGN: [+-];

DIGIT_SEQUENCE: DIGIT | DIGIT_SEQUENCE '\''? DIGIT;

FLOATING_SUFFIX: 'f' | 'l' | 'F' | 'L' | 'df' | 'dd' | 'dl' | 'DF' | 'DD' | 'DL';

HEXADECIMAL_FLOATING_CONSTANT:
    HEXADECIMAL_PREFIX HEXADECIMAL_FRACTIONAL_CONSTANT BINARY_EXPONENT_PART FLOATING_SUFFIX?
    | HEXADECIMAL_PREFIX HEXADECIMAL_DIGIT_SEQUENCE BINARY_EXPONENT_PART FLOATING_SUFFIX?;

HEXADECIMAL_FRACTIONAL_CONSTANT:
    HEXADECIMAL_DIGIT_SEQUENCE? '.' HEXADECIMAL_DIGIT_SEQUENCE
    | HEXADECIMAL_DIGIT_SEQUENCE '.';

BINARY_EXPONENT_PART: [pP] SIGN? DIGIT_SEQUENCE;

ENUMERATION_CONSTANT: IDENTIFIER;

CHARACTER_CONSTANT: ENCODING_PREFIX? '\'' C_CHAR_SEQUENCE '\'';

ENCODING_PREFIX: 'u8' | 'u' | 'U' | 'L';

C_CHAR_SEQUENCE: C_CHAR | C_CHAR_SEQUENCE C_CHAR;

C_CHAR:; // TODO
