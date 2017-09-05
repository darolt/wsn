// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef PSO_H
#define PSO_H

#include <map>
#include <utility>
#include <vector>
#include <random>
#include "optimizer.h"
#include "individual.h"

class Pso: public Optimizer {
  public:
    Pso(dict_t exclusive, regions_t overlapping,
        std::vector<u_int> ids, config_t config);
    ~Pso();

  private:
    // attributes
    std::vector<float_v> velocity_;
  
    void Optimize(const std::vector<u_int> &can_sleep);
    fitness_t Fitness(Individual &individual);
};
#endif //PSO_H
