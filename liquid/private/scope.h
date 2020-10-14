
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

        using Member = std::variant<std::monostate, Tensor, std::reference_wrapper<const Operator>>;

        Scope(const Scope &) = delete;
        Scope & operator = (const Scope &) = delete;;

        static const std::shared_ptr<const Scope> & Root();

        std::shared_ptr<const Scope> MakeInner(Span<const Rule> i_rules, Span<const Tensor> i_values) const;

        std::shared_ptr<const Scope> const & GetParent() const { return m_parent; }

        const std::vector<Rule> & GetRules() const { return m_rules; }

        const std::vector<Tensor> & GetValues() const { return m_values; }

        // never returns a monostate member
        Member Lookup(std::string_view i_name) const;

        Member TryLookup(std::string_view i_name) const;

    protected:
        Scope(const std::shared_ptr<const Scope> & i_parent,
            Span<const Rule> i_rules, Span<const Tensor> i_values);

    private:
        std::vector<Rule> m_rules;
        std::vector<Tensor> m_values;
        std::shared_ptr<const Scope> const m_parent;
    };
}
