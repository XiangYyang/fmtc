// SPDX-License-Identifier: Apache-2.0
/**
 * @file    fmt.c
 * @brief   string formatting
 * @author  XiangYang
 */
#include "fmt.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief max internal buffer length (32bits binary value, so it's 32 bytes)
 */
#define MAX_INTERNAL_BUFF_LEN   32

/**
 * @brief default precision for quantized value
 */
#define DEFAULT_QUAT_VALUE_PREC 2

/**
 * @brief return min(a, b)
 */
#define MIN_VAL(a, b)           ((a) < (b) ? (a) : (b))

/**
 * @brief return max(a, b)
 */
#define MAX_VAL(a, b)           ((a) > (b) ? (a) : (b))

/**
 * @brief check if is a number
 */
#define IS_DIGIT(ch)            ((ch) >= '0' && (ch) <= '9')

/**
 * @brief check if is a align character (`<`, `>`, `^`)
 */
#define IS_ALIGN_CH(ch)         ((ch) == '<' || (ch) == '>' || (ch) == '^')

/**
 * @brief check if is a sign character (`+`, `-`, `_`)
 */
#define IS_SIGN_CH(ch)          ((ch) == '+' || (ch) == '-' || (ch) == ' ')

/**
 * @brief check if is a value_spec
 */
#define IS_VALUE_SPEC_CH(ch)                                                                \
    ((ch) == 'b' || (ch) == 'D' || (ch) == 'd' || (ch) == 'p' || (ch) == 'P' || (ch) == 'q' \
     || (ch) == 'h' || (ch) == 'H' || (ch) == 'x' || (ch) == 'X' || (ch) == 's')

/**
 * @brief convert a char (0 ~ 9) to uint32 value
 */
#define CHAR_TO_VAL(c) ((uint32_t)((c) - '0'))

#ifndef assert_true
/**
 * @brief Assert
 */
#define assert_true(expr) ((void)0)
#endif // assert_true

#ifndef static_assert
/**
 * @brief Static assert
 */
#define static_assert(cond, msg) ((void)0)
#endif // static_assert

#ifndef unimplemented
/**
 * @brief mark an unimplemented feature
 */
#define unimplemented() ((void)0)
#endif // unimplemented

#ifndef unreachable_branch
/**
 * @brief mark an unreachable branch
 */
#define unreachable_branch() ((void)0)
#endif // unimplemented

/**
 * @brief formatting action: copy value from buffer (bitmask)
 */
typedef enum FmtOper {
    FmtOp_None = 0x0,

    /**
     * @brief copy string from input arguments
     *
     */
    FmtOp_CopyStr = 0x1,

    /**
     * @brief copy string from internal buffer, and reversed it
     *
     */
    FmtOp_CopyBuffRev = 0x2,

    /**
     * @brief data bitmask
     *
     */
    FmtOp_CopyDataMask = 0x3,

    /**
     * @brief copy `0x` prefix
     *
     */
    FmtOp_Copy0x = 0x1 << 2,

    /**
     * @brief copy `0X` prefix
     *
     */
    FmtOp_Copy0X = 0x2 << 2,

    /**
     * @brief prefix bitmask
     *
     */
    FmtOp_CopyPrefixMask = 0xc,

    /**
     * @brief copy ` ` prefix
     *
     */
    FmtOp_CopySpace = 0x1 << 4,

    /**
     * @brief copy `+` prefix
     *
     */
    FmtOp_CopyAdd = 0x2 << 4,

    /**
     * @brief copy `-` prefix
     *
     */
    FmtOp_CopySub = 0x3 << 4,

    /**
     * @brief sign bitmask
     *
     */
    FmtOp_CopySignMask = 0x30,
} fmt_oper_t;

/**
 * @brief replacement parser output
 */
typedef struct ReplacementField {
    char fill_char;
    char sign_char;
    char align_char;
    char spec_char;
    bool cut_off;
    uint32_t width;
    uint32_t spec_val;
    uint32_t spec_val_prec;
} replacement_field_t;

/**
 * @brief calcuate x / 10, x % 10
 *
 * @param[in] x: unsigned value x
 * @param[out] div: x / 10 result
 * @param[out] rem: x % 10 result
 */
