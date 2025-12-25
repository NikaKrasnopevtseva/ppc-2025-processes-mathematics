#pragma once

#include <queue>
#include <vector>

#include "krasnopevtseva_v_connected_component_labeling/common/include/common.hpp"
#include "task/include/task.hpp"

namespace krasnopevtseva_v_connected_component_labeling {

struct Point {
  int x, y;
};

class KrasnopevtsevaVCCLMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KrasnopevtsevaVCCLMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void MPIBfs(int *p_local_image, int local_pixel_count, int *p_local_labels, int curr_label, int local_rows);
  [[nodiscard]] static bool IsValidMPI(int nr, int nc, int local_rows, int n_tmp);
  void BFSCheck(const int *p_local_image, int curr_label, int *p_local_labels, int local_rows, Point cp,
                std::queue<Point> &bfs_queue, int n_tmp) const;
  [[nodiscard]] static std::vector<int> MakeMPIResult(const std::vector<int> &global_labels, int m_tmp, int n_tmp);
};
}  // namespace krasnopevtseva_v_connected_component_labeling
