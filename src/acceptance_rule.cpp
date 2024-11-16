#include "include/acceptance_rule.h"

#include <cmath>

  bool HillClimbing::Accept(int old_value, int new_value) {
    return new_value < old_value;
  }

  bool HillClimbingWithEqual::Accept(int old_value, int new_value) {
    return new_value <= old_value;
  }

  bool LateAcceptanceHillClimbing::Accept(int old_value, int new_value) {
    bool accepted = false;
    if (new_value <= old_value || new_value < values_[position_]) {
      old_value = new_value;
      accepted = true;
    }
    if (old_value < values_[position_]) {
      values_[position_] = old_value;
    }
    position_ = (position_ + 1) % length_;
    return accepted;
  }

  bool SimulatedAnnealing::Accept(int old_value, int new_value) {
    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    bool accepted = new_value <= old_value || r < exp((old_value - new_value) / temperature_);
    temperature_ *= decay_;
    return accepted;
  }