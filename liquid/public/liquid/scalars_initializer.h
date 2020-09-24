
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <variant>
#include <type_traits>
#include <array>
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

        constexpr int64_t GetRank() const
        {
            if(std::holds_alternative<std::vector<ScalarsInitializer>>(m_elements))
                return std::get<std::vector<ScalarsInitializer>>(m_elements).at(0).GetRank() + 1;
            else
                return 1;
        }

        std::vector<Integer> GetShape() const
        {
            std::vector<Integer> result(NumericCast<size_t>(GetRank()));
            GetShapeImpl(result);
            return result;
        }

    private:
        void GetShapeImpl(Span<int64_t> o_shape) const
        {
            size_t const size = std::visit([](auto const & i_elements){ return i_elements.size(); }, m_elements);
            o_shape[0] = NumericCast<Integer>(size);
            if (std::holds_alternative<std::vector<ScalarsInitializer>>(m_elements))
            {
                auto const & elements = std::get<std::vector<ScalarsInitializer>>(m_elements);
                elements.at(0).GetShapeImpl(o_shape.subspan(0));
            }
        }

    private:
        std::variant<
            std::vector<Real>,
            std::vector<Integer>,
            std::vector<Bool>,
            std::vector<ScalarsInitializer>
        > m_elements;
    };
}
