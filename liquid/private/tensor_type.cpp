
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "tensor_type.h"
#include "expression.h"

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
        std::vector<FixedShape> shapes;

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

    TensorType::TensorType(ScalarType i_scalar_type, const std::variant<std::monostate, FixedShape, Tensor> & i_shape)
        : m_scalar_type(i_scalar_type), m_shape(i_shape)
    {
        if (HasVariableShape() && IsConstant(GetVariableShape()))
        {
            const Tensor & tensor = GetVariableShape();

            const TensorType & tensor_type = tensor.GetExpression()->GetType();
            if(tensor_type.GetFixedShape().GetRank() != 1)
                Panic("The shape of a tensor must be a vector");

            // the shape is not really variable
            auto dimensions = ConstantToVector<Integer>(tensor);
            m_shape = FixedShape(dimensions);
        }
    }

    const FixedShape & TensorType::GetFixedShape() const
    { 
        if(!HasFixedShape())
            Panic("TensorType::GetFixedShape - ", *this, ": not a fixed shape");
        return std::get<FixedShape>(m_shape);
    }

    const Tensor & TensorType::GetVariableShape() const
    {
        if(!HasVariableShape())
            Panic("TensorType::GetFixedShape - ", *this, ": not a variable shape");
        return std::get<Tensor>(m_shape);
    }

    bool TensorType::operator == (const TensorType& i_other) const
    {
        if (m_scalar_type != i_other.m_scalar_type)
            return false;

        struct Visitor
        {
            const TensorType & m_this;
            const TensorType & m_other;
            
            bool operator () (std::monostate) const
            {
                return !m_other.HasShape();
            }

            bool operator () (const FixedShape & i_shape) const
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

        return std::visit(Visitor{*this, i_other}, m_shape);
    }

    bool TensorType::IsSupercaseOf(const TensorType & i_other) const
    {
        if (m_scalar_type != ScalarType::Any && m_scalar_type != i_other.m_scalar_type)
            return false;

        if (HasFixedShape() && i_other.HasFixedShape())
            return GetFixedShape() == i_other.GetFixedShape();
        return true;
    }

    std::ostream& operator << (std::ostream & i_ostream, const TensorType & i_tensor_type)
    {
        if(i_tensor_type.m_scalar_type != ScalarType::Any)
            i_ostream << i_tensor_type.m_scalar_type;

        if(i_tensor_type.HasFixedShape())
            i_ostream << i_tensor_type.GetFixedShape();

        return i_ostream;
    }

    Hash & operator << (Hash & i_dest, const TensorType & i_source)
    {
        i_dest << i_source.m_scalar_type;

        struct Visitor
        {
            Hash & m_dest;
            const TensorType & m_source;

            void operator () (std::monostate) const
            {
                m_dest << 42;
            }

            void operator () (const FixedShape & i_shape) const
            {
                m_dest << i_shape;
            }

            void operator () (const Tensor & i_shape) const
            {
                m_dest << *i_shape.GetExpression();
            }
        };

        std::visit(Visitor{i_dest, i_source}, i_source.m_shape);

        return i_dest;
    }
}
