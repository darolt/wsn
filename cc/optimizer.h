// The version of regions in c++ is used to improve performance. This
// code is supposed to be called by the Python script.

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <random>
#include "regions.h"
#include "custom_types.h"


class Optimizer {
  public:
    Optimizer(dict_t exclusive, regions_t overlapping,
                vector<u_int> ids, config_t config);
    virtual ~Optimizer();

    // returns a vector with the best configuration found (best particle),
    // indicating, for each node, if it should sleep or not;
    // the learning trace (trace of the best fitness value at each iteration);
    // and a vector with the coverage and overlapping areas for the best
    // configuration
    individual_t Run(float_v energies, u_int head_id);

    // setters & getters
    void SetAlpha(float value);
    void SetBeta(float value);
    vector<float> GetLearningTrace();
    float GetBestCoverage();
    float GetBestOverlapping();

  private:
    Regions *regions_;

    float FITNESS_ALPHA_;
    float FITNESS_BETA_;

    // Returns a float indicating how fit a individual/particle is,
    // and the coverage and overlapping areas for that particle.
    fitness_ret_t Fitness(individual_t individual, vector<float> energies,
                          float total_energy, char do_print);

  protected:
    vector<u_int> ids_;
    u_int nb_nodes_;

    // vector with all individuals
    vector<individual_t> individuals_;
    // each index contains the best individuals found in that index
    // (it supposes that the number of individuals is constant)
    vector<individual_t> best_locals_;
    // best individual found
    individual_t best_global_;
    // vector with the fitness of the best individual found
    // (used for optimization)
    vector<float> best_local_fitness_;
    // fitness of the best individual in the history
    float best_global_fitness_;

    // coverage and overlapping for the best configuration found in the
    // last run. These are not necessarily the best coverages found.
    float best_coverage_;
    float best_overlapping_;
    // learning trace for the last run
    vector<float> learning_trace_;

    // random related
    default_random_engine generator_;

    // from config.py
    u_int NB_INDIVIDUALS_;
    u_int MAX_ITERATIONS_;
    float WMAX_;
    float WMIN_;

    void PrintIndividual(individual_t individual);

    virtual void Initialize(float_v energies, u_int head_id,
                            float total_energy);
    virtual void Optimize(float_v energies, const vector<u_int> &can_sleep,
                          float total_energy) = 0;

    void UpdateGenerationFitness(float_v energies, float total_energy);
};
#endif //OPTIMIZER_H
