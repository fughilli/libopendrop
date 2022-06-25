#include "util/graph/types/opaque_storable.h"

#include <memory>

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googlemock/include/gmock/gmock-more-matchers.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

enum ConstructionState {
  kUnconstructed,
  kConstructed,
  kReconstructed,
  kDestructed,
};

ConstructionState construction_state = ConstructionState::kUnconstructed;

struct RaiiType : public OpaqueStorable<RaiiType> {
  RaiiType() {
    if (construction_state == ConstructionState::kConstructed) {
      construction_state = ConstructionState::kReconstructed;
    } else {
      construction_state = ConstructionState::kConstructed;
    }
    refcount_member = std::make_shared<int>(5);
  }
  ~RaiiType() { construction_state = ConstructionState::kDestructed; }

  int member_with_default = 123;
  std::shared_ptr<int> refcount_member;
};

TEST(OpaqueStorableTest, OpaqueStorableInvokesConstructorAndDestructor) {
  construction_state = ConstructionState::kUnconstructed;

  ASSERT_EQ(construction_state, ConstructionState::kUnconstructed);
  std::shared_ptr<int> handle = nullptr;
  {
    std::shared_ptr<uint8_t> value = RaiiType::Allocate();
    RaiiType& ref = *reinterpret_cast<RaiiType*>(value.get());
    EXPECT_EQ(construction_state, ConstructionState::kConstructed);
    EXPECT_EQ(ref.member_with_default, 123);
    EXPECT_EQ(construction_state, ConstructionState::kConstructed);
    EXPECT_EQ(ref.refcount_member.use_count(), 1);
    EXPECT_EQ(construction_state, ConstructionState::kConstructed);
    handle = ref.refcount_member;
    EXPECT_EQ(ref.refcount_member.use_count(), 2);
    EXPECT_EQ(construction_state, ConstructionState::kConstructed);
  }
  EXPECT_EQ(handle.use_count(), 1);
  EXPECT_EQ(construction_state, ConstructionState::kDestructed);
}

}  // namespace
}  // namespace opendrop
