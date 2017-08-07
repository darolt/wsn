// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef MY_PSO_H
#define MY_PSO_H

#include <map>
#include <utility>
#include <vector>
#include <random>
#include "optimizer.h"

using namespace std;

class MyPso: public Optimizer {
  public:
    MyPso(dict_t exclusive, regions_t overlapping,
        vector<u_int> ids, config_t config);
    ~MyPso();

  private:
    // attributes
    vector<float_v> velocity_;
  
    // change individual's position randomly (random walk). The number of
    // altered genes is proportional to acceleration
    void Move(individual_t &individual, vector<u_int> can_sleep, 
              float acceleration);

    // individual1 copy parts of individual2 position depending on influence
    // rate and how far it is from individual2 (the farer the more copies) 
    void Influence(const individual_t &original_individual,
                   const individual_t &influencer,
                   individual_t &new_individual,
                   vector<u_int> can_sleep,
                   float influence_rate);

    void Optimize(float_v energies, const vector<u_int> &can_sleep,
                  float total_energy);
};
#endif //MY_PSO_H
