# liquid
C++ tensor algebra &amp; compiler

Liquid is a work in progress, and currently it's not functional. A working cpu-only backend is planned for the end of 2020.

## Tensor Types
*scalar_type* ← `real`|`int`|`bool`|`any`
*tensor_type*  ← *scalar_type* [*shape_vector*]

A tensor type is composed by a scalar type (real, int, bool or any) and a shape vector (any tensor expression with rank 1). If the scalar type is any the type is dynamic. If the shape vector is omitted the tensor is shapeless.

`int [3, 4]`: integral matrix (rank is `[2]` )
`bool [3, 4, 5, 6]` bool tensor (rank is 4)
`any [3]` dynamic vector with 3 elements
`real[]` real scalar (rank is 0)
`any` dynamic shapeless tensor

## Variable declaration

*variable_expr*  ← *tensor_type*
A variable declaration is an expression that introduce a possibly named vector whose value is not known. In order to evaluate an expression all variable must be substituted with constan 
Liquid deduces tensor types

## Corollaries
Corollaries are boolean expressions that evaluates always to true. A corollary may involve variable tensors, as long as liquid can prove the true-ness of the statement. 

## Stack Operator [ ]
*tensor* ← [*tensor**]
The stack operator aggregates a list of tensors with the same rank *r* and with the same shape to a single tensor of rank *r+1*

`[1 2]`: this is a vector, with type int [2] 
`[[1 2][3,4][3,4]]`: this is a matrix with type int [3, 2]

    [[log [3 4]
      exp([5 6] + 2)]]

A matrix 

    [[ cos real θ  -sin θ ]
     [ sin θ       cos θ  ]]
^ a rotation matrix

## if Operator
if_expression  ← if bool_expression then

# principles
- no class hierarchies, no virtual functions
- write as few code as possible
- tests are part of documentation
- low-level code is in C++,  high-level code is in miu6 

<!--stackedit_data:
eyJoaXN0b3J5IjpbNjU2Mzk3NDk0LC0xNjM2MTk5MTU1XX0=
-->