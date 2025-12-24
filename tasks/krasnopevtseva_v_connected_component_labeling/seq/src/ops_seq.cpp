#include "krasnopevtseva_v_connected_component_labeling/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstdint>
#include <queue>
#include <random>
#include <tuple>
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
  if (static_cast<size_t>(height * width) != data.size()) {
    return false;
  }
  for (int pixel : data) {
    if (pixel != 0 && pixel != 1) {
      return false;
    }
  }
  return true;
}

bool KrasnopevtsevaVCCLSEQ::PreProcessingImpl() {
  const auto &[height, width, data] = GetInput();
  GetOutput() = std::vector<int>(data.size(), 0);
  return true;
}

bool KrasnopevtsevaVCCLSEQ::RunImpl() {
  const auto &[height, width, binary_data] = GetInput();
  auto &output = GetOutput();

  int total_pixels = height * width;
  output.resize(total_pixels, 0);

  std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  int label = 1;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int idx = y * width + x;

      if (binary_data[idx] == 1 && output[idx] == 0) {
        std::queue<std::pair<int, int>> q;
        q.push({y, x});
        output[idx] = label;

        while (!q.empty()) {
          auto [current_y, current_x] = q.front();
          q.pop();

          for (const auto &[dy, dx] : directions) {
            int ny = current_y + dy;
            int nx = current_x + dx;

            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
              int nidx = ny * width + nx;
              if (binary_data[nidx] == 1 && output[nidx] == 0) {
                output[nidx] = label;
                q.push({ny, nx});
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
