//
// Created by Luis Gonzalez on 12/19/20.
//

#ifndef ODO_SEMANTICANALYZER_H
#define ODO_SEMANTICANALYZER_H
#include "NodeResult.h"
#include "Parser/AST/Node.h"

namespace Odo::Semantics {
    class SemanticAnalyzer {
        SemanticAnalyzer();

        NodeResult visit(const std::shared_ptr<Parsing::Node>&);
    };
}

#endif //ODO_SEMANTICANALYZER_H
