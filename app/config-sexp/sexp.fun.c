/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Operations on S-expressions.
 */

#include "archi/sexp.fun.h"
#include "archi/sexp.typ.h"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define BLOCK 64 // a single block is enough to fit any number

static
archi_sexp_t*
parse_number(
        FILE *stream,
        size_t *current_column)
{
    // Initialize variables
    char str[BLOCK] = {0};
    size_t size = 0;

    bool is_signed = false; // whether the integer is signed
    bool is_float = false; // whether the number is floating-point
    bool has_exponent = false; // whether the floating-point number has exponent part

    enum {
        MANTISSA_SIGN,
        MANTISSA_LEADING_ZEROS,
        MANTISSA_INTEGER_PART,
        MANTISSA_FRACTION_PART,
        EXPONENT_SIGN,
        EXPONENT_LEADING_ZEROS,
        EXPONENT_INTEGER_PART,

        NUM_MODES
    } mode = 0;

    size_t num_mantissa_leading_zeros = 0;
    size_t num_mantissa_digits = 0; // significant digits only
    size_t num_exponent_leading_zeros = 0;
    size_t num_exponent_digits = 0; // significant digits only

    // Enough for quadruple precision
#define MAX_MANTISSA_DIGITS 36
#define MAX_EXPONENT_DIGITS 5

    // Parse characters
    int chr;
    while ((chr = fgetc(stream)) != EOF)
    {
        if (isspace(chr) || (chr == '(') || (chr == ')')) // a number ends on whitespace or parens
        {
            ungetc(chr, stream);
            break;
        }
        else
            (*current_column)++;

        switch (mode)
        {
            case MANTISSA_SIGN:
                if ((chr == '+') || (chr == '-'))
                {
                    is_signed = true; // signed integers are the ones with a sign
                    mode = MANTISSA_LEADING_ZEROS;
                    break;
                }
                else if (isdigit(chr))
                {
                    if (chr != '0') // no leading zeros
                    {
                        mode = MANTISSA_INTEGER_PART;
                        num_mantissa_digits++;
                    }
                    else // leading zeros
                    {
                        mode = MANTISSA_LEADING_ZEROS;
                        num_mantissa_leading_zeros++;
                    }
                    break;
                }
                else if (chr == '.') // fraction part of a floating-point number
                {
                    is_float = true;
                    mode = MANTISSA_FRACTION_PART;
                    break;
                }

                return NULL;

            case MANTISSA_LEADING_ZEROS:
                if (chr == '0')
                {
                    if (num_mantissa_leading_zeros > 0)
                        continue; // ignore superfluous leading zeros
                    else
                    {
                        num_mantissa_leading_zeros++;
                        break;
                    }
                }
                else if (isdigit(chr)) // significant digit
                {
                    mode = MANTISSA_INTEGER_PART;
                    num_mantissa_digits++;
                    break;
                }
                else if (chr == '.') // fraction part of a floating-point number
                {
                    is_float = true;
                    mode = MANTISSA_FRACTION_PART;
                    break;
                }
                else if ((chr == 'e') || (chr == 'E')) // exponent part of a floating-point number
                {
                    is_float = true;
                    has_exponent = true;
                    mode = EXPONENT_SIGN;
                    break;
                }

                return NULL;

            case MANTISSA_INTEGER_PART:
                if (isdigit(chr)) // significant digit
                {
                    num_mantissa_digits++;

                    if (num_mantissa_digits > MAX_MANTISSA_DIGITS) // overflow
                        return NULL;
                    else
                        break;
                }
                else if (chr == '.') // fraction part of a floating-point number
                {
                    is_float = true;
                    mode = MANTISSA_FRACTION_PART;
                    break;
                }
                else if ((chr == 'e') || (chr == 'E')) // exponent part of a floating-point number
                {
                    is_float = true;
                    has_exponent = true;
                    mode = EXPONENT_SIGN;
                    break;
                }

                return NULL;

            case MANTISSA_FRACTION_PART:
                if (isdigit(chr)) // significant digit
                {
                    num_mantissa_digits++;

                    if (num_mantissa_digits > MAX_MANTISSA_DIGITS) // excess precision
                        continue; // ignore rest of the digits
                    else
                        break;
                }
                else if ((chr == 'e') || (chr == 'E')) // exponent part of a floating-point number
                {
                    has_exponent = true;
                    mode = EXPONENT_SIGN;
                    break;
                }

                return NULL;

            case EXPONENT_SIGN:
                if ((chr == '+') || (chr == '-'))
                {
                    mode = EXPONENT_LEADING_ZEROS;
                    break;
                }
                else if (isdigit(chr))
                {
                    if (chr != '0') // no leading zeros
                    {
                        mode = MANTISSA_INTEGER_PART;
                        num_exponent_digits++;
                    }
                    else // leading zeros
                    {
                        mode = MANTISSA_LEADING_ZEROS;
                        num_exponent_leading_zeros++;
                    }
                    break;
                }

                return NULL;

            case EXPONENT_LEADING_ZEROS:
                if (chr == '0')
                {
                    if (num_exponent_leading_zeros > 0)
                        continue; // ignore superfluous leading zeros
                    else
                    {
                        num_exponent_leading_zeros++;
                        break;
                    }
                }
                else if (isdigit(chr)) // significant digit
                {
                    mode = EXPONENT_INTEGER_PART;
                    num_exponent_digits++;
                    break;
                }

                return NULL;

            case EXPONENT_INTEGER_PART:
                if (isdigit(chr)) // significant digit
                {
                    num_exponent_digits++;

                    if (num_exponent_digits > MAX_EXPONENT_DIGITS) // overflow/underflow
                        return NULL;
                    else
                        break;
                }

                return NULL;
        }

        // Push the character to the string
        if (size + 1 == BLOCK)
            return NULL;

        str[size++] = chr;
    }

    str[size] = '\0';

    // Check if parsing was successful
    if (num_mantissa_leading_zeros + num_mantissa_digits == 0)
        return NULL;
    else if (has_exponent && (num_exponent_leading_zeros + num_mantissa_digits == 0))
        return NULL;

    // Parse the number
    archi_sexp_t *sexp = malloc(sizeof(*sexp));
    if (sexp == NULL)
        return NULL;

    if (!is_float) // integer
    {
        if (!is_signed) // unsigned
        {
            archi_sexp_uint_t as_uint;
            if (sscanf(str, "%llu", &as_uint) != 1)
                goto failure;

            sexp->type = ARCHI_SEXP_TYPE_UINT;
            sexp->as_uint = as_uint;
        }
        else // signed
        {
            archi_sexp_sint_t as_sint;
            if (sscanf(str, "%lli", &as_sint) != 1)
                goto failure;

            sexp->type = ARCHI_SEXP_TYPE_SINT;
            sexp->as_sint = as_sint;
        }
    }
    else // floating-point number
    {
        archi_sexp_float_t as_float;
        if (sscanf(str, "%Lf", &as_float) != 1)
            goto failure;

        sexp->type = ARCHI_SEXP_TYPE_FLOAT;
        sexp->as_float = as_float;
    }

    return sexp;

failure:
    free(sexp);
    return NULL;
}

