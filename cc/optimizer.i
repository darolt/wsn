
%include "std_map.i"
%include "std_pair.i"
%include "std_vector.i"
%include "std_string.i"

%{
  #include "optimizer.h"
%}

%template(vector0)      std::vector<int>;
%template(regions_t)    std::vector<std::pair<std::vector<unsigned int>,
                                              float>>;
%template(individual_t) std::vector<char>;
%template(float_v)      std::vector<float>;
%template(vector1)      std::vector<unsigned int>;
%template(vector2)      std::vector<std::vector<char>>;
%template(dict_t)       std::map<unsigned int, float>;
%template(map0)         std::map<std::string, unsigned int>;
%template(map1)         std::map<std::string, float>;
%template(config_t)     std::pair<std::map<std::string, unsigned int>,
                                  std::map<std::string, float>>;
%template(region_t)     std::pair<std::vector<unsigned int>,
                                  float>;

%include "optimizer.h"
