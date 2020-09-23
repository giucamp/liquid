
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <variant>
#include "liquid.h"
#include "shape.h"

namespace liquid
{
    class TensorType
    {
    public:

        TensorType(ScalarType i_scalar_type, std::variant<std::monostate, Shape, Tensor> i_shape = {})
            : m_scalar_type(i_scalar_type), m_shape(std::move(i_shape))
                { }

        ScalarType GetScalarType() const { return m_scalar_type; }

        bool HasShape() const { return !std::holds_alternative<std::monostate>(m_shape); }

        bool HasFixedShape() const { return std::holds_alternative<Shape>(m_shape); }

        bool HasVariableShape() const { return std::holds_alternative<Tensor>(m_shape); }

        const std::variant<std::monostate, Shape, Tensor> & GetShape() const { return m_shape; }
        const Shape & GetFixedShape() const { return std::get<Shape>(m_shape); }
        const Tensor& GetVariableShape() const { return std::get<Tensor>(m_shape); }

        bool operator == (const TensorType & i_other) const;

        bool operator != (const TensorType & i_other) const { return !operator == (i_other); }

        bool IsSupercaseOf(const TensorType & i_other) const;

    private:
        ScalarType m_scalar_type;
        std::variant<std::monostate, Shape, Tensor> m_shape;
    };

    ScalarType DeduceScalarType(Span<const ScalarType> i_operand_types);

    TensorType DeduceType(Span<const TensorType> i_operand_types);
}
