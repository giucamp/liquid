
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "expression.h"
#include <unordered_map>

namespace liquid
{
    Expression::Expression(std::string_view i_name,
        std::string_view i_doc,
        const TensorType& i_type,
        const Operator& i_operator,
        const Operator::Overload & i_overload,
        Span<Tensor const> i_operands,
        const std::any& i_attachment)
            : m_name(i_name),
              m_doc(i_doc),
              m_type(i_type),
              m_operator(i_operator),
              m_overload(i_overload),
              m_operands(i_operands.begin(), i_operands.end()),
              m_attachment(i_attachment)
    {
        m_hash = Hash(m_name, m_type, m_operands, m_operator, m_operands);
        if(m_attachment.has_value())
            m_operator.HashAttachment(m_hash, m_attachment);
    }

    bool AlwaysEqual(const Tensor & i_tensor, const TensorValue& i_value)
    {
        return IsConstant(i_tensor) && GetConstantValue(i_tensor) == i_value;
    }

    TensorType DeduceType(Span<const Tensor> i_operands)
    {
        return DeduceType(Transform(i_operands, 
            [](const Tensor & i_tensor){ return i_tensor.GetExpression()->GetType(); }));
    }



    namespace detail
    {
        bool AreIdenticalImplShallow(const Expression & i_left, const Expression & i_right)
        {
            // we first compare hashes to early reject almost all non-identical expressions
            return
                i_left.GetHash() == i_right.GetHash() &&
                &i_left.GetOperator() == &i_right.GetOperator() &&
                i_left.GetName() == i_right.GetName() &&            
                i_left.GetType() == i_right.GetType() &&
                i_left.GetOperands().size() == i_right.GetOperands().size() &&
                i_left.GetOperator().AttachmentsEqual(i_left.GetAttachment(), i_right.GetAttachment());
        }

        using IdenticalExprMap = std::unordered_multimap<Hash::Word, std::pair<
            const Expression *, const Expression *>>;

        /* this function is resursive on the operands
          i_map contains all pairs of identical expressions found so far
          we combine the hashes of left and right by xor, which is commutative
          the combined hash is used as key in the map */
        bool AreIdenticalImpl(const Expression & i_left, const Expression & i_right, 
            IdenticalExprMap & i_map)
        {
            if(&i_left == &i_right)
                return true;

            if(!AreIdenticalImplShallow(i_left, i_right))
                return false;

            auto const combined_hash = 
                i_left.GetHash().GetValue() ^
                i_left.GetHash().GetValue();

            auto const range = i_map.equal_range(combined_hash);
            for(auto it = range.first; it != range.second; it++)
            {
                const Expression * const expr1 = it->second.first;
                const Expression * const expr2 = it->second.second;
                if(expr1 == &i_left && expr2 == &i_right ||
                    expr1 == &i_right && expr2 == &i_left)
                {
                    return true;
                }
            }

            const size_t operand_count = i_left.GetOperands().size();
            for(size_t operand_index = 0; operand_index < operand_count; operand_index++)
            {
                const Expression & left_op = *i_left.GetOperands()[operand_index].GetExpression();
                const Expression & right_op = *i_right.GetOperands()[operand_index].GetExpression();
                if(!AreIdenticalImpl(left_op, right_op, i_map))
                    return false;
            }

            i_map.insert(std::make_pair(combined_hash, std::make_pair(&i_left, &i_right)));

            return true;
        }
    }

    bool AreIdentical(const Expression & i_left, const Expression & i_right)
    {
        using namespace detail;
        IdenticalExprMap map;
        return AreIdenticalImpl(i_left, i_right, map);
    }
}