static
archi_sexp_t*
parse_symbol_or_string(
        FILE *stream,
        bool symbol,
        size_t *current_line,
        size_t *current_column)
{
    // Initialize variables
    char *str = malloc(BLOCK);
    if (str == NULL)
        return NULL;

    size_t capacity = BLOCK;
    size_t size = 0;

    bool mode_escape = false; // processing an escape character
    bool finished = false; // closing double quote was read

    // Parse characters
    int chr;
    while ((chr = fgetc(stream)) != EOF)
    {
        if (symbol)
        {
            if (isspace(chr) || (chr == '(') || (chr == ')')) // a symbol ends on white space or parens
            {
                ungetc(chr, stream);
                break;
            }
            else if (iscntrl(chr)) // control characters are forbidden in symbol names
                goto failure;
        }

        // Count lines and columns
        if (chr == '\n')
        {
            (*current_line)++;
            *current_column = 0;
            continue;
        }
        else
            (*current_column)++;

        // Process special characters
        if (!symbol)
        {
            if (!mode_escape)
            {
                switch (chr)
                {
                    case '"': // end of a string
                        finished = true;
                        goto break_loop;

                    case '\\': // next character is escape
                        mode_escape = true;
                        continue;
                }
            }
            else
            {
                mode_escape = false;

                switch (chr)
                {
                    case '\\': // insert backslash
                    case '"':  // insert double quote
                        break;

                    case 'n': // insert newline
                        chr = '\n';
                        break;

                    case '\n': // ignore newline
                        continue;

                    default: // unknown character
                        goto failure;
                }
            }
        }

        // Resize the buffer if needed
        if (size + 1 == capacity)
        {
            char *new_str = realloc(str, capacity * 2);
            if (new_str == NULL)
                goto failure;

            str = new_str;
            capacity *= 2;
        }

        // Append the character to the buffer
        str[size++] = chr;
    }
break_loop:

    str[size] = '\0';

    // Check if parsing was successful
    if (symbol)
    {
        if (size == 0)
            goto failure;
    }
    else if (!finished)
        goto failure;

    // Resize the buffer and insert null-terminator
    if (size + 1 != capacity)
    {
        char *new_str = realloc(str, size + 1);
        if (new_str == NULL)
            goto failure;

        str = new_str;
    }

    // Process a successful parsing
    archi_sexp_t *sexp = malloc(sizeof(*sexp));
    if (sexp == NULL)
        return NULL;

    if (symbol)
    {
        sexp->type = ARCHI_SEXP_TYPE_SYMBOL;
        sexp->as_symbol = str;
    }
    else
    {
        sexp->type = ARCHI_SEXP_TYPE_STRING;
        sexp->as_string = str;
    }

    return sexp;

    // Process a failure
failure:
    free(str);
    return NULL;
}

/*****************************************************************************/

#define STACK_CAPACITY 128

