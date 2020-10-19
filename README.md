

# liquid
C++ tensor algebra &amp; compiler

Liquid is a work in progress, and currently it's not functional. A working cpu-only backend is planned for the end of 2020.

## Tensor Types
*scalar_type* ← `real`|`int`|`bool`|`any`

*tensor_type*  ← *scalar_type* [*shape_vector*]

A tensor type is composed by a scalar type (real, int, bool or any) and a shape vector (any tensor expression with rank 1). If the scalar type is any the type is dynamic. If the shape vector is omitted the tensor is shapeless.

`int [3, 4]`: integral matrix (rank is 2)

`bool [3, 4, 5, 6]` bool tensor (rank is 4)

`any [3]` dynamic vector with 3 elements

`real[]` real scalar (rank is 0)

`any` dynamic shapeless tensor

## Corollaries
Corollaries are boolean expressions that evaluate always to true. A corollary may involve variable tensors, as long as liquid can prove the trueness of the statement. Corollaries listed here are parte of the test program.

## 'is' Operator 
*is_expression* ← *expression* `is` *tensor_type*

'is' operator returns a scalar boolean that is true if the type of the left-hand side expression matches the right-hand side type. 

## Variable declaration

*variable_expr*  ← *tensor_type* [*name*]

A variable declaration is an expression that introduce a possibly named vector whose value is not known. In order to evaluate an expression all variable must be substituted with constant vector, or the values must be provided to a tensor program compiled by a backend.  
The name in the declaration may be omitted, in which case the variable is unnamed.

Some corollaries:

`real * 0 * real == 0`: if any factor is zero, the product is zero

`real a * int b is real`: the product of an integer and a real is a real. Actually the names a and b are unnecessary here.

`int / int is real`: a similar corollary, showing that a quotient is always real 

`any is any is bool`: regardless of operands, is always returns a boolean

`(real == real) is bool`: the comparison of tensors returns a bool tensor (not a bool scalar)

`(rank of any) is int[]`: the operator rank gives a scalar integer, regardless of the type of the operand.

    

## Stack Operator [ ]
*tensor* ← [*tensor**]

The stack operator aggregates a list of tensors with the same rank *r* and with the same shape to a single tensor of rank *r+1*

Some corollaries:

    [1 2] is int[2]

    [ log[ 3 real[] ]
      exp[5 6] + 2  ] is real [2, 2]

    [[ cos real[] θ     -sin θ ]
     [ sin θ             cos θ ]] is real[2 2]

## if Operator

Some corollaries:

    (if 5 > 6 then 1 else 2) == 2
    
    (if [-2 2.] > 0 then 100 else 200) == [200 100]


# principles
- no class hierarchies, no virtual functions
- write as few code as possible
- tests are part of documentation
- low-level code is in C++,  high-level code is in miu6 

> Written with [StackEdit](https://stackedit.io/).
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTExNTI0MjU1MzUsOTU3ODcxNTI2LC0xNj
M2MTk5MTU1XX0=
-->
