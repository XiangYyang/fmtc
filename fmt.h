// SPDX-License-Identifier: Apache-2.0
/**
 * @file    fmt.h
 * @brief   string formatting
 * @author  XiangYang
 */
#if !defined(_INCLUDE_FMT_H_)
#define _INCLUDE_FMT_H_ 1
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef FMT_MAX_IOBUFF_LEN
/**
 * @brief configure: max io buffer length (64bytes default)
 */
#define FMT_MAX_IOBUFF_LEN 64
#endif // FMT_MAX_IOBUFF_LEN

#ifndef FMT_USE_SHIFT_MULMOD
/**
 * @brief configure: use shift operator insteads multiply operator
 */
#define FMT_USE_SHIFT_MULMOD 1
#endif // FMT_USE_SHIFT_MULMOD

#ifndef FMT_DEFAULT_IOWRITE_IMPL
/**
 * @brief configure: use an empty __weak IOwrite impl
 */
#define FMT_DEFAULT_IOWRITE_IMPL 1
#endif // FMT_DEFAULT_IOWRITE_IMPL

/**
 * @brief Max argument number
 */
#define FMT_MAX_ARGN 9

/**
 * @brief get type_id from a express
 *
 * @attention This macro requires:
 *
 *  + same_as< char,  int8_t  >
 *  + same_as< char,  int8_t  >
 *  + same_as< short, int16_t >
 *  + same_as< int,   int32_t >
 *  + same_as< long,  int32_t >
 *
 * that's satisfied normally on 32-bit target (e.g. thumbv6m-none-eabi)
 */
#define FMT_TYPEID_OF(expr)                 \
    _Generic(                               \
        expr,                               \
        bool: FmtTypeID_Bool,               \
        char: FmtTypeID_Int8,               \
        short: FmtTypeID_Int16,             \
        int: FmtTypeID_Int32,               \
        long: FmtTypeID_Int32,              \
        unsigned char: FmtTypeID_UInt8,     \
        unsigned short: FmtTypeID_UInt16,   \
        unsigned int: FmtTypeID_UInt32,     \
        unsigned long: FmtTypeID_UInt32,    \
        char*: FmtTypeID_Int8Pointer,       \
        short*: FmtTypeID_AnyPointer,       \
        int*: FmtTypeID_AnyPointer,         \
        long*: FmtTypeID_AnyPointer,        \
        void*: FmtTypeID_AnyPointer,        \
        const char*: FmtTypeID_Int8Pointer, \
        const short*: FmtTypeID_AnyPointer, \
        const int*: FmtTypeID_AnyPointer,   \
        const long*: FmtTypeID_AnyPointer,  \
        const void*: FmtTypeID_AnyPointer,  \
        default: FmtTypeID_Any              \
    )

/**
 * @brief stupid helper macro, get the number of arguments
 */
