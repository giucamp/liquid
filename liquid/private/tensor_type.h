
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <variant>
#include "private_common.h"
#include "liquid/tensor.h"
#include "fixed_shape.h"
#include "hash.h"

namespace liquid
{
    class TensorType
    {
    public:

        using ShapeVector = std::variant<std::monostate, FixedShape, Tensor>;

        TensorType(ScalarType i_scalar_type = ScalarType::Any, const ShapeVector & i_shape = {});

        ScalarType GetScalarType() const { return m_scalar_type; }

        bool HasShape() const { return !std::holds_alternative<std::monostate>(m_shape); }

        bool HasUndefinedShape() const { return std::holds_alternative<std::monostate>(m_shape); }

        bool HasFixedShape() const { return std::holds_alternative<FixedShape>(m_shape); }

        bool HasVariableShape() const { return std::holds_alternative<Tensor>(m_shape); }

        const ShapeVector & GetShape() const { return m_shape; }
        
        const FixedShape & GetFixedShape() const;

        const Tensor & GetVariableShape() const;

        bool operator == (const TensorType & i_other) const;

        bool operator != (const TensorType & i_other) const { return !operator == (i_other); }

        bool IsSupercaseOf(const TensorType & i_other) const;

        friend std::ostream & operator << (std::ostream & i_ostream, const TensorType & i_tensor_type);

        friend Hash & operator << (Hash & i_dest, const TensorType & i_source);

    private:
        ScalarType m_scalar_type;
        ShapeVector m_shape;
    };

    ScalarType DeduceScalarType(Span<const ScalarType> i_operand_types);

    TensorType DeduceType(Span<const TensorType> i_operand_types);
}
