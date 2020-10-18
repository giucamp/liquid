
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "private_common.h"
#include "liquid/tensor.h"
#include "operator.h"
#include <memory>
#include <variant>
#include <functional>

namespace liquid
{
    class Scope : public std::enable_shared_from_this<Scope>
    {
    public:

        // disable copy
        Scope(const Scope &) = delete;
        Scope & operator = (const Scope &) = delete;;

        /* Returns the root scope, which is always empty and unmodifiable */
        static const std::shared_ptr<const Scope> & Root();

        /* Creates a scope that has this as parent. This is the only way to create an instance of Scope. */
        std::shared_ptr<Scope> MakeInner() const;

        // a member may be a Tensor or an Operator
        using Member = std::variant< std::monostate, Tensor, std::reference_wrapper<const Operator> >;

        std::shared_ptr<const Scope> const & GetParent() const { return m_parent; }

        const std::vector<Rule> & GetRules() const { return m_rules; }

        const std::vector<Tensor> & GetValues() const { return m_values; }

        void AddVariable(const Tensor & i_new_variable);

        // never returns a monostate member
        Member Lookup(std::string_view i_name) const;

        Member TryLookup(std::string_view i_name) const;

    protected:
        Scope(const std::shared_ptr<const Scope> & i_parent);

    private:
        std::vector<Rule> m_rules;
        std::vector<Tensor> m_values;
        std::vector<Tensor> m_declarations;
        std::shared_ptr<const Scope> const m_parent;
    };
}
