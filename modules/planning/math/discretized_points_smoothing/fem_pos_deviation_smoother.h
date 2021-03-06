/******************************************************************************
 * Copyright 2019 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file
 **/

#pragma once

#include <utility>
#include <vector>

#include "osqp/include/osqp.h"

namespace apollo {
namespace planning {

struct FemPosDeviationOsqpSettings {
  int max_iter = 4000;
  double time_limit = 0.0;
  bool verbose = true;
  bool scaled_termination = true;
  bool warm_start = true;
};

/*
 * @brief:
 * This class solve an optimization problem:
 * Y
 * |
 * |                       P(x1, y1)  P(x2, y2)
 * |            P(x0, y0)                       ... P(x(k-1), y(k-1))
 * |P(start)
 * |
 * |________________________________________________________ X
 *
 *
 * Given an initial set of points from 0 to k-1,  The goal is to find a set of
 * points which makes the line P(start), P0, P(1) ... P(k-1) "smooth".
 */

class FemPosDeviationSmoother {
 public:
  FemPosDeviationSmoother() = default;

  virtual ~FemPosDeviationSmoother() = default;

  void set_ref_points(
      const std::vector<std::pair<double, double>>& ref_points) {
    ref_points_ = ref_points;
  }

  void set_x_bounds_around_refs(
      const std::vector<double>& x_bounds_around_refs) {
    x_bounds_around_refs_ = x_bounds_around_refs;
  }

  void set_y_bounds_around_refs(
      const std::vector<double>& y_bounds_around_refs) {
    y_bounds_around_refs_ = y_bounds_around_refs;
  }

  void set_weight_fem_pos_deviation(const double weight_fem_pos_deviation) {
    weight_fem_pos_deviation_ = weight_fem_pos_deviation;
  }

  void set_weight_path_length(const double weight_path_length) {
    weight_path_length_ = weight_path_length;
  }

  void set_weight_ref_deviation(const double weight_ref_deviation) {
    weight_ref_deviation_ = weight_ref_deviation;
  }

  bool Smooth(const FemPosDeviationOsqpSettings& solver_settings);

  const std::vector<double>& opt_x() const { return x_; }

  const std::vector<double>& opt_y() const { return y_; }

 private:
  void CalculateKernel(std::vector<c_float>* P_data,
                       std::vector<c_int>* P_indices,
                       std::vector<c_int>* P_indptr);

  void CalculateOffset(std::vector<c_float>* q);

  void CalculateAffineConstraint(std::vector<c_float>* A_data,
                                 std::vector<c_int>* A_indices,
                                 std::vector<c_int>* A_indptr,
                                 std::vector<c_float>* lower_bounds,
                                 std::vector<c_float>* upper_bounds);

  void SetPrimalWarmStart(std::vector<c_float>* primal_warm_start);

  bool OptimizeWithOsqp(
      const size_t kernel_dim, const size_t num_affine_constraint,
      std::vector<c_float>* P_data, std::vector<c_int>* P_indices,
      std::vector<c_int>* P_indptr, std::vector<c_float>* A_data,
      std::vector<c_int>* A_indices, std::vector<c_int>* A_indptr,
      std::vector<c_float>* lower_bounds, std::vector<c_float>* upper_bounds,
      std::vector<c_float>* q, std::vector<c_float>* primal_warm_start,
      OSQPData* data, OSQPWorkspace** work, OSQPSettings* settings);

 private:
  // Reference points and deviation bounds
  std::vector<std::pair<double, double>> ref_points_;
  std::vector<double> x_bounds_around_refs_;
  std::vector<double> y_bounds_around_refs_;

  // Weights in optimization cost function
  double weight_fem_pos_deviation_ = 0.0;
  double weight_path_length_ = 0.0;
  double weight_ref_deviation_ = 0.0;

  // Optimization problem definitions
  int num_of_points_ = 0;
  int num_of_variables_ = 0;
  int num_of_constraints_ = 0;

  // Optimized_result
  std::vector<double> x_;
  std::vector<double> y_;
};
}  // namespace planning
}  // namespace apollo
