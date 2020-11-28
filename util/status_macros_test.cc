#include "libopendrop/util/status_macros.h"

#include "googletest/include/gtest/gtest.h"

// Intentionally test this outside of the `absl` namespace so as to assert that
// the macros qualify the namespaces correctly.
namespace {
namespace foobar {

TEST(StatusMacrosTest, ReturnIfErrorReturnsOnError) {
  auto return_if_error_function = [](absl::Status status) {
    RETURN_IF_ERROR(status);
    return absl::OkStatus();
  };

  ASSERT_TRUE(return_if_error_function(absl::OkStatus()).ok());
  EXPECT_FALSE(
      return_if_error_function(absl::FailedPreconditionError("")).ok());
}

TEST(StatusMacrosTest, CanProvideDeclarationInRhsOfAssignOrReturn) {
  auto return_if_error_function =
      [](absl::StatusOr<int> status) -> absl::Status {
    ASSIGN_OR_RETURN(auto foo, status);
    static_assert(std::is_same<decltype(foo), int>::value, "");
    return absl::OkStatus();
  };

  ASSERT_TRUE(return_if_error_function(1).ok());
  ASSERT_FALSE(
      return_if_error_function(absl::FailedPreconditionError("")).ok());
}

struct TestCase {
  int initial_value;
  absl::StatusOr<int> status;
  int test_value;
  bool ok;
};

class StatusMacrosParameterizedTest
    : public ::testing::TestWithParam<TestCase> {};
TEST_P(StatusMacrosParameterizedTest, AssignOrReturnOnlyAssignsForOkStatus) {
  int i = GetParam().initial_value;

  auto assign_or_return_function = [&]() {
    ASSIGN_OR_RETURN(i, GetParam().status);
    return absl::OkStatus();
  };

  ASSERT_EQ(assign_or_return_function().ok(), GetParam().ok);
  EXPECT_EQ(i, GetParam().test_value);
}

INSTANTIATE_TEST_SUITE_P(
    AssignOrReturnTestSuite, StatusMacrosParameterizedTest,
    ::testing::Values(TestCase{-1234, 5, 5, true},
                      TestCase{-1234, absl::FailedPreconditionError(""), -1234,
                               false}));

}  // namespace foobar
}  // namespace
