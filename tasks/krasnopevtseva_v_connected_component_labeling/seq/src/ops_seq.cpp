#include "krasnopevtseva_v_connected_component_labeling/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <queue>
#include <ranges>
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

  ProcessImage(binary_data, output, height, width);

  return true;
}

void KrasnopevtsevaVCCLSEQ::ProcessImage(const std::vector<int> &binary_data, std::vector<int> &output, int height,
                                         int width) {
  const std::array<std::pair<int, int>, 4> directions = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
  int label = 1;

  for (int row = 0; row < height; ++row) {
    for (int col = 0; col < width; ++col) {
      const int idx = (row * width) + col;

      if (IsUnlabeledPixel(binary_data, output, idx)) {
        LabelConnectedComponent(binary_data, output, row, col, label, directions, height, width);
        label++;
      }
    }
  }
}

bool KrasnopevtsevaVCCLSEQ::IsUnlabeledPixel(const std::vector<int> &binary_data, const std::vector<int> &output,
                                             int index) {
  return binary_data[index] == 1 && output[index] == 0;
}

void KrasnopevtsevaVCCLSEQ::LabelConnectedComponent(const std::vector<int> &binary_data, std::vector<int> &output,
                                                    int start_row, int start_col, int label,
                                                    const std::array<std::pair<int, int>, 4> &directions, int height,
                                                    int width) {
  std::queue<std::pair<int, int>> queue;
  queue.emplace(start_row, start_col);
  output[(start_row * width) + start_col] = label;

  while (!queue.empty()) {
    auto [current_row, current_col] = queue.front();
    queue.pop();

    ExploreNeighbors(binary_data, output, current_row, current_col, label, directions, height, width, queue);
  }
}

void KrasnopevtsevaVCCLSEQ::ExploreNeighbors(const std::vector<int> &binary_data, std::vector<int> &output,
                                             int current_row, int current_col, int label,
                                             const std::array<std::pair<int, int>, 4> &directions, int height,
                                             int width, std::queue<std::pair<int, int>> &queue) {
  for (const auto &[delta_row, delta_col] : directions) {
    const int new_row = current_row + delta_row;
    const int new_col = current_col + delta_col;

    if (IsValidPosition(new_row, new_col, height, width)) {
      const int new_idx = (new_row * width) + new_col;
      if (IsUnlabeledPixel(binary_data, output, new_idx)) {
        output[new_idx] = label;
        queue.emplace(new_row, new_col);
      }
    }
  }
}

bool KrasnopevtsevaVCCLSEQ::IsValidPosition(int row, int col, int height, int width) {
  return row >= 0 && row < height && col >= 0 && col < width;
}

bool KrasnopevtsevaVCCLSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace krasnopevtseva_v_connected_component_labeling
