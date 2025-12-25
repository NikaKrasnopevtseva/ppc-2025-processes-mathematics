#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
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
    std::vector<int> binary_data(static_cast<size_t>(height) * static_cast<size_t>(width), 0);

    for (int row = 2000; row < 3000; ++row) {
      for (int col = 2000; col < 3000; ++col) {
        binary_data[static_cast<size_t>(row) * static_cast<size_t>(width) + static_cast<size_t>(col)] = 1;
      }
    }

    for (int col = 0; col < width; ++col) {
      binary_data[static_cast<size_t>(100) * static_cast<size_t>(width) + static_cast<size_t>(col)] = 1;
    }

    for (int row = 0; row < height; ++row) {
      binary_data[static_cast<size_t>(row) * static_cast<size_t>(width) + static_cast<size_t>(100)] = 1;
    }

    binary_data[0] = 1;
    binary_data[width - 1] = 1;
    binary_data[static_cast<size_t>(height - 1) * static_cast<size_t>(width)] = 1;
    binary_data[static_cast<size_t>(height - 1) * static_cast<size_t>(width) + static_cast<size_t>(width - 1)] = 1;

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
    const size_t bottom_left_idx = static_cast<size_t>(height - 1) * static_cast<size_t>(width);
    if (input_binary[bottom_left_idx] == 1 && output_data[bottom_left_idx] <= 0) {
      return false;
    }
    const size_t bottom_right_idx =
        static_cast<size_t>(height - 1) * static_cast<size_t>(width) + static_cast<size_t>(width - 1);
    if (input_binary[bottom_right_idx] == 1 && output_data[bottom_right_idx] <= 0) {
      return false;
    }

    int center_row = 2500;
    int center_col = 2500;
    const size_t center_idx =
        static_cast<size_t>(center_row) * static_cast<size_t>(width) + static_cast<size_t>(center_col);

    if (input_binary[center_idx] != 1) {
      return false;
    }
    if (output_data[center_idx] <= 0) {
      return false;
    }

    int center_label = output_data[center_idx];

    for (int delta_row = -1; delta_row <= 1; ++delta_row) {
      for (int delta_col = -1; delta_col <= 1; ++delta_col) {
        int row = center_row + delta_row;
        int col = center_col + delta_col;
        if (row >= 2000 && row < 3000 && col >= 2000 && col < 3000) {
          const size_t idx = static_cast<size_t>(row) * static_cast<size_t>(width) + static_cast<size_t>(col);
          if (input_binary[idx] == 1 && output_data[idx] != center_label) {
            return false;
          }
        }
      }
    }

    int line_row = 100;
    const size_t line_start_idx = static_cast<size_t>(line_row) * static_cast<size_t>(width);
    int line_label = output_data[line_start_idx];

    for (int col = 0; col < width; col += 500) {
      const size_t idx = static_cast<size_t>(line_row) * static_cast<size_t>(width) + static_cast<size_t>(col);
      if (input_binary[idx] == 1 && output_data[idx] != line_label) {
        return false;
      }
    }

    int line_col = 100;
    int vertical_label = output_data[line_col];

    for (int row = 0; row < height; row += 500) {
      const size_t idx = static_cast<size_t>(row) * static_cast<size_t>(width) + static_cast<size_t>(line_col);
      if (input_binary[idx] == 1 && output_data[idx] != vertical_label) {
        return false;
      }
    }

    const size_t intersection_idx =
        static_cast<size_t>(line_row) * static_cast<size_t>(width) + static_cast<size_t>(line_col);
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
