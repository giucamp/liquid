
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"

namespace liquid
{
    Expression::Expression(std::string_view i_name,
        std::string_view i_doc,
        const TensorType& i_type,
        const Operator& i_operator,
        const Operator::Overload & i_overload,
        Span<Tensor const> i_operands,
        Span<Tensor const> i_attributes,
        const std::any& i_attachment)
    : m_name(i_name),
      m_doc(i_doc),
      m_type(i_type),
      m_operator(i_operator),
      m_overload(i_overload),
      m_operands(i_operands.begin(), i_operands.end()),
      m_attributes(i_attributes.begin(), i_operands.end()),
      m_attachment(i_attachment)
    {

    }

    bool AlwaysEqual(const Tensor & i_tensor, const TensorValue& i_value)
    {
        return IsConstant(i_tensor) && GetConstantValue(i_tensor) == i_value;
    }

    TensorType DeduceType(Span<const Tensor> i_operands)
    {
        return DeduceType(Transform(i_operands, 
            [](const Tensor & i_tensor){ return i_tensor.GetExpression()->GetType(); }));
    }
}
