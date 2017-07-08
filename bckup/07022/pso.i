/* File pso.i */
%module pso
%include "std_map.i"
%include "std_pair.i"
%include "std_vector.i"

%{
  #include "pso.h"
%}

%template(vector0) std::vector<int>;
%template(vector1) std::vector<std::pair<std::vector<int>, float>>;
%template(vector2) std::vector<char>;
%template(map0) std::map<int, float>;

%include "pso.h"
