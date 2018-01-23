#include "regions.h"

Regions::Regions(map<u_int, float> _exclusive, vector<region_t> _overlapping)
        : _exclusive(_exclusive),
          _overlapping(_overlapping) {
}

Regions::~Regions() {
}

// Returns the total coverage (considering the all nodes are active),
// the partial coverage (considering that ignore_nodes are inactive),
// the total and partial overlapping areas.
vector<float>
Regions::get_all(vector<u_int> ignore_nodes, vector<u_int> dead_nodes) {
  float total_coverage      = 0.0;
  float total_overlapping   = 0.0;
  float partial_overlapping = 0.0; 

  // traverses the exclusive regions
  for (auto const &region: _exclusive) {
    total_coverage += region.second;
  }
  // excludes dead nodes
  for (auto const &node_id: dead_nodes) {
    total_coverage -= _exclusive[node_id];
  }
  float partial_coverage    = total_coverage;

  // remove from partial ignore_nodes
  for (auto const &ignore_node: ignore_nodes) {
    partial_coverage -= _exclusive[ignore_node];
  }

  // traverses the overlapping regions
  for (auto const &region: _overlapping) {
    // subtract sets: owners - ignore and check if region would still
    // exist or overlap
    unsigned int awake_remains = region.first.size();
    unsigned int alive_remains = region.first.size();
    for (auto const &owner: region.first) {
      for (auto const &ignore_node: ignore_nodes) {
        if (owner == ignore_node) {
          awake_remains--;
        }
      }
    // subtract sets: owners - dead and check if region would still
    // exist or overlap
      for (auto const &dead_node: dead_nodes) {
        if (owner == dead_node) {
          alive_remains--;
          awake_remains--;
        }
      }
    }
    if (alive_remains == 1) {
      total_coverage    += region.second;
    } else if (alive_remains > 1) {
      total_coverage    += region.second;
      total_overlapping += region.second;
    }

    if (awake_remains == 1) {
      partial_coverage    += region.second;
    } else if (awake_remains > 1) {
      partial_coverage    += region.second;
      partial_overlapping += region.second;
    }
  }

  // TODO allocate fixed-size array
  vector<float> returns;
  returns.push_back(total_coverage);
  returns.push_back(partial_coverage);
  returns.push_back(total_overlapping);
  returns.push_back(partial_overlapping);
   
  return returns;
}
