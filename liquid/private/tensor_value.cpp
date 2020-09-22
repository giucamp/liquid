
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "tensor_value.h"
#include "indices.h"
#include <algorithm>

namespace liquid
{
    namespace constant_values
    {
        extern TensorValue RealZero(Shape({}), Span<Real const>{0});
        extern TensorValue IntegerZero(Shape({}), Span<Integer const>{0});
        extern TensorValue True(Shape({}), Span<Bool const>{true});
        extern TensorValue False(Shape({}), Span<Bool const>{false});
    }

    template <typename SCALAR_TYPE>
        size_t TensorValue::ConstantReduction(const Shape & i_shape, Span<const SCALAR_TYPE> i_scalars)
    {
        Integer const scalar_count = static_cast<Integer>(i_scalars.size());
        for (Integer dim = i_shape.GetRank(); dim >= 0; dim--)
        {
            Integer const stride = i_shape.GetStride(dim);

            bool equals = true;
            for (Integer pos = stride; pos <= scalar_count && equals; pos += stride)
                equals = std::equal(i_scalars.begin(), i_scalars.begin() + stride, i_scalars.begin() + pos);

            if(equals)
                return static_cast<size_t>(stride);
        }
        return i_scalars.size();
    }

    TensorValue::TensorValue(const Shape& i_shape, SharedArray<const Real>&& i_reals)
        : m_type(ScalarType::Real, i_shape)
    {
        size_t const reduced_size = ConstantReduction<Real>(m_type.GetFixedShape(), GetAs<Real>());
        if(reduced_size == i_reals.size())
            m_scalars = std::move(i_reals);
        else
            m_scalars = Span<const Real>(i_reals);
    }

    TensorValue::TensorValue(const Shape& i_shape, SharedArray<const Integer>&& i_integers)
        : m_type(ScalarType::Integer, i_shape)
    {
        size_t const reduced_size = ConstantReduction<Integer>(m_type.GetFixedShape(), GetAs<Integer>());
        if (reduced_size == i_integers.size())
            m_scalars = std::move(i_integers);
        else
            m_scalars = Span<const Integer>(i_integers);
    }

    TensorValue::TensorValue(const Shape& i_shape, SharedArray<const Bool>&& i_bools)
        : m_type(ScalarType::Bool, i_shape)
    {
        size_t const reduced_size = ConstantReduction<Bool>(m_type.GetFixedShape(), GetAs<Bool>());
        if (reduced_size == i_bools.size())
            m_scalars = std::move(i_bools);
        else
            m_scalars = Span<const Bool>(i_bools);
    }

    template <typename SCALAR_TYPE>
        bool EqualsImpl(const Shape & i_shape, const TensorValue& i_first, const TensorValue& i_second)
    {
        for (Indices indices = i_shape; indices; indices++)
            if (indices.At<SCALAR_TYPE>(i_first) != indices.At<SCALAR_TYPE>(i_second))
                return false;
        return true;
    }

    bool operator == (const TensorValue & i_first, const TensorValue & i_second)
    {
        if(i_first.GetScalarType() != i_second.GetScalarType())
            return false;

        auto const shape = TryBroadcast({i_first.GetShape(), i_second.GetShape()});
        if(!shape)
            return false;

        switch (i_first.GetScalarType())
        {
        case ScalarType::Real:
            return EqualsImpl<Real>(*shape, i_first, i_second);

        case ScalarType::Integer:
            return EqualsImpl<Integer>(*shape, i_first, i_second);

        case ScalarType::Bool:
            return EqualsImpl<Bool>(*shape, i_first, i_second);

        default:
            Panic("TensorValue - operator == - Unexpected scalar type");
        }
    }
}
