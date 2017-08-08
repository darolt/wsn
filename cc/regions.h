// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef REGIONS_H
#define REGIONS_H

#include <map>
#include <utility>
#include <vector>

typedef struct {
  float partial_coverage;
  float total_coverage;
  float partial_overlapping;
  float total_overlapping;
  float exclusive_area;
} coverage_info_t;

typedef unsigned int u_int;
typedef std::pair<std::vector<u_int>,
                  float> region_t;

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
