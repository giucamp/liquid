
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "private_common.h"
#include "liquid/tensor.h"
#include "scope.h"

namespace liquid
{
    namespace miu6
    {
        Tensor ParseExpression(std::string_view i_source, 
            const std::shared_ptr<const Scope> & i_scope = Scope::Root());
    }

} // namespace liquid
