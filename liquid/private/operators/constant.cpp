
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include "operator.h"
#include "tensor_value.h"

namespace liquid
{
    TensorType ConstantDeduceType(const std::any& i_attachment, 
        [[maybe_unused]] Span<const Tensor> i_operands, 
        [[maybe_unused]] Span<const Tensor> i_attributes)
    {
        return std::any_cast<const TensorValue &>(i_attachment).GetType();
    }

    TensorValue ConstantEvaluate(const std::any & i_attachment,
        [[maybe_unused]] const TensorType & i_result_type,
        [[maybe_unused]] Span<const TensorValue> i_operands,
        [[maybe_unused]] Span<const TensorValue> i_attributes)
    {
        return std::any_cast<TensorValue>(i_attachment);
    }

    extern const Operator & GetOperatorConstant()
    {
        static auto const op = Operator("Constant")
            .SetDeduceType(ConstantDeduceType)
            .AddOverload({ ConstantEvaluate, { }, { } });
        return op;
    }

    Tensor MakeConstant(const TensorValue & i_value)
    {
        return GetOperatorConstant().Invoke({}, {}, i_value);
    }

    bool IsConstant(const Tensor & i_tensor)
    {
        return i_tensor.GetExpression()->OperatorIs(GetOperatorConstant());
    }

    const TensorValue GetConstantValue(const Tensor & i_tensor)
    {
        if(!IsConstant(i_tensor))
            Panic("GetConstantValue - not a constant tensor");
        return std::any_cast<TensorValue>(i_tensor.GetExpression()->GetAttachment());
    }
}
