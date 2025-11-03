// Herath Mudiyanselage Sahan Kavinda Sepala 231ADB248
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef enum {
    TOK_NUMBER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_POW,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_EOF,
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    double value;
    int start_pos;
    int length;
} Token;

typedef struct {
    const char *input;
    int length;
    int pos;
    int char_index;
    Token current_token;
    int has_error;
    int error_pos;
} ParserState;

void init_parser(ParserState *parser, const char *input, int length);
Token get_next_token(ParserState *parser);
void skip_whitespace(ParserState *parser);
void fail(ParserState *parser, int pos);
double parse_expression(ParserState *parser);
double parse_term(ParserState *parser);
double parse_factor(ParserState *parser);
double parse_power(ParserState *parser);
double parse_primary(ParserState *parser);

void init_parser(ParserState *parser, const char *input, int length) {
    parser->input = input;
    parser->length = length;
    parser->pos = 0;
    parser->char_index = 1;
    parser->has_error = 0;
    parser->error_pos = 0;
    parser->current_token = get_next_token(parser);
}

void skip_whitespace(ParserState *parser) {
    while (parser->pos < parser->length && isspace(parser->input[parser->pos])) {
        if (parser->input[parser->pos] == '\n') {
            parser->char_index++;
        } else {
            parser->char_index++;
        }
        parser->pos++;
    }
}

Token get_next_token(ParserState *parser) {
    Token token;
    token.start_pos = parser->char_index;
    token.length = 1;
    
    skip_whitespace(parser);
    
    if (parser->pos >= parser->length) {
        token.type = TOK_EOF;
        token.start_pos = parser->char_index;
        return token;
    }
    
    char current = parser->input[parser->pos];
    
    switch (current) {
        case '+':
            token.type = TOK_PLUS;
            parser->pos++;
            parser->char_index++;
            return token;
        case '-':
            token.type = TOK_MINUS;
            parser->pos++;
            parser->char_index++;
            return token;
        case '*':
            if (parser->pos + 1 < parser->length && parser->input[parser->pos + 1] == '*') {
                token.type = TOK_POW;
                token.length = 2;
                parser->pos += 2;
                parser->char_index += 2;
            } else {
                token.type = TOK_STAR;
                parser->pos++;
                parser->char_index++;
            }
            return token;
        case '/':
            token.type = TOK_SLASH;
            parser->pos++;
            parser->char_index++;
            return token;
        case '(':
            token.type = TOK_LPAREN;
            parser->pos++;
            parser->char_index++;
            return token;
        case ')':
            token.type = TOK_RPAREN;
            parser->pos++;
            parser->char_index++;
            return token;
    }
    
    if (isdigit(current) || current == '.') {
        char *endptr;
        double value = strtod(parser->input + parser->pos, &endptr);
        
        if (endptr > parser->input + parser->pos) {
            token.type = TOK_NUMBER;
            token.value = value;
            token.length = endptr - (parser->input + parser->pos);
            parser->char_index += token.length;
            parser->pos += token.length;
            return token;
        }
    }
    
    token.type = TOK_ERROR;
    parser->pos++;
    parser->char_index++;
    return token;
}

void fail(ParserState *parser, int pos) {
    if (!parser->has_error) {
        parser->has_error = 1;
        parser->error_pos = pos;
    }
}

double parse_primary(ParserState *parser) {
    if (parser->has_error) return 0.0;
    
    Token token = parser->current_token;
    
    if (token.type == TOK_NUMBER) {
        parser->current_token = get_next_token(parser);
        return token.value;
    }
    else if (token.type == TOK_LPAREN) {
        parser->current_token = get_next_token(parser);
        double result = parse_expression(parser);
        
        if (parser->current_token.type != TOK_RPAREN) {
            fail(parser, parser->current_token.start_pos);
            return 0.0;
        }
        
        parser->current_token = get_next_token(parser);
        return result;
    }
    else {
        fail(parser, token.start_pos);
        return 0.0;
    }
}

double parse_power(ParserState *parser) {
    if (parser->has_error) return 0.0;
    
    double left = parse_primary(parser);
    
    while (!parser->has_error && parser->current_token.type == TOK_POW) {
        Token op_token = parser->current_token;
        parser->current_token = get_next_token(parser);
        
        double right = parse_power(parser);
        
        if (left == 0.0 && right < 0.0) {
            fail(parser, op_token.start_pos);
            return 0.0;
        }
        
        left = pow(left, right);
    }
    
    return left;
}

double parse_factor(ParserState *parser) {
    if (parser->has_error) return 0.0;
    
    double left = parse_power(parser);
    
    while (!parser->has_error && (parser->current_token.type == TOK_STAR || 
                                  parser->current_token.type == TOK_SLASH)) {
        Token op_token = parser->current_token;
        TokenType op_type = op_token.type;
        parser->current_token = get_next_token(parser);
        
        double right = parse_power(parser);
        
        if (op_type == TOK_STAR) {
            left *= right;
        } else {
            if (right == 0.0) {
                fail(parser, op_token.start_pos);
                return 0.0;
            }
            left /= right;
        }
    }
    
    return left;
}

double parse_term(ParserState *parser) {
    if (parser->has_error) return 0.0;
    
    double left = parse_factor(parser);
    
    while (!parser->has_error && (parser->current_token.type == TOK_PLUS || 
                                  parser->current_token.type == TOK_MINUS)) {
        TokenType op_type = parser->current_token.type;
        parser->current_token = get_next_token(parser);
        
        double right = parse_factor(parser);
        
        if (op_type == TOK_PLUS) {
            left += right;
        } else {
            left -= right;
        }
    }
    
    return left;
}

double parse_expression(ParserState *parser) {
    return parse_term(parser);
}

void process_expression(const char* expression) {
    ParserState parser;
    init_parser(&parser, expression, strlen(expression));
    
    double result = parse_expression(&parser);
    
    if (!parser.has_error && parser.current_token.type != TOK_EOF) {
        fail(&parser, parser.current_token.start_pos);
    }
    
    if (parser.has_error) {
        printf("ERROR:%d\n", parser.error_pos);
    } else {
        if (fabs(result - round(result)) < 1e-12) {
            printf("%.0f\n", result);
        } else {
            printf("%.15g\n", result);
        }
    }
}

int main() {
    printf("=== Arithmetic Parser Test ===\n\n");
    
    printf("2 + 3 = ");
    process_expression("2 + 3");
    
    printf("10 - 4 * 2 = ");
    process_expression("10 - 4 * 2");
    
    printf("(10 - 4) * 2 = ");
    process_expression("(10 - 4) * 2");
    
    printf("2 ** 3 ** 2 = ");
    process_expression("2 ** 3 ** 2");
    
    printf("10 / 0 = ");
    process_expression("10 / 0");
    
    printf("2 + * 3 = ");
    process_expression("2 + * 3");
    
    printf("3.5 * 2 = ");
    process_expression("3.5 * 2");
    
    printf("10 / 3 = ");
    process_expression("10 / 3");
    
    return 0;
}