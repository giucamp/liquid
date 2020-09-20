
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include "operator.h"
#include "tensor_value.h"
#include "tensor_type.h"
#include "indices.h"

namespace liquid
{
    template <typename SCALAR_TYPE>
        TensorValue AddEvaluate(const TensorType & i_result_type, Span<const TensorValue> i_operands)
    {
        SharedArray<SCALAR_TYPE> result(i_result_type.GetConstantShape().GetLinearSize());
        for (Indices indices(i_result_type); indices; indices++)
        {
            SCALAR_TYPE element = {};
            for(const TensorValue & operand : i_operands)
                element += indices[operand];
            indices[result] = element;
        }
        return TensorValue(i_result_type, result);
    }

    extern const Operator & GetOperatorAdd()
    {
        static auto op = Operator("Add").
            AddFlags(Operator::Flags::Commutative | Operator::Flags::Associative);
        return op;
    }
}
