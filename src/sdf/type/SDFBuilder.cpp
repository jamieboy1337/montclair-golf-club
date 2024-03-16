#include "sdf/type/SDFBuilder.hpp"
#include "sdf/CPPSmoother.hpp"

// next job:
// - move height sampler over to chunks
// - create demo using this code?

namespace mgc {
  // tba1: push fairway back a bit further around sand
  // tba2: pad fairway around green a bit more
  SDFBuilder::SDFBuilder(
    const std::shared_ptr<b_fairway>& fairway,
    const std::shared_ptr<b_green>& green,
    const std::shared_ptr<b_sand>& sand
  ) : base_fairway(fairway), base_green(green), base_sand(sand),
      green_cache(nullptr), sand_cache(nullptr), fairway_cache(nullptr) {}

  std::shared_ptr<SDFBuilder::green_type> SDFBuilder::GetGreen() {
    if (green_cache == nullptr) {
      green_cache = base_green;
    }

    return green_cache;
  }

  std::shared_ptr<SDFBuilder::sand_type> SDFBuilder::GetSand() {
    if (sand_cache == nullptr) {
      auto res = smooth::SmootherSSub(
        base_sand,
        base_green,
        15.5
      );

      sand_cache = std::make_shared<SDFBuilder::sand_type>(res);
    }

    // lazy cache?
    return sand_cache;
  }

  std::shared_ptr<SDFBuilder::fairway_type> SDFBuilder::GetFairway() {
    if (fairway_cache == nullptr) {
      auto sand = GetSand();
      auto fairway_carve = std::make_shared<CPPSmoother<b_sand, b_fairway>>(smooth::SmootherSSub(base_fairway, base_sand, 16.0));
      auto fairway_with_green = smooth::SmootherSMin(
        fairway_carve,
        GetGreen(),
        12.5
      );

      fairway_cache = std::make_shared<fairway_type>(std::move(fairway_with_green));
    }

    return fairway_cache;
  }
}
