#pragma once

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
};

}  // namespace krasnopevtseva_v_connected_component_labeling
