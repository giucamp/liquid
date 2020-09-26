
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/tensor.h"
#include "expression.h"

namespace liquid
{
    Tensor::Tensor(const TensorInitializer& i_scalars)
        : m_expression(MakeConstant(TensorValue(i_scalars)).GetExpression())
    {

    }

    Tensor::Tensor(const TensorInitializer& i_scalars, Span<const Integer> i_shape)
        : m_expression(MakeConstant(TensorValue(i_scalars, i_shape)).GetExpression())
    {

    }

    ScalarType Tensor::GetScalarType() const
    {
        return m_expression->GetType().GetScalarType();
    }

    bool Tensor::HasFixedShape() const
    {
        return m_expression->GetType().HasFixedShape();
    }

    Span<const Integer> Tensor::GetFixedShape() const
    {
        return m_expression->GetType().GetFixedShape().GetDimensions();
    }

    Integer Tensor::GetRank() const
    {
        return NumericCast<Integer>(GetFixedShape().size());
    }

    bool Always(const Tensor& i_bool_tensor)
    {
        static const TensorValue True(true);
        return AlwaysEqual(i_bool_tensor, True);
    }

    bool Never(const Tensor& i_bool_tensor)
    {
        static const TensorValue False(false);
        return AlwaysEqual(i_bool_tensor, False);
    }
}
