#ifndef ECCA_H
#define ECCA_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <random>
#include "regions.h"
#include "custom_types.h"

class Ecca {
  public:
    Ecca(dict_t exclusive, regions_t overlapping,
                std::vector<u_int> ids, config_t config);
    virtual ~Ecca();

    // returns a std::vector with the best configuration found (best particle),
    // indicating, for each node, if it should sleep or not;
    // the learning trace (trace of the best fitness value at each iteration);
    // and a std::vector with the coverage and overlapping areas for the best
    // configuration
    individual_t Run(float_v energies, u_int head_id);

    // setters & getters
    void SetAlpha(float value);
    void SetBeta(float value);
    std::vector<float> GetLearningTrace();
    float GetBestCoverage();
    float GetBestOverlapping();

  private:
    Regions *regions_;

    // Returns a float indicating how fit a individual/particle is,
    // and the coverage and overlapping areas for that particle.
    fitness_ret_t Fitness(const individual_t &individual, std::vector<float> energies,
                          float total_energy, char do_print);

  protected:
    std::vector<u_int> ids_;
    u_int nb_nodes_;

    // std::vector with all individuals
    std::vector<individual_t> population_;
    // each index contains the best individuals found in that index
    // (it supposes that the number of individuals is constant)
    std::vector<individual_t> best_locals_;
    // best individual found
    individual_t best_global_;
    // std::vector with the fitness of the best individual found
    // (used for optimization)
    std::vector<float> best_local_fitness_;
    // fitness of the best individual in the history
    float best_global_fitness_;

    // coverage and overlapping for the best configuration found in the
    // last run. These are not necessarily the best coverages found.
    float best_coverage_;
    float best_overlapping_;
    // learning trace for the last run
    std::vector<float> learning_trace_;

    // random related
    std::default_random_engine generator_;

    // from config.py
    u_int NB_INDIVIDUALS_;
    u_int MAX_ITERATIONS_;
    float WMAX_;
    float WMIN_;

    void PrintIndividual(individual_t individual);

    virtual void Initialize(float_v energies, u_int head_id,
                            float total_energy);
    virtual void Optimize(float_v energies, const std::vector<u_int> &can_sleep,
                          float total_energy) = 0;

    void UpdateGenerationFitness(float_v energies, float total_energy);
};
#endif //ECCA_H
