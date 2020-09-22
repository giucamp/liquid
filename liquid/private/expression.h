
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <variant>
#include <any>
#include "liquid.h"
#include "span.h"
#include "shared_array.h"
#include "tensor_type.h"
#include "operator.h"

namespace liquid
{
    class Expression
    {
    public:

        const Operator & GetOperator() const { return m_operator; }
        const std::string & GetName() const { return m_name; }
        const std::string & GetDoc() const { return m_doc; }
        const TensorType & GetType() const { return m_type; }
        const std::vector<Tensor> & GetAttributes() const { return m_attributes; }
        const std::vector<Tensor> & GetOperands() const { return m_operands; }
        const std::any & GetAttachment() const { return m_attachment; }

        bool OperatorIs(const Operator& i_op) const { return &m_operator == &i_op; }

    private:
        std::string m_name;
        std::string m_doc;
        TensorType m_type;
        const Operator & m_operator;
        std::vector<Tensor> m_attributes;
        std::vector<Tensor> m_operands;
        std::any m_attachment;
    };

    Tensor MakeConstant(const TensorValue & i_value);

    bool IsConstant(const Tensor & i_tensor);

    const TensorValue GetConstantValue(const Tensor& i_tensor);

    bool AlwaysEqual(const Tensor& i_tensor, const TensorValue & i_value);
}
