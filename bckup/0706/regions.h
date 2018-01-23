// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef REGIONS_H
#define REGIONS_H

#include <map>
#include <utility>
#include <vector>

using namespace std;

typedef unsigned int u_int;

typedef pair<vector<u_int>, float> region_t;

class Regions {
  public:
    Regions(map<u_int, float> _exclusive, vector<region_t> _overlapping);
    ~Regions();

    vector<float> get_all(vector<u_int> ignore_nodes, vector<u_int> dead_nodes);

  private:
    map<u_int, float> _exclusive;
    vector<region_t> _overlapping; 

};
#endif //REGIONS_H
