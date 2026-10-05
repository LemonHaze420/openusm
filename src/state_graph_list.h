#pragma once

#include "string_hash.h"
#include <vector.hpp>

namespace ai {

struct state_graph_list {
    _std::vector<string_hash> graphs;

    void add_entry(const string_hash &graph)
    {
        for (const auto &entry : graphs)
            if (entry == graph)
                return;
        graphs.push_back(graph);
    }
};
}  // namespace ai