#define FMT_HELPER_VA_ARGS_N(...) \
    FMT_HELPER_VA_ARGS_N_INNER(0 __VA_OPT__(, ) __VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define FMT_HELPER_VA_ARGS_N_INNER(_9, _8, _7, _6, _5, _4, _3, _2, _1, _0, n, ...) n

/**
 * @brief concat two symbol
 */
#define FMT_HELPER_CONCAT_SYMBOL(a, b)  FMT_HELPER_CONCAT_SYMBOL1(a, b)
#define FMT_HELPER_CONCAT_SYMBOL1(a, b) FMT_HELPER_CONCAT_SYMBOL2(a, b)
#define FMT_HELPER_CONCAT_SYMBOL2(a, b) a##b

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=0
 */
#define FMT_HELPER_TYPEARG_0(fn, r0, r1, r2, r3, ...) \
    fn(r0, r1, r2, r3 __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=1
 */
#define FMT_HELPER_TYPEARG_1(fn, r0, r1, r2, r3, a1, ...) \
    FMT_HELPER_TYPEARG_0(fn, r0, r1, r2, r3, FMT_TYPEID_OF(a1), a1 __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=2
 */
#define FMT_HELPER_TYPEARG_2(fn, r0, r1, r2, r3, a1, a2, ...)                    \
    FMT_HELPER_TYPEARG_1(                                                        \
        fn, r0, r1, r2, r3, a1, FMT_TYPEID_OF(a2), a2 __VA_OPT__(, ) __VA_ARGS__ \
    )

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=3
 */
#define FMT_HELPER_TYPEARG_3(fn, r0, r1, r2, r3, a1, a2, a3, ...)                    \
    FMT_HELPER_TYPEARG_2(                                                            \
        fn, r0, r1, r2, r3, a1, a2, FMT_TYPEID_OF(a3), a3 __VA_OPT__(, ) __VA_ARGS__ \
    )

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=4
 */
#define FMT_HELPER_TYPEARG_4(fn, r0, r1, r2, r3, a1, a2, a3, a4, ...)                    \
    FMT_HELPER_TYPEARG_3(                                                                \
        fn, r0, r1, r2, r3, a1, a2, a3, FMT_TYPEID_OF(a4), a4 __VA_OPT__(, ) __VA_ARGS__ \
    )

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=5
 */
#define FMT_HELPER_TYPEARG_5(fn, r0, r1, r2, r3, a1, a2, a3, a4, a5, ...)                    \
    FMT_HELPER_TYPEARG_4(                                                                    \
        fn, r0, r1, r2, r3, a1, a2, a3, a4, FMT_TYPEID_OF(a5), a5 __VA_OPT__(, ) __VA_ARGS__ \
    )

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=6
 */
#define FMT_HELPER_TYPEARG_6(fn, r0, r1, r2, r3, a1, a2, a3, a4, a5, a6, ...) \
    FMT_HELPER_TYPEARG_5(                                                     \
        fn,                                                                   \
        r0,                                                                   \
        r1,                                                                   \
        r2,                                                                   \
        r3,                                                                   \
        a1,                                                                   \
        a2,                                                                   \
        a3,                                                                   \
        a4,                                                                   \
        a5,                                                                   \
        FMT_TYPEID_OF(a6),                                                    \
        a6 __VA_OPT__(, ) __VA_ARGS__                                         \
    )

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=7
 */
#define FMT_HELPER_TYPEARG_7(fn, r0, r1, r2, r3, a1, a2, a3, a4, a5, a6, a7, ...) \
    FMT_HELPER_TYPEARG_6(                                                         \
        fn,                                                                       \
        r0,                                                                       \
        r1,                                                                       \
        r2,                                                                       \
        r3,                                                                       \
        a1,                                                                       \
        a2,                                                                       \
        a3,                                                                       \
        a4,                                                                       \
        a5,                                                                       \
        a6,                                                                       \
        FMT_TYPEID_OF(a7),                                                        \
        a7 __VA_OPT__(, ) __VA_ARGS__                                             \
    )

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=8
 */
#define FMT_HELPER_TYPEARG_8(fn, r0, r1, r2, r3, a1, a2, a3, a4, a5, a6, a7, a8, ...) \
    FMT_HELPER_TYPEARG_7(                                                             \
        fn,                                                                           \
        r0,                                                                           \
        r1,                                                                           \
        r2,                                                                           \
        r3,                                                                           \
        a1,                                                                           \
        a2,                                                                           \
        a3,                                                                           \
        a4,                                                                           \
        a5,                                                                           \
        a6,                                                                           \
        a7,                                                                           \
        FMT_TYPEID_OF(a8),                                                            \
        a8 __VA_OPT__(, ) __VA_ARGS__                                                 \
    )

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N=9
 */
#define FMT_HELPER_TYPEARG_9(fn, r0, r1, r2, r3, a1, a2, a3, a4, a5, a6, a7, a8, a9, ...) \
    FMT_HELPER_TYPEARG_8(                                                                 \
        fn,                                                                               \
        r0,                                                                               \
        r1,                                                                               \
        r2,                                                                               \
        r3,                                                                               \
        a1,                                                                               \
        a2,                                                                               \
        a3,                                                                               \
        a4,                                                                               \
        a5,                                                                               \
        a6,                                                                               \
        a7,                                                                               \
        a8,                                                                               \
        FMT_TYPEID_OF(a9),                                                                \
        a9 __VA_OPT__(, ) __VA_ARGS__                                                     \
    )

/**
 * @brief expand macro symbol name for @ref FMT_HELPER_TYPEARG_N
 */
#define FMT_HELPER_TYPEARG_N_SYM(...) \
    FMT_HELPER_CONCAT_SYMBOL(FMT_HELPER_TYPEARG_, FMT_HELPER_VA_ARGS_N(__VA_ARGS__))

/**
 * @brief expand @ref FMT_HELPER_TYPEARG_N at N, the N need less_than 9
 */
#define FMT_HELPER_TYPEARG_N(fn, r0, r1, r2, ...) \
    FMT_HELPER_TYPEARG_N_SYM(__VA_ARGS__)         \
    (fn, r0, r1, r2, FMT_HELPER_VA_ARGS_N(__VA_ARGS__) __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief deduce two arguments in fmt_tobuff_impl
 */
#define FMT_HELPER_IMPL_IOWRAPPER(fmt, buff, max_sz, n_args, ...) \
    fmt_io_impl(fmt, n_args __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief formatting to string, with typeinfo, see also @ref fmt_tobuff_impl
 */
#define fmt_tobuff(fmt_str, buff, max_len, ...) \
    FMT_HELPER_TYPEARG_N(fmt_tobuff_impl, fmt_str, buff, max_len __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief formatting to string and write to io, with typeinfo, see also @ref fmt_io_impl
 */
#define fmt_write(fmt_str, ...) \
    FMT_HELPER_TYPEARG_N(FMT_HELPER_IMPL_IOWRAPPER, fmt_str, 0, 0 __VA_OPT__(, ) __VA_ARGS__)

/**
 * @brief typeid
 */
typedef enum FmtTypeID {
    FmtTypeID_Bool,
    FmtTypeID_Int8,
    FmtTypeID_Int16,
    FmtTypeID_Int32,
    FmtTypeID_UInt8,
    FmtTypeID_UInt16,
    FmtTypeID_UInt32,
    FmtTypeID_AnyPointer,
    FmtTypeID_Int8Pointer,
    FmtTypeID_Any,
} fmt_type_id_t;

/**
 * @brief error code
 */
typedef enum FmtError {
    /**
     * @brief succ
     *
     */
    FmtError_Ok = 0,

    /**
     * @brief buffer is too small
     *
     */
    FmtError_BufferTooSmall = -31,

    /**
     * @brief syntax error: missing `}`
     *
     */
    FmtError_MissingRightBracket,

    /**
     * @brief invaild token after `{`
     *
     */
    FmtError_InvaildTokenAfterLeftBracket,

    /**
     * @brief invaild spec character
     *
     */
    FmtError_InvaildSpecChar,

    /**
     * @brief missing a number after quat-value spec char
     *
     */
    FmtError_MissingNumberAfterQSpec,

    /**
     * @brief missing a precision number after quat-value spec char
     *
     */
    FmtError_MissingPrecisionAfterQSpec,

    /**
     * @brief unsupport type
     *
     */
    FmtError_UnsupportType,

    /**
     * @brief invaild argumnet index
     *
     */
    FmtError_InvaildIndex,

    /**
     * @brief invaild spec-char for integer type
     *
     */
    FmtError_IntValueInvaildSpecChar,

    /**
     * @brief invaild spec-char for pointer type
     *
     */
    FmtError_PointerValueInvaildSpecChar,

    /**
     * @brief invaild spec-char for char* type
     *
     */
    FmtError_UInt8PointerValueInvaildSpecChar,

    /**
     * @brief IO error (returned by fmt_io_impl ONLY)
     *
     */
    FmtError_IOError,
} fmt_error_t;

#if __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief formatting to string, with typeinfo
 *
 * @param[in] fmt: formatting string
 * @param[out] buff: result output
 * @param[inout] max_sz: result buffer size, and return the number of characters written
 * @param[in] n_args: varargs length
 */
extern fmt_error_t
    fmt_tobuff_impl(const char* fmt, char* buff, size_t* max_sz, size_t n_args, ...);

/**
 * @brief formatting to string, with typeinfo and va_list
 *
 * @param[in] fmt: formatting string
 * @param[out] buff: result output
 * @param[inout] max_sz: result buffer size, and return the number of characters written
 * @param[in] n_args: varargs length
 * @param[in] v_args: varargs list
 */
extern fmt_error_t
    fmt_tobuff_varg(const char* fmt, char* buff, size_t* max_sz, size_t n_args, va_list v_args);

/**
 * @brief formatting to string and write it into IObuff by @ref fmt_write_io_impl
 *
 * @attention requires implementation of @ref fmt_write_io_impl
 *
 * @param[in] fmt: formatting string
 * @param[in] n_args: varargs length
 */
extern fmt_error_t fmt_io_impl(const char* fmt, size_t n_args, ...);

/**
 * @brief external implement: write string to IO
 *
 */
extern fmt_error_t fmt_write_io_impl(const char* buff, size_t write_len);

#if __cplusplus
}
#endif // __cplusplus
#endif // _INCLUDE_FMT_H_
