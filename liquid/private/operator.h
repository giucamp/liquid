
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "tensor_value.h"
#include "tensor_type.h"
#include "hash.h"
#include <optional>
#include <any>
#include <variant>
#include <string_view>

namespace liquid
{
    class Operator
    {
    public:

        Operator(std::string_view i_name);

        const std::string & GetName() const      { return m_name; }

        Tensor Invoke(Span<const Tensor> i_operands, const std::any & i_attachment = {} ) const;

        Tensor Invoke(std::string_view i_name, std::string_view i_doc,
            Span<const Tensor> i_operands, const std::any & i_attachment = {}) const;

                // doc

        Operator & SetDoc(std::string_view i_description, std::string_view i_return_type);

                // flags
 
        enum class Flags
        {
            None = 0,
            Commutative = 1 << 0,
            Associative = 1 << 1
        };

        friend Flags operator | (Flags i_first, Flags i_second)
            { return static_cast<Flags>(static_cast<int>(i_first) | static_cast<int>(i_second)); }


        bool Has(Flags i_flags) const { return liquid::HasFlags(m_flags, i_flags); }

        /* An operator is regular n-ary iff it is associative, commutative, and has
            a single variadic operand in all overloads. 'add', 'mul', 'and' and 'or'
            are regular n-ary, whilst 'if' and 'not' are not. 
        
            Regular n-ary operators have an identity element, which is computed by
            invoking the operator with no operands. For example: add() == 0,
            mul() == 1, and() == true, or() == false.

            Some common canonicalizaions are applied to regular n-ary operators:
            - directly nested expressions with the same operator are flattened:
                op(x... op(y...) z...) -> op(x... y... z...)
            - partial constant propagation: all constant operands are removed, 
                merged together by applying the operator on them, and then if the
                merged value is not the identity element, it gets appendend as operand.
            - operands are sorted using a deterministic but unspecified criterium
        */
        bool IsRegularNAry() const;

        Operator & AddFlags(Flags i_flags);


                // type deduction

        using DeduceTypeFunction = TensorType(*)(const std::any & i_attachment, 
            Span<const Tensor> i_operands);

        Operator & SetDeduceType(DeduceTypeFunction i_func);
        

                // evaluation

        using EvaluateWithAttachmentFunction = TensorValue(*)(const std::any & i_attachment,
            const TensorType & i_result_type, Span<const TensorValue> i_operands);

        using EvaluateFromVariablesFunction = TensorValue(*)(const std::any & i_attachment,
            const TensorType & i_result_type, Span<const Tensor> i_operands);

        using EvaluateFunction = TensorValue(*)(const TensorType & i_result_type,
            Span<const TensorValue> i_operands);

        using EvaluateSingleArgument = TensorValue(*)(const TensorType & i_result_type,
            const TensorValue & i_operand);

        using VarEvaluateFunction = std::variant<EvaluateFunction, 
            EvaluateSingleArgument, EvaluateWithAttachmentFunction,
            EvaluateFromVariablesFunction>;

        struct Parameter
        {
            TensorType m_type;
            std::string m_name;
        };

        struct Overload
        {
            VarEvaluateFunction m_evaluate = {};
            std::vector<Parameter> m_parameters;
            size_t m_variadic_parameters_count = {};
        };

        Operator & AddOverload(const Overload& i_overload);

        Operator & AddOverload(VarEvaluateFunction i_evaluate, 
            std::vector<Parameter> i_parameters = {},
            size_t i_variadic_parameters_count = {})
        {
            return AddOverload({i_evaluate, std::move(i_parameters), i_variadic_parameters_count});
        }
        
            // canonicalization

        /* In math you can express the same thing in a countless number of ways. For example
           'real x * 2' and 'x + x' are the same thing. But we are not interested in spelling, 
           but  rather in semantics, and we consider x*2 and x+x the same thing.
           So we transform expresssions in a way that if they have the same semantics, the also 
           have the same syntactic tree. This transformation is arbitrary: for example we can sort
           the operands of a regural n-ary operator (see IsRegularNAry) to transform any 'b+a' to 
           'a+b', so that 'a+b == b+a' can be reduced to 'true' simply by comparing syntactic trees.
           This transformation is called canonicalization, and it's a best effort: we may have an
           f(x) and a g(x) that are always equal, but we may not have a canonicalization that proves
           it. */
        
        using CanonicalizeFunction = std::optional<Tensor>(*)(const Tensor & i_source);

        Operator & SetCanonicalize(CanonicalizeFunction i_func);


            // eligible for propagation

