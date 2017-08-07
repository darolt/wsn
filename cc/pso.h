// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef PSO_H
#define PSO_H

#include <map>
#include <utility>
#include <vector>
#include <random>
#include "optimizer.h"

using namespace std;

class Pso: public Optimizer {
  public:
    Pso(dict_t exclusive, regions_t overlapping,
        vector<u_int> ids, config_t config);
    ~Pso();

  private:
    // attributes
    vector<float_v> velocity_;
  
    void Optimize(float_v energies, const vector<u_int> &can_sleep,
                  float total_energy);
};
#endif //PSO_H
