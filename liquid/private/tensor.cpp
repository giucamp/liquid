
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/tensor.h"
#include "expression.h"

namespace liquid
{
    Tensor::Tensor(const TensorInitializer & i_scalars)
        : m_expression(MakeConstant(TensorValue(i_scalars)).GetExpression())
    {

    }

    Tensor::Tensor(const TensorInitializer & i_scalars, Span<const Integer> i_shape)
        : m_expression(MakeConstant(TensorValue(i_scalars, i_shape)).GetExpression())
    {

    }

    Tensor::Tensor(std::string_view i_liquid_code, ScalarType i_scalar_type)
    {
        Panic("To do");
    }

    Tensor::Tensor(std::string_view i_liquid_code, Span<const Integer> i_shape,
        ScalarType i_scalar_type)
    {
        Panic("To do");
    }

    ScalarType Tensor::GetScalarType() const
    {
        return m_expression->GetType().GetScalarType();
    }

    bool Always(const Tensor & i_bool_tensor)
    {
        return AlwaysEqual(i_bool_tensor, TensorValue::True());
    }

    bool Never(const Tensor & i_bool_tensor)
    {
        return AlwaysEqual(i_bool_tensor, TensorValue::False());
    }
}
