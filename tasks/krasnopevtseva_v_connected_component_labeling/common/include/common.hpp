#pragma once

#include <cmath>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace krasnopevtseva_v_connected_component_labeling {

using InType = std::tuple<int, int, std::vector<int>>;
using OutType = std::vector<int>;
using TestType = std::tuple<InType, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace krasnopevtseva_v_connected_component_labeling