static void div_mod_10(uint32_t x, uint32_t* div, uint32_t* rem)
{
    uint32_t q, r;

    // calcuate q = x * 0.8
    q = (x >> 1) + (x >> 2);
    q += (q >> 4);
    q += (q >> 8);
    q += (q >> 16);

    // get x / 10 and x % 10
    q >>= 3;
    r = x - (q * 10);
    if (r > 9) {
        *div = q + 1;
        *rem = r - 10;
    } else {
        *div = q;
        *rem = r;
    }
}

/**
 * @brief convert a quantized value to a string
 */
static size_t fmt_impl_qtoa(char* buff, size_t prec, uint32_t quat, uint32_t value)
{
    (void)buff;
    (void)prec;
    (void)quat;
    (void)value;
    unimplemented();
    return 0;
}

/**
 * @brief convert an unsigned integer to a string
 *
 * @param[out] buff: result buffer
 * @param[in] index: value index, 2 / 8 / 10 / 16
 * @param[in] upper: use upper-case character in hexadecimal result
 * @param[in] value: integer value
 *
 * @attention The returned string needs to be reversed
 *
 * @return size_t : the number of characters written
 */
static size_t fmt_impl_utoa(char* buff, uint32_t index, bool upper, uint32_t value)
{
    size_t offset = 0;
    assert_true(index == 2 || index == 8 || index == 10 || index == 16);
    const char* const utoa_lower_digit = "0123456789abcdef";
    const char* const utoa_upper_digit = "0123456789ABCDEF";
    const char* const utoa_digit = upper ? utoa_upper_digit : utoa_lower_digit;

    if (index != 10) {
        uint32_t mask = index - 1;
        uint32_t shift_bits = index == 2 ? 1 : index == 8 ? 3 : index == 16 ? 4 : 0;
        do {
            buff[offset] = utoa_digit[value & mask];
            value >>= shift_bits;
            offset += 1;
        } while (value > 0);
    } else {
        do {
            uint32_t next_val = 0, rem_val = 0;

            div_mod_10(value, &next_val, &rem_val);

            buff[offset] = utoa_digit[rem_val];
            value = next_val;
            offset += 1;
        } while (value > 0);
    }

    buff[offset] = 0;

    return offset;
}

/**
 * @brief format a value to string
 *
 * @param[out] res: result output
 * @param[inout] max_sz: buff_sz, and return the number of characters written
 * @param[in] fmt_field: replacement field
 * @param[in] typ: argument type
 * @param[in] arg: argument value
 */
