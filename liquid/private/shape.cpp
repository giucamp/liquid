
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "shape.h"

namespace liquid
{
    void ValidateDimensions(Span<const Integer> i_dimensions)
    {
        for(size_t i = 0; i < i_dimensions.size(); i++)
            if(i_dimensions[i] <= 0)
                Panic("ValidateDimensions - bad shape, ", i, "-th dimension is ", i_dimensions[i]);
    }

    void ValidateStrides(Span<const Integer> i_stride)
    {
        for (size_t i = 0; i < i_stride.size(); i++)
            if (i_stride[i] <= 0)
                Panic("ValidateStrides - bad strides, ", i, "-th stride is ", i_stride[i]);
    }

    std::vector<Integer> ComputeStrides(Span<const Integer> i_dimensions)
    {
        std::vector<Integer> strides(i_dimensions.size() + 1);
        ComputeStrides(i_dimensions, strides);
        return strides;
    }

    void ComputeStrides(Span<const Integer> i_dimensions, Span<Integer> o_strides)
    {
        ValidateDimensions(i_dimensions);

        if(o_strides.size() != i_dimensions.size() + 1)
            Panic("ComputeStrides - bad size of o_strides");

        Integer product = 1;
        size_t stride_index = i_dimensions.size();
        for (auto dim_it = i_dimensions.rbegin(); dim_it != i_dimensions.crend(); dim_it++)
        {
            o_strides[stride_index] = product;
            stride_index--;
            product *= *dim_it;
        }
        o_strides[0] = product;
    }

    Integer GetPhysicalLinearIndex(Span<const Integer> i_indices,
        Span<const Integer> i_dimensions,
        Span<const Integer> i_strides)
    {
        ValidateDimensions(i_dimensions);
        ValidateStrides(i_strides);

        if(i_strides.size() != i_dimensions.size() + 1)
            Panic("LinearIndex - The rank is ", i_dimensions.size(), ", the length of strides should be ", i_dimensions.size() + 1);

        if (i_indices.size() < i_dimensions.size())
            Panic("LinearIndex - Too few indices");

        if (i_indices.size() > i_strides.size())
            i_indices = i_indices.subspan(i_indices.size() - i_strides.size());

        Integer linear_index = 0;
        for (size_t i = 0; i < i_indices.size(); i++)
        {
            Integer const index = i_indices[i];
            Integer const stride = i_strides[i];
            Integer const dimension = i_dimensions[i];

            if (index < dimension)
                linear_index += index * stride;
            else if(dimension != 1)
                Panic("LinearIndex - the ", i, "-th index is out of bound");
        }
        return linear_index;
    }

    Shape Broadcast(Span<const Shape> i_shapes)
    {
        Integer rank = 0;
        for(const Shape & shape: i_shapes)
            if(shape.GetRank() > rank)
                rank = shape.GetRank();

        std::vector<Integer> dimensions(static_cast<size_t>(rank), 1);

        for (Integer dim_index = 0; dim_index < rank; dim_index++)
        {
            Integer & this_dim = dimensions[static_cast<size_t>(dim_index)];
            for (size_t shape_index = 0; shape_index < i_shapes.size(); shape_index++)
            {
                Shape const & shape = i_shapes[shape_index];
                Integer const rank_offset = rank - shape.GetRank();
                if(dim_index >= rank_offset)
                {
                    Integer const source_dim = shape.GetDimension(dim_index - rank_offset);
                    if (source_dim != 1)
                    {
                        if(this_dim != 1 && this_dim != source_dim)
                            Panic("Broadcast failure for ", dim_index, "-th dimension");
                        this_dim = source_dim;
                    }
                }
            }
        }

        return { dimensions };
    }

    Shape::Shape(std::initializer_list<Integer> i_initializer_list)
        : m_dimensions(i_initializer_list), m_strides(i_initializer_list.size() + 1)
    {
        ComputeStrides(m_dimensions, m_strides);
    }

    Shape::Shape(Span<const Integer> i_initializer_list)
        : m_dimensions(i_initializer_list), m_strides(i_initializer_list.size() + 1)
    {
        ComputeStrides(m_dimensions, m_strides);
    }

    Integer Shape::GetPhysicalLinearIndex(Span<const Integer> i_indices) const
    {
        return liquid::GetPhysicalLinearIndex(i_indices, m_dimensions, m_strides);
    }
}
