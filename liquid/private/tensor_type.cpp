
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "tensor_type.h"

namespace liquid
{
    ScalarType DeduceScalarType(Span<const ScalarType> i_operand_types)
    {
        ScalarType result_type = ScalarType::Any;
        for (ScalarType operand_type : i_operand_types)
        {
            if(operand_type == ScalarType::Any)
                return ScalarType::Any;
            else if(result_type == ScalarType::Any)
                result_type = operand_type;
            else if (result_type != operand_type)
            {
                if(operand_type == ScalarType::Real 
                    && result_type == ScalarType::Integer)
                {
                    // integer to real promotion
                    result_type = ScalarType::Real;
                }
                else
                {
                    return ScalarType::Any;
                }
            }
        }
        return result_type;
    }

    TensorType DeduceType(Span<const TensorType> i_operand_types)
    {
        std::vector<ScalarType> scalar_types;
        std::vector<Shape> shapes;

        scalar_types.reserve(i_operand_types.size());
        shapes.reserve(i_operand_types.size());

        for (auto const & operand_type : i_operand_types)
        {
            scalar_types.push_back(operand_type.GetScalarType());
            if(operand_type.HasFixedShape())
                shapes.push_back(operand_type.GetFixedShape());
        }

        ScalarType const scalar_type = DeduceScalarType(scalar_types);
        if(shapes.size() == i_operand_types.size())
            return { scalar_type, Broadcast(shapes) };
        else
            return { scalar_type };
    }

    bool TensorType::operator == (const TensorType& i_other) const
    {
        if (m_scalar_type != i_other.m_scalar_type)
            return false;

        struct Visistor
        {
            const TensorType & m_this;
            const TensorType & m_other;
            
            bool operator () (std::monostate) const
            {
                return !m_other.HasShape();
            }

            bool operator () (const Shape & i_shape) const
            {
                if (m_other.HasFixedShape())
                    return m_this.GetFixedShape() == m_other.GetFixedShape();
                return false;
            }

            bool operator () (const Tensor & i_shape) const
            {
                return Always(m_this.GetVariableShape() == m_other.GetVariableShape());
            }
        };

        return std::visit(Visistor{*this, i_other}, m_shape);
    }

    bool TensorType::IsSupercaseOf(const TensorType & i_other) const
    {
        if (m_scalar_type != ScalarType::Any && m_scalar_type != i_other.m_scalar_type)
            return false;

        if (HasFixedShape() && i_other.HasFixedShape())
            return GetFixedShape() == i_other.GetFixedShape();
        return true;
    }
}
