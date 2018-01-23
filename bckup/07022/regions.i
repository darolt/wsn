/* File regions.i */
%module regions
%include "std_map.i"
%include "std_pair.i"
%include "std_vector.i"

%{
  #include "regions.h"
%}

%template(vector0) std::vector<int>;
%template(vector1) std::vector<std::pair<std::vector<int>, float>>;
%template(map0) std::map<int, float>;

%include "regions.h"
