#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>

#include "krasnopevtseva_v_bubble_sort/common/include/common.hpp"
#include "krasnopevtseva_v_bubble_sort/mpi/include/ops_mpi.hpp"
#include "krasnopevtseva_v_bubble_sort/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace krasnopevtseva_v_bubble_sort {

class KrasnopevtsevaVBubbleSortFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    const auto &input = std::get<0>(test_param);
    int size = input.size();
    std::string s;
    for (int i = 0; i < size; i++) {
      s += std::to_string(input[i]);
      if (i < size - 1) {
        s += "_";
      }
    }
    std::string result = "array_" + s + "_size" + std::to_string(size) + "_" + std::get<1>(test_param);
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
    int size = output_data.size();
    bool result = true;
    for (int i = 0; i < size - 1; i++) {
      if (output_data[i] > output_data[i + 1]) {
        result = false;
      }
    }
    return result;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

namespace {

TEST_P(KrasnopevtsevaVBubbleSortFuncTests, Bubble_sort) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 5> kTestParam = {
    std::make_tuple(std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, "sorted_array"),
    std::make_tuple(std::vector<int>{5, 1, 3, 4, 2, 34, 4, 24, 16, 31, 666, 22, 14}, "default_array"),
    std::make_tuple(std::vector<int>{10, 1, 30, 2}, "short_array"),
    std::make_tuple(std::vector<int>{10, 31, 120, 4,   2,     1000, 23,    34,   30,  42,  1,   45, 24, 15,  32,  111,
                                     35, 25, 252, 222, 66234, 2325, 23423, 2355, 745, 579, 875, 33, 66, 345, 4666},
                    "long_array"),
    std::make_tuple(std::vector<int>{10, 20, 20, 10, 20, 10, 20, 10, 20}, "two_number_array")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<KrasnopevtsevaVBubbleSortMPI, InType>(kTestParam, PPC_SETTINGS_krasnopevtseva_v_bubble_sort),
    ppc::util::AddFuncTask<KrasnopevtsevaVBubbleSortSEQ, InType>(kTestParam,
                                                                 PPC_SETTINGS_krasnopevtseva_v_bubble_sort));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KrasnopevtsevaVBubbleSortFuncTests::PrintFuncTestName<KrasnopevtsevaVBubbleSortFuncTests>;

INSTANTIATE_TEST_SUITE_P(BubbleSortTests, KrasnopevtsevaVBubbleSortFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace krasnopevtseva_v_bubble_sort
