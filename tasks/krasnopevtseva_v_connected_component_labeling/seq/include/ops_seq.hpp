#pragma once

#include <array>
#include <queue>
#include <utility>
#include <vector>

#include "krasnopevtseva_v_connected_component_labeling/common/include/common.hpp"
#include "task/include/task.hpp"

namespace krasnopevtseva_v_connected_component_labeling {

class KrasnopevtsevaVCCLSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KrasnopevtsevaVCCLSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  static void ProcessImage(const std::vector<int> &binary_data, std::vector<int> &output, int height, int width);
  static bool IsUnlabeledPixel(const std::vector<int> &binary_data, const std::vector<int> &output, int index);
  static void LabelConnectedComponent(const std::vector<int> &binary_data, std::vector<int> &output, int start_row,
                                      int start_col, int label, const std::array<std::pair<int, int>, 4> &directions,
                                      int height, int width);
  static void ExploreNeighbors(const std::vector<int> &binary_data, std::vector<int> &output, int current_row,
                               int current_col, int label, const std::array<std::pair<int, int>, 4> &directions,
                               int height, int width, std::queue<std::pair<int, int>> &queue);
  static bool IsValidPosition(int row, int col, int height, int width);
};

}  // namespace krasnopevtseva_v_connected_component_labeling
