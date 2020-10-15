
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <variant>
#include <any>
#include "private_common.h"
#include "liquid/span.h"
#include "shared_array.h"
#include "tensor_type.h"
#include "operator.h"
#include "hash.h"

namespace liquid
{
    class Expression
    {
    public:

        Expression(std::string_view i_name, 
            std::string_view i_doc,
            const TensorType & i_type,
            const Operator & i_operator,
            const Operator::Overload & i_overload,
            Span<Tensor const> i_operands,
            const std::any & i_attachment);

        const Operator & GetOperator() const { return m_operator; }
        const Operator::Overload & GetOverload() const { return m_overload; }
        const std::string & GetName() const { return m_name; }
        const std::string & GetDoc() const { return m_doc; }
        const TensorType & GetType() const { return m_type; }
        const std::vector<Tensor> & GetOperands() const { return m_operands; }
        const std::any & GetAttachment() const { return m_attachment; }
        const Hash & GetHash() const { return m_hash; }

        bool OperatorIs(const Operator & i_op) const { return &m_operator == &i_op; }

    private:
        std::string m_name;
        std::string m_doc;
        TensorType m_type;
        const Operator & m_operator;
        const Operator::Overload & m_overload;
        std::vector<Tensor> m_operands;
        std::any m_attachment;
        Hash m_hash;
    };

    Tensor MakeConstant(const TensorValue & i_value);

    const TensorValue & GetConstantValue(const Tensor & i_tensor);

    Tensor MakeVariable(const TensorType & i_type, std::string_view i_name);

    bool IsVariable(const Tensor & i_tensor);

    bool AlwaysEqual(const Tensor & i_tensor, const TensorValue & i_value);

    bool AreIdentical(const Expression & i_left, const Expression & i_right);

    TensorType DeduceType(Span<const Tensor> i_operands);

    Tensor Is(const Tensor & i_tensor, const TensorType & i_type);

    inline Hash & operator << (Hash & i_dest, const Expression & i_source)
    {
        i_dest << i_source.GetHash();
        return i_dest;
    }

    inline Hash & operator << (Hash & i_dest, const Tensor & i_source)
    {
        i_dest << i_source.GetExpression()->GetHash();
        return i_dest;
    }
}
