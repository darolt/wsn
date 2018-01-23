// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef REGIONS_H
#define REGIONS_H

#include <map>
#include <utility>
#include <vector>

using namespace std;

typedef unsigned int u_int;

typedef pair<vector<int>, float> region_t;

class Regions {
  public:
    Regions(map<int, float> _exclusive, vector<region_t> _overlapping);
    ~Regions();

    vector<float> get_all(vector<u_int> ignore_nodes);

  private:
    map<int, float> _exclusive;
    vector<region_t> _overlapping; 

};
#endif //REGIONS_H
