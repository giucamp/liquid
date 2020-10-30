
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/tensor.h"
#include "expression.h"

namespace liquid
{
    std::ostream & operator << (std::ostream & i_dest, const Tensor & i_tensor)
    {
        if(IsConstant(i_tensor))
        {
            switch(i_tensor.GetScalarType())
            {
                case ScalarType::Real: i_dest << GetConstantStorage<Real>(i_tensor); break;
                case ScalarType::Integer: i_dest << GetConstantStorage<Integer>(i_tensor); break;
                case ScalarType::Bool: i_dest << GetConstantStorage<Bool>(i_tensor); break;
                default: Panic("PrintTensor: unsupported scalar type");
            }
        }
        else if(IsVariable(i_tensor))
        {
            i_dest << i_tensor.GetExpression()->GetName();
        }
        else
            i_dest << i_tensor.GetExpression()->GetOperator().GetName() 
                << "(" << Span(i_tensor.GetExpression()->GetOperands()) << ")";
        return i_dest;
    }
}
