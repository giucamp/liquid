
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "tensor_value.h"
#include "indices.h"
#include <algorithm>

namespace liquid
{
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

    void TensorValue::UntypedShapedReduction()
    {
        switch (m_type.GetScalarType())
        {
            case ScalarType::Real:
            {
                auto const scalars = GetAs<Real>();
                size_t const reduced_size = ConstantReduction<Real>(m_type.GetFixedShape(), scalars);
                if(reduced_size != scalars.size())
                    m_scalars = Span<const Real>(scalars.data(), reduced_size);
                break;
            }

            case ScalarType::Integer:
            {
                auto const scalars = GetAs<Integer>();
                size_t const reduced_size = ConstantReduction<Integer>(m_type.GetFixedShape(), scalars);
                if (reduced_size != scalars.size())
                    m_scalars = Span<const Integer>(scalars.data(), reduced_size);
                break;
            }

            case ScalarType::Bool:
            {
                auto const scalars = GetAs<Bool>();
                size_t const reduced_size = ConstantReduction<Bool>(m_type.GetFixedShape(), scalars);
                if (reduced_size != scalars.size())
                    m_scalars = Span<const Bool>(scalars.data(), reduced_size);
                break;
            }

            case ScalarType::Any: Panic("TensorValue - ScalarType::Any canot be used for a value");

            default: Panic("Unrecognized scalar type ", static_cast<int>(m_type.GetScalarType()));
        }
    }

    TensorValue::TensorValue(SharedArray<const Real> && i_reals, const Shape& i_shape)
        : m_type(ScalarType::Real, i_shape)
    {
        size_t const reduced_size = ConstantReduction<Real>(m_type.GetFixedShape(), i_reals);
        if(reduced_size == i_reals.size())
            m_scalars = std::move(i_reals);
        else
            m_scalars = Span<const Real>(i_reals.data(), reduced_size);
    }

    TensorValue::TensorValue(SharedArray<const Integer> && i_integers, const Shape & i_shape)
        : m_type(ScalarType::Integer, i_shape)
    {
        size_t const reduced_size = ConstantReduction<Integer>(m_type.GetFixedShape(), i_integers);
        if (reduced_size == i_integers.size())
            m_scalars = std::move(i_integers);
        else
            m_scalars = Span<const Integer>(i_integers.data(), reduced_size);
    }

    TensorValue::TensorValue(SharedArray<const Bool> && i_bools, const Shape& i_shape)
        : m_type(ScalarType::Bool, i_shape)
    {
        size_t const reduced_size = ConstantReduction<Bool>(m_type.GetFixedShape(), i_bools);
        if (reduced_size == i_bools.size())
            m_scalars = std::move(i_bools);
        else
            m_scalars = Span<const Bool>(i_bools.data(), reduced_size);
    }

    TensorValue::TensorValue(const ScalarsInitializer& i_scalars, const std::optional<Shape> & i_shape)
    {
        auto shape_and_type = i_scalars.GetShapeAndType();
        m_type = TensorType(shape_and_type.second, Span<const Integer>(shape_and_type.first));
        const Shape & shape = m_type.GetFixedShape();

        switch (m_type.GetScalarType())
        {
        case ScalarType::Real:
        {
            auto scalars = SharedArray<Real>(NumericCast<size_t>(shape.GetLinearSize()));
            for(Indices indices(shape); indices; indices++)
                indices[Span<Real>(scalars)] = indices.At<Real>(i_scalars);
            m_scalars = scalars;
            break;
        }
        case ScalarType::Integer:
        {
            auto scalars = SharedArray<Integer>(NumericCast<size_t>(shape.GetLinearSize()));
            for (Indices indices(shape); indices; indices++)
                indices[Span<Integer>(scalars)] = indices.At<Integer>(i_scalars);
            m_scalars = scalars;
            break;
        }
        case ScalarType::Bool:
        {
            auto scalars = SharedArray<Bool>(NumericCast<size_t>(shape.GetLinearSize()));
            for (Indices indices(shape); indices; indices++)
                indices[Span<Bool>(scalars)] = indices.At<Bool>(i_scalars);
            m_scalars = scalars;
            break;
        }

        case ScalarType::Any: 
            Panic("TensorValue - ScalarType::Any canot be used for a value");

        default: 
            Panic("Unrecognized scalar type ", static_cast<int>(m_type.GetScalarType()));
        }

        UntypedShapedReduction();
    }

    template <typename SCALAR_TYPE>
        bool EqualsImpl(const Shape & i_shape, const TensorValue & i_first, const TensorValue & i_second)
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