static fmt_error_t fmt_impl(
    char* res,
    size_t* max_sz,
    const replacement_field_t* fmt_field,
    fmt_type_id_t typ,
    intptr_t arg
)
{
    char buff[MAX_INTERNAL_BUFF_LEN];
    fmt_oper_t oper = FmtOp_None;
    fmt_error_t res_code = FmtError_Ok;

    // stage4: preprocess for integer type
    bool is_integer;
    uint32_t uint_val;
    if (typ == FmtTypeID_Int8 || typ == FmtTypeID_Int16 || typ == FmtTypeID_Int32) {
        is_integer = true;
        uint_val = (uint32_t)(arg < 0 ? -arg : arg);

        switch (fmt_field->sign_char) {
        case '+': oper |= arg < 0 ? FmtOp_CopySub : FmtOp_CopyAdd; break;
        case ' ': oper |= arg < 0 ? FmtOp_CopySub : FmtOp_CopySpace; break;
        default: oper |= arg < 0 ? FmtOp_CopySub : 0; break;
        }
    } else if (typ == FmtTypeID_UInt8 || typ == FmtTypeID_UInt16 || typ == FmtTypeID_UInt32) {
        is_integer = true;
        uint_val = (uint32_t)(arg);

        switch (fmt_field->sign_char) {
        case '+': oper |= FmtOp_CopyAdd; break;
        case ' ': oper |= FmtOp_CopySpace; break;
        default: break;
        }
    } else {
        is_integer = false;
    }

    // stage5: convert data
    size_t content_len = 0;
    if (is_integer) {
        // integer
        bool ival;
        bool upper;
        uint32_t index;
        switch (fmt_field->spec_char) {
        case 'b':
            ival = true;
            upper = false;
            index = 2;
            oper |= FmtOp_CopyBuffRev;
            break;
        case 'd':
        case '\0':
            ival = true;
            upper = false;
            index = 10;
            oper |= FmtOp_CopyBuffRev;
            break;
        case 'D':
        case 'h':
            ival = true;
            upper = false;
            index = 16;
            oper |= FmtOp_CopyBuffRev;
            break;
        case 'x':
            ival = true;
            upper = false;
            index = 16;
            oper |= FmtOp_CopyBuffRev | FmtOp_Copy0x;
            break;
        case 'H':
            ival = true;
            upper = true;
            index = 16;
            oper |= FmtOp_CopyBuffRev;
            break;
        case 'X':
            ival = true;
            upper = true;
            index = 16;
            oper |= FmtOp_CopyBuffRev | FmtOp_Copy0X;
            break;
        case 'q':
            ival = false;
            upper = false;
            index = 10;
            oper |= FmtOp_CopyBuffRev;
            break;
        default: res_code = FmtError_IntValueInvaildSpecChar; goto ret;
        };

        // convert it to string
        if (ival) {
            // integer
            content_len = fmt_impl_utoa(buff, index, upper, uint_val);
        } else {
            // quantized value (fixed-point value)
            uint32_t quat = fmt_field->spec_val;
            uint32_t prec = fmt_field->spec_val_prec;
            content_len = fmt_impl_qtoa(buff, prec, quat, uint_val);
        }
    } else if (typ == FmtTypeID_AnyPointer || typ == FmtTypeID_Int8Pointer) {
        // pointer
        uintptr_t value = (uintptr_t)arg;

        switch (fmt_field->spec_char) {
        case 'p':
            oper |= FmtOp_CopyBuffRev | FmtOp_Copy0x;
            content_len = fmt_impl_utoa(buff, 16, false, value);
            break;
        case 'P':
            oper |= FmtOp_CopyBuffRev | FmtOp_Copy0X;
            content_len = fmt_impl_utoa(buff, 16, true, value);
            break;
        case 's':
            if (typ != FmtTypeID_Int8Pointer) {
                res_code = FmtError_UInt8PointerValueInvaildSpecChar;
                goto ret;
            }

            oper |= FmtOp_CopyStr;
            content_len = strlen((const char*)value);
            break;
        case '\0':
            if (typ == FmtTypeID_Int8Pointer) {
                // default: string
                oper |= FmtOp_CopyStr;
                content_len = strlen((const char*)value);
            } else {
                // default: pointer (lower-case)
                oper |= FmtOp_CopyBuffRev | FmtOp_Copy0x;
                content_len = fmt_impl_utoa(buff, 16, false, value);
            }
            break;
        default: res_code = FmtError_PointerValueInvaildSpecChar; goto ret;
        }
    } else {
        content_len = 0;
        res_code = FmtError_UnsupportType;
        goto ret;
    }

    // stage6: copy the result to the buff
    char* res_buff = res;

    // stage6.1: calcuate the result length
    size_t need_len = content_len;
    if ((oper & (FmtOp_Copy0X | FmtOp_Copy0x)) != 0) {
        // need append `0x` or `0X` prefix
        need_len += 2;
    }

    if ((oper & (FmtOp_CopyAdd | FmtOp_CopySub | FmtOp_CopySpace)) != 0) {
        // need a sign character
        need_len += 1;
    }

    if (need_len > *max_sz) {
        res_code = FmtError_BufferTooSmall;
        goto ret;
    }

    // the length of data that need be copied
    size_t copy_len = fmt_field->cut_off ? MIN_VAL(fmt_field->width, need_len)
                                         : MAX_VAL(fmt_field->width, need_len);

    // stage6.2: fill characters if necessary
    size_t offset;
    if (copy_len > need_len) {
        char fill_char = fmt_field->fill_char;
        size_t fill_beg, fill_len;
        switch (fmt_field->align_char) {
        case '<':
            // align left, fill characters into right
            offset = 0;
            fill_beg = need_len;
            fill_len = copy_len - need_len;
            break;
        case '>':
            // align right, fill characters into left
            offset = copy_len - need_len;
            fill_beg = 0;
            fill_len = copy_len - need_len;
            break;
        case '^':
            // align to center, fill characters into buffer,
            // then the `copy_data` pass will overwrite the data
            offset = (copy_len - need_len) >> 1;
            fill_beg = 0;
            fill_len = copy_len;
            break;
        default: unreachable_branch();
        }

        // fill characters
        while (fill_len > 0) {
            res_buff[fill_beg] = fill_char;
            fill_beg += 1;
            fill_len -= 1;
        }
    } else {
        offset = 0;
    }

    // stage6.3: fill a sign character
    if ((oper & FmtOp_CopySignMask) == FmtOp_CopyAdd) {
        res_buff[offset] = '+';
        offset += 1;
    } else if ((oper & FmtOp_CopySignMask) == FmtOp_CopySub) {
        res_buff[offset] = '-';
        offset += 1;
    } else if ((oper & FmtOp_CopySignMask) == FmtOp_CopySpace) {
        res_buff[offset] = ' ';
        offset += 1;
    } else if ((oper & FmtOp_CopySignMask) == 0) {
        // nop
    } else {
        unreachable_branch();
    }

    // stage6.4: fill `0x` or `0X` prefix
    if ((oper & FmtOp_CopyPrefixMask) == FmtOp_Copy0x) {
        res_buff[offset + 0] = '0';
        res_buff[offset + 1] = 'x';
        offset += 2;
    } else if ((oper & FmtOp_CopyPrefixMask) == FmtOp_Copy0X) {
        res_buff[offset + 0] = '0';
        res_buff[offset + 1] = 'X';
        offset += 2;
    } else if ((oper & FmtOp_CopyPrefixMask) == 0) {
        // nop
    } else {
        unreachable_branch();
    }

    // stage6.5: copy data from internale buffer `buff` or `fmt` string
    if ((oper & FmtOp_CopyDataMask) == FmtOp_CopyBuffRev) {
        for (size_t i = 0; i < content_len; i += 1) {
            res_buff[offset + i] = buff[content_len - i - 1];
        }
    } else if ((oper & FmtOp_CopyDataMask) == FmtOp_CopyStr) {
        memcpy(res_buff + offset, (const char*)arg, content_len);
    } else if ((oper & FmtOp_CopyDataMask) == 0) {
        // nop
    } else {
        unreachable_branch();
    }

    *max_sz = copy_len;
    *(res + copy_len) = 0;
ret:
    return res_code;
}

