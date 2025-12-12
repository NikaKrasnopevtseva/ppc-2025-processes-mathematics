#pragma once

#include <cmath>
#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace krasnopevtseva_v_bubble_sort {

using InType = std::vector<int>;
using OutType = std::vector<int>;
using TestType = std::tuple<std::vector<int>, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;
}  // namespace krasnopevtseva_v_bubble_sort
