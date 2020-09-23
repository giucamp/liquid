
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid.h"
#include "expression.h"

namespace liquid
{
    TensorType DeduceTypeFromScalars(const std::any & i_scalars)
    {
        if(std::any_cast<const Real*>(i_scalars))
            return TensorType(ScalarType::Real, Shape({}));
        else if (std::any_cast<const Integer *>(i_scalars))
            return TensorType(ScalarType::Integer, Shape({}));
        else if (std::any_cast<const Bool *>(i_scalars))
            return TensorType(ScalarType::Bool, Shape({}));
        Panic("Unsupported type");
    }

    Tensor::Tensor(CostructConstant, const std::any & i_scalars,
        const std::optional<Span<Integer>>& i_shape)
    {
        if (auto const scalar = std::any_cast<Real>(&i_scalars))
            m_expression = MakeConstant(TensorValue(Shape({}), Span<const Real>{*scalar})).GetExpression();
        else if (auto const scalar = std::any_cast<Integer>(&i_scalars))
            m_expression = MakeConstant(TensorValue(Shape({}), Span<const Integer>{*scalar})).GetExpression();
        else if (auto const scalar = std::any_cast<Bool>(&i_scalars))
            m_expression = MakeConstant(TensorValue(Shape({}), Span<const Bool>{*scalar})).GetExpression();
        else
            Panic("Unsupported type");
    }

    bool Always(const Tensor& i_bool_tensor)
    {
        return AlwaysEqual(i_bool_tensor, constant_values::True);
    }

    bool Never(const Tensor& i_bool_tensor)
    {
        return AlwaysEqual(i_bool_tensor, constant_values::False);
    }
}
