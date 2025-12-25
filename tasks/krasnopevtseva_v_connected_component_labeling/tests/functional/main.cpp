#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "krasnopevtseva_v_connected_component_labeling/common/include/common.hpp"
#include "krasnopevtseva_v_connected_component_labeling/mpi/include/ops_mpi.hpp"
#include "krasnopevtseva_v_connected_component_labeling/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace krasnopevtseva_v_connected_component_labeling {

class KrasnopevtsevaVCCLFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    const auto &[input_data, description] = test_param;

    std::string result = description;

    return result;
  }

 private:
  InType input_data_;

  bool CheckBasicProperties(const std::vector<int> &input_binary, const std::vector<int> &output_data, int height,
                            int width) const {
    const int total_pixels = height * width;

    if (output_data.size() != static_cast<size_t>(total_pixels)) {
      return false;
    }

    for (int index = 0; index < total_pixels; ++index) {
      if (input_binary[index] == 0 && output_data[index] != 0) {
        return false;
      }
      if (input_binary[index] == 1 && output_data[index] <= 0) {
        return false;
      }
    }

    return true;
  }

  bool CheckAdjacentConsistency(const std::vector<int> &input_binary, const std::vector<int> &output_data, int height,
                                int width) const {
    for (int row = 0; row < height; ++row) {
      for (int col = 0; col < width; ++col) {
        const int idx = (row * width) + col;
        if (input_binary[idx] == 1) {
          if (col > 0 && input_binary[idx - 1] == 1) {
            if (output_data[idx - 1] != output_data[idx]) {
              return false;
            }
          }
          if (col < width - 1 && input_binary[idx + 1] == 1) {
            if (output_data[idx + 1] != output_data[idx]) {
              return false;
            }
          }
          if (row > 0 && input_binary[idx - width] == 1) {
            if (output_data[idx - width] != output_data[idx]) {
              return false;
            }
          }
          if (row < height - 1 && input_binary[idx + width] == 1) {
            if (output_data[idx + width] != output_data[idx]) {
              return false;
            }
          }
        }
      }
    }

    return true;
  }

  bool CheckComponentConnectivity(const std::unordered_map<int, std::vector<int>> &label_to_indices,
                                  const std::vector<int> &output_data, int height, int width) const {
    for (const auto &[label, indices] : label_to_indices) {
      if (indices.empty()) {
        continue;
      }

      std::vector<bool> visited(static_cast<size_t>(height * width), false);
      std::queue<int> queue;

      queue.push(indices[0]);
      visited[static_cast<size_t>(indices[0])] = true;
      int visited_count = 1;

      while (!queue.empty()) {
        int current = queue.front();
        queue.pop();
        const int row = current / width;
        const int col = current % width;

        const std::array<std::pair<int, int>, 4> directions = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
        for (const auto &[delta_row, delta_col] : directions) {
          const int new_row = row + delta_row;
          const int new_col = col + delta_col;

          if (new_row >= 0 && new_row < height && new_col >= 0 && new_col < width) {
            const int new_idx = (new_row * width) + new_col;
            if (!visited[static_cast<size_t>(new_idx)] && output_data[new_idx] == label) {
              visited[static_cast<size_t>(new_idx)] = true;
              visited_count++;
              queue.push(new_idx);
            }
          }
        }
      }

      if (visited_count != static_cast<int>(indices.size())) {
        return false;
      }
    }

    return true;
  }

  bool CheckLabelConsistencyAcrossComponents(const std::vector<int> &input_binary, const std::vector<int> &output_data,
                                             int height, int width) const {
    for (int row = 0; row < height; ++row) {
      for (int col = 0; col < width; ++col) {
        const int idx = (row * width) + col;
        if (input_binary[idx] == 0) {
          continue;
        }
        const int current_label = output_data[idx];
        const std::array<std::pair<int, int>, 4> directions = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
        for (const auto &[delta_row, delta_col] : directions) {
          const int new_row = row + delta_row;
          const int new_col = col + delta_col;
          if (new_row >= 0 && new_row < height && new_col >= 0 && new_col < width) {
            const int new_idx = (new_row * width) + new_col;
            if (input_binary[new_idx] == 1 && output_data[new_idx] != current_label) {
              return false;
            }
          }
        }
      }
    }

    return true;
  }

  bool CheckLabelNumbering(const std::vector<int> &output_data) const {
    std::vector<int> all_labels;
    for (int value : output_data) {
      if (value > 0) {
        all_labels.push_back(value);
      }
    }

    std::ranges::sort(all_labels);
    auto unique_end = std::ranges::unique(all_labels);
    all_labels.erase(unique_end.begin(), all_labels.end());

    if (!all_labels.empty()) {
      if (all_labels[0] != 1) {
        return false;
      }

      for (size_t index = 1; index < all_labels.size(); ++index) {
        if (all_labels[index] != all_labels[index - 1] + 1) {
          return false;
        }
      }
    }

    return true;
  }

 protected:
  void SetUp() override {
    auto test_param = std::get<static_cast<size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(test_param);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &[height, width, input_binary] = input_data_;

    if (!CheckBasicProperties(input_binary, output_data, height, width)) {
      return false;
    }

    if (!CheckAdjacentConsistency(input_binary, output_data, height, width)) {
      return false;
    }

    std::unordered_map<int, std::vector<int>> label_to_indices;
    for (int index = 0; index < height * width; ++index) {
      if (output_data[index] > 0) {
        label_to_indices[output_data[index]].push_back(index);
      }
    }

    if (!CheckComponentConnectivity(label_to_indices, output_data, height, width)) {
      return false;
    }

    if (!CheckLabelConsistencyAcrossComponents(input_binary, output_data, height, width)) {
      return false;
    }

    if (!CheckLabelNumbering(output_data)) {
      return false;
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

namespace {
TEST_P(KrasnopevtsevaVCCLFuncTests, CCL) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 12> kTestParam = {
  std::make_tuple(std::make_tuple(3, 3, std::vector<int>{
    0, 0, 0,
    0, 0, 0,
    0, 0, 0
  }), "empty_3x3"),
  
  std::make_tuple(std::make_tuple(3, 3, std::vector<int>{
    0, 0, 0,
    0, 1, 0,
    0, 0, 0
  }), "single_pixel"),
  
  std::make_tuple(std::make_tuple(4, 4, std::vector<int>{
    1, 1, 0, 0,
    1, 1, 0, 0,
    0, 0, 1, 1,
    0, 0, 1, 1
  }), "two_components"),
  
  std::make_tuple(std::make_tuple(10, 10, std::vector<int>{
    1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
    1, 0, 1, 0, 0, 0, 1, 0, 1, 0,
    1, 1, 1, 0, 0, 0, 1, 1, 1, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 1, 0, 0, 0, 0, 0, 0, 1, 1,
    0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 0, 0, 0
  }), "complex_10x10"),
  
  std::make_tuple(std::make_tuple(20, 30, []() {
    std::vector<int> data(static_cast<size_t>(20) * static_cast<size_t>(30), 0);
    for (int row = 0; row < 20; ++row) {
      for (int col = 0; col < 30; ++col) {
        if (col % 5 == 0) {  
          data[static_cast<size_t>(row) * static_cast<size_t>(30) + static_cast<size_t>(col)] = 1;
        }
      }
    }
    return data;
  }()), "vertical_lines_20x30"),
  
  std::make_tuple(std::make_tuple(30, 20, []() {
    std::vector<int> data(static_cast<size_t>(30) * static_cast<size_t>(20), 0);
    for (int row = 0; row < 30; ++row) {
      for (int col = 0; col < 20; ++col) {
        if (row % 5 == 0) {  
          data[static_cast<size_t>(row) * static_cast<size_t>(20) + static_cast<size_t>(col)] = 1;
        }
      }
    }
    return data;
  }()), "horizontal_lines_30x20"),
  
  std::make_tuple(std::make_tuple(8, 8, []() {
    std::vector<int> data(64, 0);
    for (int row = 0; row < 8; ++row) {
      for (int col = 0; col < 8; ++col) {
        if ((col + row) % 2 == 0) {
          data[static_cast<size_t>(row) * static_cast<size_t>(8) + static_cast<size_t>(col)] = 1;
        }
      }
    }
    return data;
  }()), "chessboard_8x8"),
  
  std::make_tuple(std::make_tuple(15, 15, std::vector<int>(225, 1)), "full_15x15"),
  
  std::make_tuple(std::make_tuple(17, 25, []() {
    std::vector<int> data(static_cast<size_t>(17) * static_cast<size_t>(25), 0);
    for (int row = 5; row < 12; ++row) {
      for (int col = 8; col < 17; ++col) {
        data[static_cast<size_t>(row) * static_cast<size_t>(25) + static_cast<size_t>(col)] = 1;
      }
    }
    return data;
  }()), "cross_boundary_17x25"),
  
  std::make_tuple(std::make_tuple(25, 25, []() {
    std::vector<int> data(625, 0);
    for (int index = 0; index < 25; ++index) {
      data[static_cast<size_t>(index) * static_cast<size_t>(25) + static_cast<size_t>(index)] = 1;
    }
    return data;
  }()), "big_diagonal_25x25"),
  
  std::make_tuple(std::make_tuple(20, 20, []() {
    std::vector<int> data(400, 0);
    const std::array<int, 6> points = {2, 5, 8, 12, 15, 18};
    for (const int row : points) {
      for (const int col : points) {
        const int idx = row * 20 + col;
        if (idx < 400) {
          data[static_cast<size_t>(idx)] = 1;
        }
      }
    }
    return data;
  }()), "scattered_points_20x20"),
  
  std::make_tuple(std::make_tuple(8, 8, std::vector<int>{
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1
  }), "square_8x8")
};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<KrasnopevtsevaVCCLMPI, InType>(
                                               kTestParam, PPC_SETTINGS_krasnopevtseva_v_connected_component_labeling),
                                           ppc::util::AddFuncTask<KrasnopevtsevaVCCLSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_krasnopevtseva_v_connected_component_labeling));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KrasnopevtsevaVCCLFuncTests::PrintFuncTestName<KrasnopevtsevaVCCLFuncTests>;

INSTANTIATE_TEST_SUITE_P(CCLTests, KrasnopevtsevaVCCLFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace krasnopevtseva_v_connected_component_labeling
