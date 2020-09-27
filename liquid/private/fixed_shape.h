
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "liquid/liquid_common.h"
#include "liquid/span.h"
#include "shared_array.h"

namespace liquid
{
    class FixedShape
    {
    public:

        FixedShape(std::initializer_list<Integer> i_initializer_list);

        FixedShape(Span<const Integer> i_initializer_list);

        Integer GetRank() const { return static_cast<Integer>(m_dimensions.size()); }

        Integer GetLinearSize() const { return m_strides[0]; }

        Integer GetPhysicalLinearIndex(Span<const Integer> i_indices) const;

        Integer GetDimension(Integer i_index) const { return m_dimensions.at(NumericCast<size_t>(i_index)); }

        Integer GetDimensionBackward(Integer i_backward_index) const 
            { return m_dimensions.at(NumericCast<size_t>(GetRank() - 1 - i_backward_index)); }

        Integer GetStride(Integer i_index) const { return m_strides.at(static_cast<size_t>(i_index)); }

        Span<const Integer> GetDimensions() const { return m_dimensions; }

        Span<const Integer> GetStrides() const { return m_strides; }

        bool operator == (const FixedShape & i_other) const
        {
            return m_dimensions == i_other.m_dimensions;
        }

        bool operator != (const FixedShape & i_other) const
        {
            return m_dimensions != i_other.m_dimensions;
        }

        static const FixedShape & Scalar()
        {
            static const FixedShape scalar({});
            return scalar;
        }

    private:
        SharedArray<Integer> m_dimensions;
        SharedArray<Integer> m_strides;
    };

    std::ostream & operator << (std::ostream & i_ostream, const FixedShape & i_shape);

    /* Strides[i] = Product of Dim[j], for i <= j < rank
       Strides[0] = product of all dimensions
       Strides[rank] = 1
       The length of strides is rank + 1 */
    std::vector<Integer> ComputeStrides(Span<const Integer> i_dimensions);

    void ComputeStrides(Span<const Integer> i_dimensions, Span<Integer> o_strides);

    /* Linearize the indices in i_indices.
        The length of i_strides must be equal to the length of i_dimensions + 1
        The length of i_indices can be greater than the length of i_dimensions, in
        which case broadcasting is applied. If the length of i_indices is less than the
        length of i_dimensions, an error is raised. */
    Integer GetPhysicalLinearIndex(Span<const Integer> i_indices,
        Span<const Integer> i_dimensions,
        Span<const Integer> i_strides);

    std::optional<FixedShape> TryBroadcast(Span<const FixedShape> i_shapes);

    FixedShape Broadcast(Span<const FixedShape> i_shapes);

} // namespace liquid
