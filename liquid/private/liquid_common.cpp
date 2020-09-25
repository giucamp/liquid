
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "liquid/liquid_common.h"

namespace liquid
{
    std::ostream& operator << (std::ostream& i_ostream, ScalarType i_scalar_type)
    {
        switch (i_scalar_type)
        {
            case ScalarType::Any: i_ostream << "Any"; break;
            case ScalarType::Real: i_ostream << "Real"; break;
            case ScalarType::Integer: i_ostream << "Integer"; break;
            case ScalarType::Bool: i_ostream << "Bool"; break;
            default: Panic("Unrecognized scalar type ", static_cast<int>(i_scalar_type));
        }
    }
}
