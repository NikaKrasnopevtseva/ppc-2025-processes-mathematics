#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "krasnopevtseva_v_monte_carlo_integration/common/include/common.hpp"
#include "krasnopevtseva_v_monte_carlo_integration/mpi/include/ops_mpi.hpp"
#include "krasnopevtseva_v_monte_carlo_integration/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace krasnopevtseva_v_monte_carlo_integration {

class KrasnopevtsevaV_MCIntegrationFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    int width = -1;
    int height = -1;
    int channels = -1;
    std::vector<uint8_t> img;
    // Read image
    {
      std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_krasnopevtseva_v_monte_carlo_integration, "pic.jpg");
      auto *data = stbi_load(abs_path.c_str(), &width, &height, &channels, 0);
      if (data == nullptr) {
        throw std::runtime_error("Failed to load image: " + std::string(stbi_failure_reason()));
      }
      img = std::vector<uint8_t>(data, data + (static_cast<ptrdiff_t>(width * height * channels)));
      stbi_image_free(data);
      if (std::cmp_not_equal(width, height)) {
        throw std::runtime_error("width != height: ");
      }
    }

    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = width - height + std::min(std::accumulate(img.begin(), img.end(), 0), channels);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (input_data_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_ = 0;
};

namespace {

TEST_P(KrasnopevtsevaV_MCIntegrationFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {std::make_tuple(3, "3"), std::make_tuple(5, "5"), std::make_tuple(7, "7")};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<KrasnopevtsevaV_MCIntegrationMPI, InType>(kTestParam, PPC_SETTINGS_krasnopevtseva_v_monte_carlo_integration),
                   ppc::util::AddFuncTask<KrasnopevtsevaV_MCIntegrationSEQ, InType>(kTestParam, PPC_SETTINGS_krasnopevtseva_v_monte_carlo_integration));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KrasnopevtsevaV_MCIntegrationFuncTests::PrintFuncTestName<KrasnopevtsevaV_MCIntegrationFuncTests>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, KrasnopevtsevaV_MCIntegrationFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace krasnopevtseva_v_monte_carlo_integration
