// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef REGIONS_H
#define REGIONS_H

#include <map>
#include <utility>
#include <vector>
#include "types.h"

typedef unsigned int u_int;
typedef std::pair<std::vector<u_int>,
                  float> region_t;

class Regions {
  public:
    Regions(std::map<u_int, float> _exclusive,
            std::vector<region_t> _overlapping);
    ~Regions();

    coverage_info_t GetCoverage(const std::vector<char> &individual,
                                const std::vector<float> &energies);

    void InitSession(const std::vector<float> &energies);

  private:
    std::map<u_int, float> exclusive_;
    std::vector<region_t> overlapping_; 

    float total_coverage_exclusive_ = 0.0;
    // session attributes
    float total_coverage_    = 0.0;
    float total_overlapping_ = 0.0;

};
#endif //REGIONS_H
