# FMT

A lightweight, type-safe string formatting library for microcontroller, inspired by PEP-3101.

## Features

- Type Info: no manual format specifier needed -- `fmt("{}", 42) -> "42"`
- Alignment: alignment, fill, width, and forced cut-off
- Integer fmt: binary, decimal, hex (with/without prefix), BCD
- Pointer and string formatting
- Positional arguments

## Supported types

- `bool`
- `i8` ~ `i32`
- `u8` ~ `u32`
- `char*` (string)
- `void*` (pointer)

## Syntax

```plaintext
replacement_field :=
   `{` arg_idx [`:` [ [fill] align ] [ sign ] [ width [ `!` ] ] [ spec ] `}`

arg_idx :=
   digit

fill :=
   any character                     default = ` `

align :=
   <        -- align to left         default
   >        -- align to right
   ^        -- align to center

sign :=
   ` `      -- ` ` or `-`            number type ONLY
   +        -- `+` or `-`            number type ONLY
   -        -- `-` if value < 0      default, number type ONLY

width :=
   digits

spec :=
   b        -- binary
   D        -- BCD                   same as hex, used for BCD values
   d        -- dec                   default
   p        -- pointer               hex, lower-case
   P        -- pointer               hex, upper-case
   q prec   -- quantized value       (not implemented yet)
   h        -- hex
   H        -- hex                   upper-case
   x        -- hex                   with prefix `0x`
   X        -- hex                   with prefix `0X`, upper-case

prec :=
   digits [`.` digits]

digits :=
   one or more digit

digit :=
   one of
      0 1 2 3 4 5 6 7 8 9
```

## Run examples

```bash
$ clang -std=c11 fmt.c examples.c -O2 -o examples && ./examples

fmt("{}", "Hello")   -> "Hello"

fmt("{}", 42)        -> 42

fmt("{:d}", 42)      -> 42
fmt("{:h}", 42)      -> 2a
fmt("{:x}", 42)      -> 0x2a

fmt("{:>8d}", 42)    -> |      42|
fmt("{:<8d}", 42)    -> |42      |
fmt("{:^8d}", 42)    -> |   42   |

fmt("{}", "QwQ")     -> QwQ
fmt("{:p}", "QwQ")   -> 0x2649538

fmt("0:{} 1:{} 1:{1} 0:{0}", 42, "QwQ")   -> 0:42 1:QwQ 1:QwQ 0:42
```
