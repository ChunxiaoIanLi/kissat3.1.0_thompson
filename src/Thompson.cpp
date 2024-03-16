#include "Thompson.hpp"

extern "C" int select_lever_C(double reset_wins, double reset_loses, double noreset_wins, double noreset_loses)
{
  return (select_lever(reset_wins, reset_loses, noreset_wins, noreset_loses));
}

template<class T>
size_t argmax(const std::vector<T>& v){
  return std::distance(v.begin(), std::max_element(v.begin(), v.end()));
}

unsigned int select_lever (double reset_wins, double reset_loses, double noreset_wins, double noreset_\
loses){
    // gen is a Mersenne Twister random generator. We initialzie it here to keep
    // the binary deterministic.
    base_generator gen;

    // Number of trials per bandit
    std::vector<double> trials;
    // Number of wins per bandif
    std::vector<double> wins;
    // Beta distributions of the priors for each bandit
    std::vector<boost::random::beta_distribution<> > prior_dists;

    int num_arms = 2;
    trials = std::vector<double>(num_arms, 0);
    wins = std::vector<double>(num_arms, 0);

    prior_dists.push_back(boost::random::beta_distribution<>(reset_wins, reset_loses));
    prior_dists.push_back(boost::random::beta_distribution<>(noreset_wins, noreset_loses));


    std::vector<double> priors;
    // Sample a random value from each prior distribution.
    for (auto& dist : prior_dists) {
      priors.push_back(dist(gen));
    }
        
    // Select the bandit that has the highest sampled value from the prior
    return static_cast<unsigned int>(argmax(priors));
}
