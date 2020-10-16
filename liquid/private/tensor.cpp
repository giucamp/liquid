
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/tensor.h"
#include "expression.h"
#include "miu6/parser.h"

namespace liquid
{
    Tensor::Tensor(std::initializer_list<Tensor> i_vector)
        : Tensor(Stack(i_vector))
    {        
    }

    Tensor::Tensor(std::initializer_list<Tensor> i_vector, Span<const Integer> i_shape)
        : Tensor(Stack(i_vector))
    {
        if (IsConstant(*this))
        {
            const TensorValue & value = GetConstantValue(*this);
            switch (GetScalarType())
            {
                case ScalarType::Real:
                {
                    TensorValue value(Span(value.GetAs<Real>()), FixedShape(i_shape));
                    m_expression = MakeConstant(value).GetExpression();
                    break;
                }

                case ScalarType::Integer:
                {
                    TensorValue value(Span(value.GetAs<Integer>()), FixedShape(i_shape));
                    m_expression = MakeConstant(value).GetExpression();
                    break;
                }

                case ScalarType::Bool:
                {
                    TensorValue value(Span(value.GetAs<Bool>()), FixedShape(i_shape));
                    m_expression = MakeConstant(value).GetExpression();
                    break;
                }
            }
        }
    }

    Tensor::Tensor(ScalarConst, Real i_scalar)
        : m_expression(MakeConstant(TensorValue(SharedArray<const Real>({i_scalar}), FixedShape{})).GetExpression())
    {
    }

    Tensor::Tensor(ScalarConst, Integer i_scalar)
        : m_expression(MakeConstant(TensorValue(SharedArray<const Integer>({i_scalar}), FixedShape{})).GetExpression())
    {
    }

    Tensor::Tensor(ScalarConst, Bool i_scalar)
        : m_expression(MakeConstant(TensorValue(SharedArray<const Bool>({i_scalar}), FixedShape{})).GetExpression())
    {
    }

    Tensor::Tensor(ScalarConst, Real i_scalar, Span<const Integer> i_shape)
        : m_expression(MakeConstant(TensorValue(SharedArray<const Real>({i_scalar}), i_shape)).GetExpression())
    {
    }

    Tensor::Tensor(ScalarConst, Integer i_scalar, Span<const Integer> i_shape)
        : m_expression(MakeConstant(TensorValue(SharedArray<const Integer>({i_scalar}), i_shape)).GetExpression())
    {
    }

    Tensor::Tensor(ScalarConst, Bool i_scalar, Span<const Integer> i_shape)
        : m_expression(MakeConstant(TensorValue(SharedArray<const Bool>({i_scalar}), i_shape)).GetExpression())
    {
    }

    Tensor::Tensor(std::string_view i_miu6_code, ScalarType i_scalar_type)
        : Tensor(miu6::ParseExpression(i_miu6_code))
    {
        
    }

    Tensor::Tensor(std::string_view i_miu6_code, Span<const Integer> i_shape,
        ScalarType i_scalar_type)
        : Tensor(miu6::ParseExpression(i_miu6_code))
    {
        Panic("to do");
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

    std::ostream & operator << (std::ostream & i_dest, const Tensor& i_tensor)
    {
        i_dest << "to_do";
        return i_dest;
    }
}
