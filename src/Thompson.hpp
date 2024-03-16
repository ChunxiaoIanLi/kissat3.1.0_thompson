
#ifdef __cplusplus
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <vector>
#include <boost/random.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
extern "C" {
typedef boost::mt19937 base_generator;

  // template<class T>
  // size_t argmax(const std::vector<T>& v){
  // return std::distance(v.begin(), std::max_element(v.begin(), v.end()));
  // }  

#endif
  
int select_lever_C(double reset_wins, double reset_loses, double noreset_wins, double noreset_loses);

unsigned int select_lever (double reset_wins, double reset_loses, double noreset_wins, double noreset_loses);

#ifdef __cplusplus  
} // extern "C"  
#endif