/**
 * @brief formatting to string, with typeinfo
 *
 * @param[in] fmt_str: formatting string
 * @param[out] buff: formatting result output
 * @param[inout] max_sz: buff_sz, and return the number of characters written
 * @param[in] n_args: varargs length
 * @param[in] v_args: va_list
 */
static fmt_error_t fmt_tobuff_varg_impl(
    const char* restrict fmt_str,
    char* restrict buff,
    size_t* max_sz,
    size_t n_args,
    va_list v_args
)
{
    // arguments buffer
    size_t rem_sz = 0;
    size_t arg_used = 0;
    intptr_t arg_vptr[FMT_MAX_ARGN];
    fmt_type_id_t arg_type[FMT_MAX_ARGN];

    // parse the string
    char ch = '\0';
    const char* fmt = fmt_str;
    char* const buff_beg = buff;
    char* const buff_end = buff + *max_sz;
    fmt_error_t res_code = FmtError_Ok;

    // current argument index
    // because some reasons, it starts with 1
    size_t arg_index = 0;

    // stage1: parse input formatting string
    intptr_t cur_arg;
    fmt_type_id_t cur_typeid;
    replacement_field_t cur_field;

parse_next:
    if (!!(!*fmt || buff >= buff_end)) {
        goto ret;
    } else {
        goto parse_one_char;
    }

parse_one_char:
    // find the replacement_field
    // if not a replacement_field, copy to the buffer
    if (*fmt == '{') {
        goto is_replacement_field;
    } else if (*fmt == '}') {
        goto fold_right_bracket;
    } else {
        // not a replacement_field
        *buff = *fmt;
        fmt += 1;
        goto accept_char;
    }

fold_right_bracket:
    *buff = '}';

    if (*(fmt + 1) == '}') {
        // `}}`
        fmt += 2;
    } else {
        // `}`
        fmt += 1;
    }

    goto accept_char;

is_replacement_field:
    // replacement_field ?
    if (*(fmt + 1) == '{') {
        // `{{`
        *buff = '{';
        fmt += 2;
        goto accept_char;
    } else {
        // replacement field
        goto replacement_field;
    }

accept_char:
    buff += 1;
    goto parse_next;

replacement_field:
    // parse replacement_field
    fmt += 1;

    // load default value
    cur_field = (replacement_field_t) {
        .fill_char = ' ',
        .sign_char = '-',
        .align_char = '<',
        .width = 0,
        .cut_off = false,
        .spec_char = '\0',
        .spec_val = 0,
        .spec_val_prec = DEFAULT_QUAT_VALUE_PREC,
    };

    // parse format_spec
    ch = *fmt;
    if (IS_DIGIT(ch)) {
        // arg_idx
        fmt += 1;
        arg_index = (size_t)CHAR_TO_VAL(ch) + 1;
        if (*fmt == ':') {
            // format_spec
            fmt += 1;
            goto format_spec;
        } else {
            goto replacement_field_end;
        }
    } else if (ch == ':') {
        // format_spec
        fmt += 1;
        arg_index += 1;
        goto format_spec;
    } else if (ch == '}') {
        arg_index += 1;
        goto replacement_field_end;
    } else {
        res_code = FmtError_InvaildTokenAfterLeftBracket;
        goto ret;
    }

format_spec:
    // format_spec begin
    ch = *fmt;

    if (IS_ALIGN_CH(*(fmt + 1))) {
        // current is fill_char
        cur_field.fill_char = ch;
        fmt += 1;
        goto align;
    } else if (IS_ALIGN_CH(ch)) {
        goto align;
    } else if (IS_DIGIT(ch)) {
        goto width;
    } else if (IS_SIGN_CH(ch)) {
        goto sign;
    } else if (ch != '}') {
        goto spec;
    } else {
        goto replacement_field_end;
    }

align:
    // align, `<`, `>`, and `=`
    ch = *fmt;
    assert_true(IS_ALIGN_CH(ch));
    cur_field.align_char = ch;
    fmt += 1;

    // peek next character
    ch = *fmt;
    if (IS_DIGIT(ch)) {
        goto width;
    } else if (IS_SIGN_CH(ch)) {
        goto sign;
    } else if (ch != '}') {
        goto spec;
    } else {
        goto replacement_field_end;
    }

sign:
    // sign symbol, `+`, `-`, and ` `
    ch = *fmt;
    assert_true(IS_SIGN_CH(ch));
    cur_field.sign_char = ch;
    fmt += 1;

    // peek next character
    ch = *fmt;
    if (IS_DIGIT(ch)) {
        goto width;
    } else if (ch != '}') {
        goto spec;
    } else {
        goto replacement_field_end;
    }

width:
    // content width
    ch = *fmt;
    assert_true(IS_DIGIT(ch));
    cur_field.width = (cur_field.width * 10) + CHAR_TO_VAL(ch);
    fmt += 1;

    // peek next character
    ch = *fmt;
    if (IS_DIGIT(ch)) {
        goto width;
    } else if (ch == '!') {
        // cut-off symbol
        cur_field.cut_off = true;
        fmt += 1;
        goto width_next;
    } else {
        goto width_next;
    }

width_next:
    // peek next character
    ch = *fmt;
    if (ch != '}') {
        goto spec;
    } else {
        goto replacement_field_end;
    }

spec:
    // value spec
    ch = *fmt;
    if (!IS_VALUE_SPEC_CH(ch)) {
        res_code = FmtError_InvaildSpecChar;
        goto ret;
    }

    fmt += 1;
    cur_field.spec_char = ch;

    // quantized value has digits suffix
    if (ch == 'q') {
        if (!IS_DIGIT(*fmt)) {
            res_code = FmtError_MissingNumberAfterQSpec;
            goto ret;
        } else {
            goto spec_q;
        }
    } else {
        goto replacement_field_end;
    }

spec_q:
    ch = *fmt;
    assert_true(IS_DIGIT(ch));
    cur_field.spec_val = (cur_field.spec_val * 10) + CHAR_TO_VAL(ch);
    fmt += 1;

    if (IS_DIGIT(*fmt)) {
        goto spec_q;
    } else if (*fmt == '.') {
        goto spec_q_point;
    } else {
        goto replacement_field_end;
    }

spec_q_point:
    ch = *fmt;
    assert_true(*fmt == '.');
    fmt += 1;

    if (IS_DIGIT(*fmt)) {
        goto spec_q_prec;
    } else {
        res_code = FmtError_MissingPrecisionAfterQSpec;
        goto ret;
    }

spec_q_prec:
    ch = *fmt;
    assert_true(IS_DIGIT(ch));
    cur_field.spec_val_prec = (cur_field.spec_val_prec * 10) + CHAR_TO_VAL(ch);
    fmt += 1;

    if (IS_DIGIT(*fmt)) {
        goto spec_q;
    } else {
        goto replacement_field_end;
    }

replacement_field_end:
    // check the right bracket
    ch = *fmt;
    if (ch != '}') {
        res_code = FmtError_MissingRightBracket;
        goto ret;
    }

    fmt += 1;

    // stage2: get arguments from va_list or arguments buffer
    if (arg_index == arg_used + 1 && arg_index <= n_args) {
        // get it from va_list
        cur_typeid = (fmt_type_id_t)va_arg(v_args, int);

        // to avoid an undefined behavior,
        // the following code assume same_as<int32_t, int>.
        static_assert(sizeof(int) == sizeof(uint32_t), "unsigned int must same as uint32_t");

        switch (cur_typeid) {
        case FmtTypeID_UInt8:
        case FmtTypeID_UInt16:
        case FmtTypeID_UInt32: cur_arg = (intptr_t)va_arg(v_args, unsigned int); break;
        case FmtTypeID_Int8:
        case FmtTypeID_Int16:
        case FmtTypeID_Int32: cur_arg = (intptr_t)va_arg(v_args, int); break;
        case FmtTypeID_Bool: cur_arg = (intptr_t)va_arg(v_args, int); break;
        case FmtTypeID_AnyPointer:
        case FmtTypeID_Int8Pointer: cur_arg = (intptr_t)va_arg(v_args, const char*); break;
        case FmtTypeID_Any: res_code = FmtError_UnsupportType; goto ret;
        default: unreachable_branch();
        }

        arg_vptr[arg_used] = cur_arg;
        arg_type[arg_used] = cur_typeid;

        arg_used += 1;
        goto fmt_data;
    } else if (arg_index <= arg_used && arg_index <= n_args) {
        // get it from arguments buffer
        cur_arg = arg_vptr[arg_index - 1];
        cur_typeid = arg_type[arg_index - 1];
        goto fmt_data;
    } else {
        // invaild index
        res_code = FmtError_InvaildIndex;
        goto ret;
    }

fmt_data:
    // stage3: format the value
    assert_true(*max_sz >= (size_t)(buff - buff_beg));
    rem_sz = *max_sz - (size_t)(buff - buff_beg);
    res_code = fmt_impl(buff, &rem_sz, &cur_field, cur_typeid, cur_arg);

    if (res_code != FmtError_Ok) {
        goto ret;
    } else {
        buff += rem_sz;
        goto parse_next;
    }

ret:
    // resule length
    *max_sz = (size_t)(buff - buff_beg);
    return res_code;
}

//
// public (va_args version)
//
fmt_error_t fmt_tobuff_impl(
    const char* restrict fmt, char* restrict buff, size_t* max_sz, size_t n_args, ...
)
{
    va_list args;
    va_start(args, n_args);

    fmt_error_t ret = fmt_tobuff_varg_impl(fmt, buff, max_sz, n_args, args);

    va_end(args);
    return ret;
}

//
// public (va_list version)
//
fmt_error_t fmt_tobuff_varg(
    const char* restrict fmt, char* restrict buff, size_t* max_sz, size_t n_args, va_list v_args
)
{
    assert_true(buff != nullptr);
    assert_true(fmt != nullptr);
    assert_true(max_sz > 0);
    assert_true(n_args <= FMT_MAX_ARGN);

    return fmt_tobuff_varg_impl(fmt, buff, max_sz, n_args, v_args);
}
