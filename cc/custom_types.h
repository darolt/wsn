#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

// most of these definitions are used to improve readability
typedef unsigned int u_int;
typedef std::vector<float> float_v;
typedef std::vector<u_int> u_int_v;

typedef std::pair<std::vector<u_int>,
                  float> region_t;
typedef std::vector<region_t> regions_t;
typedef std::map<u_int, float> dict_t;
// using char for the smallest addressable unit
typedef std::vector<char> individual_t;
typedef std::pair<std::map<std::string, u_int>, 
                  std::map<std::string, float>> config_t;

typedef struct {
  float partial_coverage;
  float total_coverage;
  float partial_overlapping;
  float total_overlapping;
  float exclusive_area;
} coverage_info_t;

typedef struct {
  float fitness_value;
  coverage_info_t coverage_info;
} fitness_ret_t;

#endif // CUSTOM_TYPES_H
