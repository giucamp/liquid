
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

    std::vector<Integer> Strides(Span<const Integer> i_dimensions)
    {
        ValidateDimensions(i_dimensions);

        std::vector<Integer> strides(i_dimensions.size() + 1);

        Integer product = 1;
        size_t stride_index = i_dimensions.size();
        for (auto dim_it = i_dimensions.rbegin(); dim_it != i_dimensions.crend(); dim_it++)
        {
            strides[stride_index] = product;
            stride_index--;
            product *= *dim_it;
        }
        strides[0] = product;

        return strides;
    }

    Integer LinearIndex(Span<const Integer> i_indices,
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
}