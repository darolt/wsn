// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef ECCA_H
#define ECCA_H

#include <map>
#include <utility>
#include <vector>
#include <random>

class Ecca {
  public:
    Ecca(dict_t exclusive, regions_t overlapping,
        vector<u_int> ids, config_t config);
    ~Ecca();

  private:
    // attributes
  
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
#endif //ECCA_H
