// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef REGIONS_H
#define REGIONS_H

#include <map>
#include <utility>
#include <vector>
#include "custom_types.h"

class Regions {
  public:
    Regions(std::map<u_int, float> _exclusive, std::vector<region_t> _overlapping);
    ~Regions();

    coverage_info_t GetAll(std::vector<u_int> ignore_nodes, std::vector<u_int> dead_nodes);

  private:
    std::map<u_int, float> _exclusive;
    std::vector<region_t> _overlapping; 

};
#endif //REGIONS_H
