#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <random>
#include <tuple>
#include <vector>

#include "krasnopevtseva_v_connected_component_labeling/common/include/common.hpp"
#include "krasnopevtseva_v_connected_component_labeling/mpi/include/ops_mpi.hpp"
#include "krasnopevtseva_v_connected_component_labeling/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace krasnopevtseva_v_connected_component_labeling {

class KrasnopevtsevaVCCLPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    int height = 5000;
    int width = 5000;
    std::vector<int> binary_data(height * width, 0);

    for (int y = 2000; y < 3000; ++y) {
      for (int x = 2000; x < 3000; ++x) {
        binary_data[y * width + x] = 1;
      }
    }

    for (int x = 0; x < width; ++x) {
      binary_data[100 * width + x] = 1;
    }

    for (int y = 0; y < height; ++y) {
      binary_data[y * width + 100] = 1;
    }

    binary_data[0] = 1;
    binary_data[width - 1] = 1;
    binary_data[(height - 1) * width] = 1;
    binary_data[(height - 1) * width + width - 1] = 1;

    input_data_ = std::make_tuple(height, width, binary_data);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &[height, width, input_binary] = input_data_;
    int total_pixels = height * width;

    if (output_data.size() != static_cast<size_t>(total_pixels)) {
      return false;
    }

    if (input_binary[0] == 1 && output_data[0] <= 0) {
      return false;
    }
    if (input_binary[width - 1] == 1 && output_data[width - 1] <= 0) {
      return false;
    }
    if (input_binary[(height - 1) * width] == 1 && output_data[(height - 1) * width] <= 0) {
      return false;
    }
    if (input_binary[(height - 1) * width + width - 1] == 1 && output_data[(height - 1) * width + width - 1] <= 0) {
      return false;
    }

    int center_y = 2500;
    int center_x = 2500;
    int center_idx = center_y * width + center_x;

    if (input_binary[center_idx] != 1) {
      return false;
    }
    if (output_data[center_idx] <= 0) {
      return false;
    }

    int center_label = output_data[center_idx];

    for (int dy = -1; dy <= 1; ++dy) {
      for (int dx = -1; dx <= 1; ++dx) {
        int y = center_y + dy;
        int x = center_x + dx;
        if (y >= 2000 && y < 3000 && x >= 2000 && x < 3000) {
          int idx = y * width + x;
          if (input_binary[idx] == 1 && output_data[idx] != center_label) {
            return false;
          }
        }
      }
    }

    int line_y = 100;
    int line_label = output_data[line_y * width];

    for (int x = 0; x < width; x += 500) {
      int idx = line_y * width + x;
      if (input_binary[idx] == 1 && output_data[idx] != line_label) {
        return false;
      }
    }

    int line_x = 100;
    int vertical_label = output_data[line_x];

    for (int y = 0; y < height; y += 500) {
      int idx = y * width + line_x;
      if (input_binary[idx] == 1 && output_data[idx] != vertical_label) {
        return false;
      }
    }

    int intersection_idx = line_y * width + line_x;
    if (input_binary[intersection_idx] == 1) {
      if (line_label != vertical_label) {
        return false;
      }
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(KrasnopevtsevaVCCLPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, KrasnopevtsevaVCCLMPI, KrasnopevtsevaVCCLSEQ>(
    PPC_SETTINGS_krasnopevtseva_v_connected_component_labeling);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KrasnopevtsevaVCCLPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KrasnopevtsevaVCCLPerfTests, kGtestValues, kPerfTestName);

}  // namespace krasnopevtseva_v_connected_component_labeling
