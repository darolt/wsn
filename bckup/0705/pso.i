/* File pso.i */
%module pso
%include "std_map.i"
%include "std_pair.i"
%include "std_vector.i"

%{
  #include "pso.h"
%}

%template(vector0) std::vector<int>;
%template(vector1) std::vector<std::pair<std::vector<unsigned int>,
                                         float>>;
%template(vector2) std::vector<char>;
%template(vector3) std::vector<float>;
%template(vector4) std::vector<unsigned int>;
%template(map0)    std::map<unsigned int, float>;
%template(pair0)   std::pair<std::vector<unsigned int>,
                             std::vector<float>>;
%template(pair1)   std::pair<std::vector<unsigned int>,
                             float>;
%template(pair2)   std::pair<std::vector<char>,
                             std::vector<float>>;

%include "pso.h"
