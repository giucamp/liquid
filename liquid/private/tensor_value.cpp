
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
        size_t TensorValue::ConstantWrapping(const FixedShape & i_shape, Span<const SCALAR_TYPE> i_scalars)
    {
        /* Constant wrapping is the opposite of broadcasting. The actual stored tensor value
           has uppper dimensions stripped out, as long as the respective subtensors are identical. 
           When accessing the tensor the modulo operator is applied to the physical linear index. */

        Integer const scalar_count = static_cast<Integer>(i_scalars.size());
        for (Integer dim = i_shape.GetRank(); dim >= 0; dim--)
        {
            Integer const stride = i_shape.GetStride(dim);

            if(scalar_count > stride && stride > 0)
            {
                bool equals = true;
                for (Integer pos = stride; pos < scalar_count && equals; pos += stride)
                    equals = equals && std::equal(
                        i_scalars.begin(), i_scalars.begin() + stride, i_scalars.begin() + pos);

                if(equals)
                    return static_cast<size_t>(stride);
            }
        }
        return i_scalars.size();
    }

    void TensorValue::DynamicConstantWrapping()
    {
        switch (m_type.GetScalarType())
        {
            case ScalarType::Real:
            {
                auto const scalars = GetAs<Real>();
                size_t const reduced_size = ConstantWrapping<Real>(m_type.GetFixedShape(), scalars);
                if(reduced_size != scalars.size())
                    m_scalars = Span<const Real>(scalars.data(), reduced_size);
                break;
            }

            case ScalarType::Integer:
            {
                auto const scalars = GetAs<Integer>();
                size_t const reduced_size = ConstantWrapping<Integer>(m_type.GetFixedShape(), scalars);
                if (reduced_size != scalars.size())
                    m_scalars = Span<const Integer>(scalars.data(), reduced_size);
                break;
            }

            case ScalarType::Bool:
            {
                auto const scalars = GetAs<Bool>();
                size_t const reduced_size = ConstantWrapping<Bool>(m_type.GetFixedShape(), scalars);
                if (reduced_size != scalars.size())
                    m_scalars = Span<const Bool>(scalars.data(), reduced_size);
                break;
            }

            case ScalarType::Any: Panic("TensorValue - ScalarType::Any canot be used for a value");

            default: Panic("Unrecognized scalar type ", static_cast<int>(m_type.GetScalarType()));
        }
    }

    TensorValue::TensorValue(SharedArray<const Real> && i_reals, const FixedShape& i_shape)
        : m_type(ScalarType::Real, i_shape)
    {
        size_t const reduced_size = ConstantWrapping<Real>(m_type.GetFixedShape(), i_reals);
        if(reduced_size == i_reals.size())
            m_scalars = std::move(i_reals);
        else
            m_scalars = Span<const Real>(i_reals.data(), reduced_size);
    }

    TensorValue::TensorValue(SharedArray<const Integer> && i_integers, const FixedShape & i_shape)
        : m_type(ScalarType::Integer, i_shape)
    {
        size_t const reduced_size = ConstantWrapping<Integer>(m_type.GetFixedShape(), i_integers);
        if (reduced_size == i_integers.size())
            m_scalars = std::move(i_integers);
        else
            m_scalars = Span<const Integer>(i_integers.data(), reduced_size);
    }

    TensorValue::TensorValue(SharedArray<const Bool> && i_bools, const FixedShape& i_shape)
        : m_type(ScalarType::Bool, i_shape)
    {
        size_t const reduced_size = ConstantWrapping<Bool>(m_type.GetFixedShape(), i_bools);
        if (reduced_size == i_bools.size())
            m_scalars = std::move(i_bools);
        else
            m_scalars = Span<const Bool>(i_bools.data(), reduced_size);
    }

    void TensorValue::UnflattenLowerDim(const FixedShape & i_dest_shape)
    {
        const FixedShape & source_shape = m_type.GetFixedShape();

        if(source_shape.GetRank() > 0 && i_dest_shape.GetRank() > 0)
        {
            Integer const source_lower = source_shape.GetDimensions().back();

            Integer dest_lower = i_dest_shape.GetDimensions().back();
            Integer dim_index = i_dest_shape.GetRank() - 2;
            for (; dim_index >= 0 && dest_lower < source_lower; dim_index--)
            {
                dest_lower *= i_dest_shape.GetDimension(dim_index);
                if (dest_lower == source_lower)
                {
                    auto const to_prepend = source_shape.GetDimensions().subspan(1);
                    auto const to_append = i_dest_shape.GetDimensions().subspan(static_cast<size_t>(dim_index));
                    std::vector<Integer> new_shape;
                    new_shape.reserve(to_prepend.size() + to_append.size());
                    new_shape.insert(new_shape.end(), to_prepend.begin(), to_prepend.end());
                    new_shape.insert(new_shape.end(), to_append.begin(), to_append.end());
                    m_type = TensorType(m_type.GetScalarType(), FixedShape(new_shape));
                    break;
                }
            }
        }
    }

    void TensorValue::StripSuperfluousUpperDims(const FixedShape& i_dest_shape)
    {
        const FixedShape & source_shape = m_type.GetFixedShape();
        if(source_shape.GetRank() > i_dest_shape.GetRank())
        {
            const Span<const Integer> source_dims = source_shape.GetDimensions();

            size_t leading_ones = 0;
            while (source_shape.GetRank() - leading_ones > i_dest_shape.GetDimensions().size() 
                    && source_dims[leading_ones] == 1)
            {
                leading_ones++;
            }

            if (leading_ones > 0)
            {
                m_type = TensorType(m_type.GetScalarType(), FixedShape(source_dims.subspan(leading_ones)));
            }
        }
    }

    void TensorValue::SetFromInitializer(const TensorInitializer& i_scalars)
    {
        auto shape_and_type = i_scalars.GetShapeAndType();
        m_type = TensorType(shape_and_type.second, Span<const Integer>(shape_and_type.first));
        const FixedShape& shape = m_type.GetFixedShape();
        switch (m_type.GetScalarType())
        {
        case ScalarType::Real:
        {
            auto scalars = SharedArray<Real>(NumericCast<size_t>(shape.GetLinearSize()));
            for (Indices indices(shape); indices; indices++)
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
    }

    TensorValue::TensorValue(const TensorInitializer & i_initializer)
    {
        SetFromInitializer(i_initializer);

        DynamicConstantWrapping();
    }

    TensorValue::TensorValue(const TensorInitializer & i_initializer, const FixedShape & i_shape)
    {
        SetFromInitializer(i_initializer);

        UnflattenLowerDim(i_shape);

        const FixedShape broadcasted = Broadcast({ m_type.GetFixedShape(), i_shape });
        m_type = TensorType(m_type.GetScalarType(), broadcasted);

        StripSuperfluousUpperDims(i_shape);

        if(i_shape != m_type.GetFixedShape())
        {
            Panic("TensorValue - initializer has shape ", 
                m_type.GetFixedShape(), ", tensor has shape ", i_shape);
        }

        DynamicConstantWrapping();
    }

    template <typename COMMON_TYPE, typename LEFT_TYPE, typename RIGHT_TYPE>
        bool EqualsImpl(const FixedShape & i_shape, const TensorValue & i_left, const TensorValue & i_right)
    {
        for (Indices indices = i_shape; indices; indices++)
            if (static_cast<COMMON_TYPE>(indices.At<LEFT_TYPE>(i_left)) != 
                static_cast<COMMON_TYPE>(indices.At<RIGHT_TYPE>(i_right)) )
                    return false;
        return true;
    }

    bool operator == (const TensorValue & i_first, const TensorValue & i_second)
    {
        if( (i_first.GetScalarType() == ScalarType::Bool) != 
            (i_second.GetScalarType() == ScalarType::Bool) )
                return false;

        auto const shape = TryBroadcast({i_first.GetShape(), i_second.GetShape()});
        if(!shape)
            return false;

        std::pair const types = {i_first.GetScalarType(), i_second.GetScalarType()};
        
        if(types == std::pair{ScalarType::Bool, ScalarType::Bool})
            return EqualsImpl<Bool, Bool, Bool>(*shape, i_first, i_second);

        else if(types == std::pair{ScalarType::Real, ScalarType::Real})
            return EqualsImpl<Real, Real, Real>(*shape, i_first, i_second);

        else if(types == std::pair{ScalarType::Real, ScalarType::Integer})
            return EqualsImpl<Real, Real, Integer>(*shape, i_first, i_second);

        else if(types == std::pair{ScalarType::Integer, ScalarType::Real})
            return EqualsImpl<Real, Integer, Real>(*shape, i_first, i_second);

        else if(types == std::pair{ScalarType::Integer, ScalarType::Integer})
            return EqualsImpl<Real, Integer, Integer>(*shape, i_first, i_second);

        else
            Panic("TensorValue - operator == - Unexpected scalar types: ",
                types.first, " and ", types.second);
    }
}
