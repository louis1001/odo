//
// Created by Luis Gonzalez on 12/19/20.
//

#ifndef ODO_NODERESULT_H
#define ODO_NODERESULT_H

#include "Interpreter/symbol.h"

namespace Odo::Semantics {
    struct NodeResult {
        Interpreting::Symbol* type {nullptr};
        bool is_constant {true};
        bool has_side_effects {false};
    };
}

#endif //ODO_NODERESULT_H
