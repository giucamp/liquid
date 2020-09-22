
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

        const std::variant<std::monostate, Shape, Tensor> & GetShape() const { return m_shape; }
        const Shape & GetConstantShape() const { return std::get<Shape>(m_shape); }
        const Tensor& GetVariableShape() const { return std::get<Tensor>(m_shape); }

    private:
        ScalarType m_scalar_type;
        std::variant<std::monostate, Shape, Tensor> m_shape;
    };
}
