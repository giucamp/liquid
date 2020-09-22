
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include "operator.h"
#include "tensor_value.h"

namespace liquid
{
    TensorType VariableDeduceType(const std::any& i_attachment,
        [[maybe_unused]] Span<const Tensor> i_operands,
        [[maybe_unused]] Span<const Tensor> i_attributes)
    {
        return std::any_cast<TensorType>(i_attachment);
    }

    extern const Operator & GetOperatorVariable()
    {
        static auto const op = Operator("Variable")
            .SetDeduceType(VariableDeduceType);
        return op;
    }

    Tensor MakeVariable(const TensorType & i_type, std::string_view i_name)
    {
        return GetOperatorVariable().Invoke({}, {}, i_type);
    }

    bool IsVariable(const Tensor& i_tensor)
    {
        return i_tensor.GetExpression()->OperatorIs(GetOperatorVariable());
    }
}