archi_sexp_t*
archi_sexp_parse_stream(
        FILE *stream,

        size_t *error_line,
        size_t *error_column)
{
    if (stream == NULL)
        return NULL;

    // Initialize variables
    size_t current_line = 1; // lines are counted starting from one
    size_t current_column = 0; // before the first character

    bool mode_comment = false; // whether processing a comment

    struct parser_state {
        archi_sexp_t *sexp; // S-expression as a whole
        archi_sexp_cons_t *list_tail; // tail cons-cell of the list
        bool mode_point; // whether the final cons separator have been read
    } *stack;

    stack = malloc(sizeof(*stack) * STACK_CAPACITY);
    if (stack == NULL)
        return NULL;
    size_t stack_capacity = STACK_CAPACITY;
    size_t stack_size = 0;

    // Stack operations
#define STACK_PUSH() do {                                                           \
        if (stack_size == stack_capacity) {                                         \
            struct parser_state *new_stack = realloc(stack, stack_capacity * 2);    \
            if (new_stack == NULL) goto failure;                                    \
            stack = new_stack;                                                      \
        }                                                                           \
        stack[stack_size++] = (struct parser_state){0};                             \
        stack[stack_size].sexp = malloc(sizeof(*stack->sexp));                      \
        if (stack[stack_size].sexp == NULL) goto failure;                           \
        *stack[stack_size].sexp = (archi_sexp_t){0};                                \
    } while (0)

#define STACK_POP() do {                    \
        if (stack_size == 0) goto failure;  \
        --stack_size;                       \
    } while (0)

#define STACK_TOP stack[stack_size - 1]

    // Allocate top expression
    STACK_PUSH();

    // Parse characters
    int chr;
    while ((chr = fgetc(stream)) != EOF)
    {
        // Count lines and columns
        if (chr == '\n')
        {
            (*current_line)++;
            *current_column = 0;
            mode_comment = false;
            continue;
        }
        else
            (*current_column)++;

        // Skip whitespace and comments
        if (isspace(chr) || mode_comment)
            continue;
        else if (chr == ';')
        {
            mode_comment = true;
            continue;
        }

        if (chr == '(') // enter sublist
        {
            STACK_PUSH();
            continue;
        }
        else if (chr == ')') // exit sublist
        {
            archi_sexp_t *sexp = STACK_TOP.sexp;
            STACK_POP();

            if (stack_size == 0) // unmatched parens
                goto failure;



            // TODO pop
        }
        else if (chr == '.')
        {
            // TODO
        }
        else if (isdigit(chr) || (chr == '+') || (chr == '-'))
        {
            // TODO
        }
        else if (chr == '"')
        {
            // TODO
        }
        else
        {
            // TODO
        }









        else if (isalpha(chr))
        {
            mode = MODE_SYMBOL;
            ungetc(chr, stream);
        }
        else if (isdigit(chr) || (chr == '-') || (chr == '+'))
        {
            ungetc(chr, stream);
            if (!parse_number(STACK_TOP.sexp, stream))
                goto failure;
        }
        else if (chr == '"')
        {
            // TODO
        }
        else
        {
            // TODO error
        }
    }

    // TODO

    if (stack_size != 1) // unbalanced parens
        goto failure;

    archi_sexp_t *sexp = STACK_TOP.sexp;
    free(stack);

    return sexp;

failure:
    for (size_t i = 0; i < stack_size; i++)
        archi_sexp_destroy_tree(stack[i].sexp);

    free(stack);

    if (error_line != NULL)
        *error_line = current_line;
    if (error_column != NULL)
        *error_column = current_column;

    return NULL;
}

void
archi_sexp_destroy_tree(
        archi_sexp_t *sexp)
{
    if (sexp == NULL)
        return;

    // Initialize the stack
    archi_sexp_t* stack[STACK_CAPACITY];

    size_t stack_size = 1;
    stack[0] = sexp;

    // Deallocate the whole S-expression tree
    while (stack_size > 0)
    {
        // Pop the stack
        sexp = stack[--stack_size];

        // Process both pointers of a cons-cell
        if (sexp->type == ARCHI_SEXP_TYPE_CONS)
        {
            // Process the 'rest' pointer
            if ((sexp->as_cons.rest != NULL) && (sexp->as_cons.rest != sexp->as_cons.first))
            {
                if (stack_size < STACK_CAPACITY) // push the stack
                    stack[stack_size++] = sexp->as_cons.rest;
                else // make a recursive call
                {
                    // Swap the pointers to keep the traverse order
                    archi_sexp_destroy_tree(sexp->as_cons.first);
                    sexp->as_cons.first = sexp->as_cons.rest;
                }
            }

            // Process the 'first' pointer
            if (sexp->as_cons.first != NULL)
            {
                if (stack_size < STACK_CAPACITY) // push the stack
                    stack[stack_size++] = sexp->as_cons.first;
                else // make a recursive call
                    archi_sexp_destroy_tree(sexp->as_cons.first);
            }
        }

        // Deallocate the S-expression
        free(sexp);
    }
}

