
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <variant>
#include "liquid/liquid_common.h"
#include "liquid/span.h"
#include "shared_array.h"
#include "tensor_type.h"

namespace liquid
{
    class TensorValue
    {
    public:

        TensorValue(const Shape & i_shape, SharedArray<const Real> && i_reals);

        TensorValue(const Shape & i_shape, SharedArray<const Integer> && i_integers);

        TensorValue(const Shape & i_shape, SharedArray<const Bool> && i_bools);

        const TensorType & GetType() const { return m_type; }

        const Shape & GetShape() const { return m_type.GetFixedShape(); }

        ScalarType GetScalarType() const { return m_type.GetScalarType(); }

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

        friend bool operator == (const TensorValue & i_first, const TensorValue& i_second);

        friend bool operator != (const TensorValue& i_first, const TensorValue& i_second)
        {
            return !(i_first == i_second);
        }

    private:
        template <typename SCALAR_TYPE>
            static size_t ConstantReduction(const Shape & i_shape, Span<const SCALAR_TYPE> i_scalars);

    private:
        TensorType m_type;
        std::variant<
            SharedArray<const Real>,
            SharedArray<const Integer>,
            SharedArray<const Bool>
        > m_scalars;
    };

    namespace constant_values
    {
        extern TensorValue RealZero;
        extern TensorValue IntegerZero;
        extern TensorValue True;
        extern TensorValue False;
    }
}
