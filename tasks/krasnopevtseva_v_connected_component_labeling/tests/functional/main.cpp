#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <queue>
#include <tuple>
#include <unordered_map>
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

 protected:
  void SetUp() override {
    auto test_param = std::get<static_cast<size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(test_param);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &[height, width, input_binary] = input_data_;
    int total_pixels = height * width;

    if (output_data.size() != static_cast<size_t>(total_pixels)) {
      return false;
    }

    for (int i = 0; i < total_pixels; ++i) {
      if (input_binary[i] == 0 && output_data[i] != 0) {
        return false;
      }
      if (input_binary[i] == 1 && output_data[i] <= 0) {
        return false;
      }
    }

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        int idx = (y * width) + x;
        if (input_binary[idx] == 1) {
          if (x > 0 && input_binary[idx - 1] == 1) {
            if (output_data[idx - 1] != output_data[idx]) {
              return false;
            }
          }
          if (x < width - 1 && input_binary[idx + 1] == 1) {
            if (output_data[idx + 1] != output_data[idx]) {
              return false;
            }
          }
          if (y > 0 && input_binary[idx - width] == 1) {
            if (output_data[idx - width] != output_data[idx]) {
              return false;
            }
          }
          if (y < height - 1 && input_binary[idx + width] == 1) {
            if (output_data[idx + width] != output_data[idx]) {
              return false;
            }
          }
        }
      }
    }

    std::unordered_map<int, std::vector<int>> label_to_indices;
    for (int i = 0; i < total_pixels; ++i) {
      if (output_data[i] > 0) {
        label_to_indices[output_data[i]].push_back(i);
      }
    }

    for (const auto &[label, indices] : label_to_indices) {
      if (indices.empty()) {
        continue;
      }

      std::vector<bool> visited(total_pixels, false);
      std::queue<int> q;

      q.push(indices[0]);
      visited[indices[0]] = true;
      int visited_count = 1;

      while (!q.empty()) {
        int current = q.front();
        q.pop();
        int y = current / width;
        int x = current % width;

        const std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (const auto &[dy, dx] : directions) {
          int ny = y + dy;
          int nx = x + dx;

          if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
            int nidx = (ny * width) + nx;
            if (!visited[nidx] && output_data[nidx] == label) {
              visited[nidx] = true;
              visited_count++;
              q.push(nidx);
            }
          }
        }
      }

      if (visited_count != static_cast<int>(indices.size())) {
        return false;
      }
    }

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        int idx = (y * width) + x;
        if (input_binary[idx] == 0) {
          continue;
        }
        int current_label = output_data[idx];
        const std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (const auto &[dy, dx] : directions) {
          int ny = y + dy;
          int nx = x + dx;
          if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
            int nidx = (ny * width) + nx;
            if (input_binary[nidx] == 1 && output_data[nidx] != current_label) {
              return false;
            }
          }
        }
      }
    }

    std::vector<int> all_labels;
    for (int val : output_data) {
      if (val > 0) {
        all_labels.push_back(val);
      }
    }

    std::sort(all_labels.begin(), all_labels.end());
    all_labels.erase(std::unique(all_labels.begin(), all_labels.end()), all_labels.end());

    if (!all_labels.empty()) {
      if (all_labels[0] != 1) {
        return false;
      }

      for (size_t i = 1; i < all_labels.size(); ++i) {
        if (all_labels[i] != all_labels[i - 1] + 1) {
          return false;
        }
      }
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
  std::make_tuple(make_tuple(3, 3, std::vector<int>{
    0, 0, 0,
    0, 0, 0,
    0, 0, 0
  }), "empty_3x3"),
  
  std::make_tuple(make_tuple(3, 3, std::vector<int>{
    0, 0, 0,
    0, 1, 0,
    0, 0, 0
  }), "single_pixel"),
  
  std::make_tuple(make_tuple(4, 4, std::vector<int>{
    1, 1, 0, 0,
    1, 1, 0, 0,
    0, 0, 1, 1,
    0, 0, 1, 1
  }), "two_components"),
  
  std::make_tuple(make_tuple(10, 10, std::vector<int>{
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
  
  std::make_tuple(make_tuple(20, 30, []() {
    std::vector<int> data(20 * 30, 0);
    for (int y = 0; y < 20; y++) {
      for (int x = 0; x < 30; x++) {
        if (x % 5 == 0) {  
          data[y * 30 + x] = 1;
        }
      }
    }
    return data;
  }()), "vertical_lines_20x30"),
  
  std::make_tuple(make_tuple(30, 20, []() {
    std::vector<int> data(30 * 20, 0);
    for (int y = 0; y < 30; y++) {
      for (int x = 0; x < 20; x++) {
        if (y % 5 == 0) {  
          data[y * 20 + x] = 1;
        }
      }
    }
    return data;
  }()), "horizontal_lines_30x20"),
  
  std::make_tuple(make_tuple(8, 8, []() {
    std::vector<int> data(64, 0);
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if ((x + y) % 2 == 0) {
          data[y * 8 + x] = 1;
        }
      }
    }
    return data;
  }()), "chessboard_8x8"),
  
  std::make_tuple(make_tuple(15, 15, std::vector<int>(225, 1)), "full_15x15"),
  
  std::make_tuple(make_tuple(17, 25, []() {
    std::vector<int> data(17 * 25, 0);
    for (int y = 5; y < 12; y++) {
      for (int x = 8; x < 17; x++) {
        data[y * 25 + x] = 1;
      }
    }
    return data;
  }()), "cross_boundary_17x25"),
  
  std::make_tuple(make_tuple(25, 25, []() {
    std::vector<int> data(625, 0);
    for (int i = 0; i < 25; i++) {
      data[i * 25 + i] = 1;
    }
    return data;
  }()), "big_diagonal_25x25"),
  
  std::make_tuple(make_tuple(20, 20, []() {
    std::vector<int> data(400, 0);
    int points[] = {2, 5, 8, 12, 15, 18};
    for (int y : points) {
      for (int x : points) {
        if (y * 20 + x < 400) {
          data[y * 20 + x] = 1;
        }
      }
    }
    return data;
  }()), "scattered_points_20x20"),
  
  std::make_tuple(make_tuple(8, 8, std::vector<int>{
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
