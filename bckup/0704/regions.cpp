#include "regions.h"

Regions::Regions(map<int, float> _exclusive, vector<region_t> _overlapping)
        : _exclusive(_exclusive),
          _overlapping(_overlapping) {
}

Regions::~Regions() {
}

// Returns the total coverage (considering the all nodes are active),
// the partial coverage (considering that ignore_nodes are inactive),
// the total and partial overlapping areas.
vector<float>
Regions::get_all(vector<int> ignore_nodes) {
  float total_coverage      = 0.0;
  float total_overlapping   = 0.0;
  float partial_overlapping = 0.0; 

  // traverses the exclusive regions
  for (auto const &region: _exclusive) {
    total_coverage += region.second;
  }
  float partial_coverage    = total_coverage;

  // remove from partial ignore_nodes
  for (auto const &ignore_node: ignore_nodes) {
    partial_coverage -= _exclusive[ignore_node];
  }

  // traverses the overlapping regions
  for (auto const &region: _overlapping) {
    total_coverage    += region.second;
    total_overlapping += region.second;

    // subtract sets: owners - ignore and check if region would still
    // exist or overlap
    unsigned int remains = region.first.size();
    for (auto const &owner: region.first) {
      for (auto const &ignore_node: ignore_nodes) {
        if (owner == ignore_node) {
          remains--;
        }
      }
    }

    if (remains == 1) {
      partial_coverage    += region.second;
    } else if (remains > 1) {
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
