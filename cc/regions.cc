#include "regions.h"
#include <stdio.h>

using namespace std;

Regions::Regions(map<u_int, float> exclusive, vector<region_t> overlapping)
        : exclusive_(exclusive),
          overlapping_(overlapping) {
  // traverses the exclusive regions
  total_coverage_exclusive_ = 0.0;
  for (auto const &region: exclusive_) 
    total_coverage_exclusive_ += region.second;
}

Regions::~Regions() {
}

// Returns the total coverage (considering the all nodes are active),
// the partial coverage (considering that inactive_nodes are inactive),
// the total and partial overlapping areas.
coverage_info_t
Regions::GetCoverage(const vector<char> &individual,
                     const vector<float> &energies) {
  float partial_overlapping = 0.0; 

  float partial_coverage = total_coverage_;

  // remove from partial inactive_nodes
  for (unsigned int idx=0; idx<individual.size(); idx++)
    if (individual[idx] == 1 && energies[idx] != 0.0)
      partial_coverage -= exclusive_[idx];

  if (partial_coverage < 0.0)
    partial_coverage = 0.0;

  float exclusive_area = partial_coverage;

  // traverses the overlapping regions
  for (auto const &region: overlapping_) {
    bool all_sleeping = true;
    bool all_depleted = true;
    short int nb_remaining_owners = 0;
    for (auto const &owner: region.first) {
      if (individual[owner] == 0 && energies[owner] != 0.0) {// active && alive
        nb_remaining_owners++;
        all_sleeping = false;
        if (nb_remaining_owners > 1)
          break;
      }
      if (energies[owner] != 0.0)
        all_depleted = false;
    }
   
    if (all_sleeping && !all_depleted)
      partial_coverage -= region.second;

    if (nb_remaining_owners == 1)
      exclusive_area += region.second;
  }

  //printf("partial coverage: %f\n", partial_coverage);
  //printf("total coverage: %f\n", total_coverage_);
  //printf("overlapped partial coverage: %f\n", partial_overlapping);
  //printf("overlapped total coverage: %f\n", total_overlapping);
  coverage_info_t coverage_info;
  coverage_info.partial_coverage    = partial_coverage;
  coverage_info.total_coverage      = total_coverage_;
  coverage_info.partial_overlapping = partial_overlapping;
  coverage_info.total_overlapping   = total_overlapping_;
  coverage_info.exclusive_area      = exclusive_area;
   
  return coverage_info;
}

void
Regions::InitSession(const std::vector<float> &energies) {
  total_coverage_ = total_coverage_exclusive_;

  // excludes dead nodes
  bool all_dead = true;
  for (unsigned int idx=0; idx<energies.size(); idx++)
    if (energies[idx] == 0.0)
      total_coverage_ -= exclusive_[idx];
    else
      all_dead = false;

  // avoids approximation errors
  if (all_dead)
    total_coverage_ = 0.0;

  total_overlapping_ = 0.0;
  for (auto const &region: overlapping_) {
    for (auto const &owner: region.first)
      if (energies[owner] != 0.0) {
        total_coverage_ += region.second;
        break;
      }

    //if (alive_remains == 1) {
    //} else if (alive_remains > 1) {
    //  total_overlapping_ += region.second;
    //}
  }
}

