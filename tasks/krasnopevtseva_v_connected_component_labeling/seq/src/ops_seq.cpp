#include "krasnopevtseva_v_connected_component_labeling/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

#include "krasnopevtseva_v_connected_component_labeling/common/include/common.hpp"

namespace krasnopevtseva_v_connected_component_labeling {

KrasnopevtsevaVCCLSEQ::KrasnopevtsevaVCCLSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool KrasnopevtsevaVCCLSEQ::ValidationImpl() {
  const auto &[height, width, data] = GetInput();
  if (data.empty()) {
    return false;
  }
  if (height <= 0 || width <= 0) {
    return false;
  }
  const size_t total_size = static_cast<size_t>(height) * static_cast<size_t>(width);
  if (total_size != data.size()) {
    return false;
  }
  return std::ranges::all_of(data, [](int pixel) { return pixel == 0 || pixel == 1; });
}

bool KrasnopevtsevaVCCLSEQ::PreProcessingImpl() {
  const auto &[height, width, data] = GetInput();
  GetOutput() = std::vector<int>(data.size(), 0);
  return true;
}

bool KrasnopevtsevaVCCLSEQ::RunImpl() {
  const auto &[height, width, binary_data] = GetInput();
  auto &output = GetOutput();

  const int total_pixels = height * width;
  output.resize(static_cast<size_t>(total_pixels), 0);

  const std::array<std::pair<int, int>, 4> directions = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
  int label = 1;

  for (int row = 0; row < height; ++row) {
    for (int col = 0; col < width; ++col) {
      const int idx = (row * width) + col;

      if (binary_data[idx] == 1 && output[idx] == 0) {
        std::queue<std::pair<int, int>> queue;
        queue.emplace(row, col);
        output[idx] = label;

        while (!queue.empty()) {
          auto [current_row, current_col] = queue.front();
          queue.pop();

          for (const auto &[delta_row, delta_col] : directions) {
            const int new_row = current_row + delta_row;
            const int new_col = current_col + delta_col;

            if (new_row >= 0 && new_row < height && new_col >= 0 && new_col < width) {
              const int new_idx = (new_row * width) + new_col;
              if (binary_data[new_idx] == 1 && output[new_idx] == 0) {
                output[new_idx] = label;
                queue.emplace(new_row, new_col);
              }
            }
          }
        }

        label++;
      }
    }
  }

  return true;
}

bool KrasnopevtsevaVCCLSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace krasnopevtseva_v_connected_component_labeling