        using EligibleForPropagation = bool(*)(const std::any & i_attachment, Span<const Tensor> i_operands);

        Operator & SetEligibleForPropagation(EligibleForPropagation i_func);


            // gradient of operand

        using GradientOfOperandFunction = Tensor(*)(const Tensor & i_self, const Tensor & i_self_gradient, size_t i_operand_index);

        Operator & SetGradientOfOperand(GradientOfOperandFunction i_func);


            // identity element

        const std::optional<TensorValue> & GetIdentityValue() const { return m_identity_value; }


            // attachment compare

        using AttachmentComparer = bool (*)(const std::any & i_left, const std::any & i_right);

        Operator & SetAttachmentComparer(AttachmentComparer i_attachment_comparer);

        template <typename ATTACHMENT_TYPE>
            Operator & SetAttachmentComparer()
        {
            return SetAttachmentComparer([](const std::any & i_left, const std::any & i_right) {
                return std::any_cast<const ATTACHMENT_TYPE &>(i_left) ==
                    std::any_cast<const ATTACHMENT_TYPE &>(i_right);
            });
        }

        bool AttachmentsEqual(const std::any & i_left, const std::any & i_right) const;


            // attachment hash

        using AttachmentHasher = void (*)(Hash & i_dest, const std::any & i_attachment);

        Operator & SetAttachmentHasher(AttachmentHasher i_attachment_hasher);

        template <typename ATTACHMENT_TYPE>
            Operator & SetAttachmentHasher()
        {
            return SetAttachmentHasher([](Hash & i_dest, const std::any & i_attachment) {
                i_dest << std::any_cast<const ATTACHMENT_TYPE &>(i_attachment);
            });
        }

        void HashAttachment(Hash & i_dest, const std::any & i_attachment) const;


            // hash

        friend Hash & operator << (Hash & i_dest, const Operator & i_source);

    private:

        static TensorType DefaultDeduceType(const std::any & i_attachment,
            Span<const Tensor> i_operands);

        enum class OverloadMatchFlags
        {
            None = 0,
            AllowNumericPromotion = 1 << 0, /**< consider implicit numeric promotion 
                for the match */
            ProcessArguments = 1 << 1 /**< perform implicit numeric promotion 
                and default argument substitution */
        };

        friend OverloadMatchFlags operator | (OverloadMatchFlags i_first, OverloadMatchFlags i_second)
            { return CombineFlags(i_first, i_second); }

        bool IsEligibleForPropagation(const std::any & i_attachment, 
            Span<const Tensor> i_operands) const;

        static bool OverloadMatch(const Operator::Overload & i_overload, 
            Span<const Tensor> i_operands, OverloadMatchFlags i_flags,
            std::vector<Tensor> & o_arguments);

        const Overload * TryFindOverload(Span<const Tensor> i_operands,
            OverloadMatchFlags i_flags, std::vector<Tensor> & o_arguments) const;

        const Overload & FindOverload(Span<const Tensor> i_operands, 
            std::vector<Tensor> & o_arguments) const;

        std::optional<Tensor> TryConstantPropagation(
            const Overload & i_overload, const TensorType & i_result_type,
            Span<const Tensor> i_operands, const std::any & i_attachment) const;

        TensorValue Evaluate(const Overload & i_overload, const TensorType & i_result_type,
            Span<const Tensor> i_operands, const std::any & i_attachment) const;

        TensorValue Evaluate(const Overload & i_overload, const TensorType & i_result_type,
            Span<const TensorValue> i_operands, const std::any & i_attachment) const;

        static std::vector<TensorValue> ToValues(Span<const Tensor> i_tensors);

        void EnforceIdentityValue();

        // common canonicalizations for commutative-associative operators
        void CA_SortOperands(std::vector<Tensor> & i_operands) const;
        void CA_Flatten(std::vector<Tensor> & i_operands) const;
        void CA_PartialConstantPropagation(std::vector<Tensor> & i_operands,
            const std::any & i_attachment) const;

    private:
        std::string const m_name;
        std::string_view m_doc_description;
        std::string_view m_doc_return_type;
        Flags m_flags = {};
        DeduceTypeFunction m_deduce_type_func = {};
        EligibleForPropagation m_eligible_for_propagation = {};
        std::vector<Overload> m_overloads = {};
        CanonicalizeFunction m_canonicalize_func = {};
        GradientOfOperandFunction m_gradient_of_input_func = {};
        AttachmentComparer m_attachment_comparer = {};
        AttachmentHasher m_attachment_hasher = {};
        std::optional<TensorValue> m_identity_value;
    };
}
