
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <variant>
#include "liquid.h"
#include "span.h"
#include "shared_array.h"
#include "shape.h"

namespace liquid
{
    using Scalars = std::variant<
        SharedArray<const Real>,
        SharedArray<const Integer>,
        SharedArray<const Bool>
    >;

    class TensorValue
    {
    public:

        TensorValue(const Shape & i_shape, Span<const Real> i_reals)
            : m_shape(i_shape), m_scalars(i_reals)
        {
        }

        TensorValue(const Shape& i_shape, SharedArray<const Real> && i_reals)
            : m_shape(i_shape), m_scalars(std::move(i_reals))
        {
        }

        TensorValue(const Shape& i_shape, Span<const Integer> i_integers)
            : m_shape(i_shape), m_scalars(i_integers)
        {
        }

        TensorValue(const Shape& i_shape, SharedArray<const Integer> && i_integers)
            : m_shape(i_shape), m_scalars(std::move(i_integers))
        {
        }

        TensorValue(const Shape& i_shape, Span<const Bool> i_bools)
            : m_shape(i_shape), m_scalars(i_bools)
        {
        }

        TensorValue(const Shape& i_shape, SharedArray<const Bool> && i_bools)
            : m_shape(i_shape), m_scalars(std::move(i_bools))
        {
        }

        const Shape & GetShape() const { return m_shape; }

        template <typename SCALAR_TYPE> bool Is() const
        {
            return std::holds_alternative<const SCALAR_TYPE>>(m_scalars);
        }

        template <typename SCALAR_TYPE>
            SharedArray<const SCALAR_TYPE> const & GetAs() const
        {
            return std::get<SharedArray<const SCALAR_TYPE>>(m_scalars);
        }

        template <typename SCALAR_TYPE>
            SharedArray<const SCALAR_TYPE> const & TryGetAs() const
        {
            if(Is<SCALAR_TYPE>())
                return GetAs<SCALAR_TYPE>();
            else
                return {};
        }

    private:
        Shape m_shape;
        std::variant<
            SharedArray<const Real>,
            SharedArray<const Integer>,
            SharedArray<const Bool>
        > m_scalars;
    };
}
