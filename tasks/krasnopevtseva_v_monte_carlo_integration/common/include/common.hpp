#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace krasnopevtseva_v_monte_carlo_integration {

using InType = int;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace krasnopevtseva_v_monte_carlo_integration
