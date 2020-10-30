
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "liquid/tensor.h"
#include "operator.h"
#include <unordered_map>

namespace liquid
{
    namespace detail
    {
        using ReplacementMap = std::unordered_map<
            std::shared_ptr<const Expression>,
            std::shared_ptr<const Expression> >;

        template <typename PREDICATE>
            Tensor SubstituteByPredicateImpl(const Tensor & i_where, 
                const PREDICATE & i_predicate,
                ReplacementMap & i_replacement_map)
        {
            auto const it = i_replacement_map.find(i_where.GetExpression());
            if(it != i_replacement_map.end())
                return Tensor(it->second);

            Tensor replacement = i_predicate(i_where);

            std::vector<Tensor> new_operands;
            new_operands.reserve(replacement.GetExpression()->GetOperands().size());
        
            bool some_operand_replaced = false;
            for(const Tensor & operand : replacement.GetExpression()->GetOperands())
            {
                new_operands.push_back(
                    SubstituteByPredicateImpl(operand, i_predicate, i_replacement_map));

                some_operand_replaced = some_operand_replaced || 
                    new_operands.back().GetExpression() != operand.GetExpression();
            }
            if(some_operand_replaced)
                replacement = replacement.GetExpression()->GetOperator().Invoke(
                    replacement.GetExpression()->GetName(), 
                    replacement.GetExpression()->GetDoc(), 
                    new_operands, 
                    replacement.GetExpression()->GetAttachment());

            if(!i_replacement_map.insert(std::make_pair(
                i_where.GetExpression(), replacement.GetExpression())).second)
            {
                Panic("SubstituteByPredicate - Cyclic expression");
            }

            return replacement;
        }

    } // namespace detail

    template <typename PREDICATE>
        [[nodiscard]] Tensor SubstituteByPredicate(
            const Tensor & i_where,
            const PREDICATE & i_predicate)
    {
        detail::ReplacementMap replacement_map;
        return detail::SubstituteByPredicateImpl(i_where, i_predicate, replacement_map);
    }

    template <typename PREDICATE>
        [[nodiscard]] std::vector<Tensor> SubstituteByPredicate(
            Span<const Tensor> i_where,
            const PREDICATE & i_predicate)
    {
        detail::ReplacementMap replacement_map;

        std::vector<Tensor> result;
        result.reserve(i_where.size());
        for(auto const & where : i_where)
            result.push_back(detail::SubstituteByPredicateImpl(i_where, replacement_map));

        return result;
    }
}
