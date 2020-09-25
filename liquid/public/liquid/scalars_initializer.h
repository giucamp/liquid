
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <variant>
#include <type_traits>
#include <utility>
#include "liquid/liquid_common.h"
#include "liquid/span.h"

namespace liquid
{
    class ScalarsInitializer
    {
    public:

        template <typename ELEMENT_TYPE>
            ScalarsInitializer(std::initializer_list<ELEMENT_TYPE> i_list)
        {
            if constexpr (std::is_floating_point_v<ELEMENT_TYPE>)
            {
                std::vector<Real> scalars(i_list.size());
                for(size_t i = 0; i < i_list.size(); i++)
                    scalars[i] = static_cast<Real>(i_list.begin()[i]);
                m_elements = std::move(scalars);
            }
            else if constexpr (std::is_same_v<ELEMENT_TYPE, Bool>)
            {
                std::vector<Bool> scalars(i_list.size());
                for (size_t i = 0; i < i_list.size(); i++)
                    scalars[i] = i_list.begin()[i];
                m_elements = std::move(scalars);
            }
            else if constexpr (std::is_integral_v<ELEMENT_TYPE>)
            {
                std::vector<Integer> scalars(i_list.size());
                for (size_t i = 0; i < i_list.size(); i++)
                    scalars[i] = NumericCast<Integer>(i_list.begin()[i]);
                m_elements = std::move(scalars);
            }
            else
            {
                std::vector<ScalarsInitializer> elements(i_list.size());
                for (size_t i = 0; i < i_list.size(); i++)
                    elements[i] = ScalarsInitializer(i_list.begin()[i]);
                m_elements = std::move(elements);
            }
        }

        ScalarsInitializer(ScalarsInitializer &&) = default;
        ScalarsInitializer & operator = (ScalarsInitializer&&) = default;

        int64_t GetRank() const;

        std::pair<std::vector<Integer>, ScalarType> GetShapeAndType() const;

        Integer GetFirstDimension() const;

        template <typename ELEMENT_TYPE>
            bool Is() const
                { return std::holds_alternative<std::vector<ELEMENT_TYPE>>(m_elements); }

        template <typename ELEMENT_TYPE>
            const ELEMENT_TYPE & At(Span<const Integer> i_indices) const
        {
            if(i_indices.size() != static_cast<size_t>(GetRank()))
                Panic("ScalarsInitializer - wrong number of indices");
            
            if (!Is<ELEMENT_TYPE>())
                Panic("ScalarsInitializer - wrong scalar type");

            const ScalarsInitializer * curr = this;
            while (i_indices.size() > 1)
            {
                auto const & elements = std::get<std::vector<ScalarsInitializer>>(m_elements);
                curr = &elements.at(NumericCast<size_t>(i_indices[0]));
                i_indices = i_indices.subspan(1);
            }
            auto const & elements = std::get<std::vector<ELEMENT_TYPE>>(curr->m_elements);
            return elements.at(NumericCast<size_t>(i_indices[0]));
        }

    private:

        void GetShapeAndTypeImpl(size_t i_curr_dim, Span<int64_t> o_shape, ScalarType & o_type) const;

    private:
        std::variant<
            std::vector<Real>,
            std::vector<Integer>,
            std::vector<Bool>,
            std::vector<ScalarsInitializer>
        > m_elements;
    };
}
