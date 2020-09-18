#pragma once

#include <variant>
#include "liquid.h"
#include "span.h"
#include "shared_array.h"
#include "shape.h"

namespace liquid
{
    class TensorValue
    {
    public:

        TensorValue(const Shape & i_shape, Span<const Real> i_reals)
            : m_shape(i_shape), m_scalars(i_reals)
        {
        }

        TensorValue(const Shape& i_shape, Span<const Integer> i_integers)
            : m_shape(i_shape), m_scalars(i_integers)
        {
        }

        TensorValue(const Shape& i_shape, Span<const Bool> i_bools)
            : m_shape(i_shape), m_scalars(i_bools)
        {
        }

    private:
        Shape m_shape;
        std::variant<
            SharedArray<Real>,
            SharedArray<Integer>,
            SharedArray<Bool>
        > m_scalars;
    };
}