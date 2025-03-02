#include "mgc/CourseGen.hpp"

#include "course/sampler/gaussian/GaussianConfig.hpp"
#include "course/sampler/gaussian/GaussianFairwaySampler.hpp"
#include "course/sampler/gaussian/GaussianSandSampler.hpp"

#include "course/sampler/SimplexNoiseSampler.hpp"

#include "course/generator/BruteForceCourseGenerator.hpp"

#include "course/path/CoursePathToCurve.hpp"

using namespace course;
using namespace sampler;
using namespace gaussian;
using namespace generator;

// need a lil bit of logic to establish bounds on the course itself (for course render)

namespace mgc {
  CourseGen::CourseGen(
    uint64_t seed
  ) {
    course_sampler = std::make_shared<GaussianMetaballSampler>();

    auto feature_sampler = std::make_shared<GaussianMetaballSampler>();

    GaussianPathConfig config;

    config.scatter_config.density = 6;
    // tba: can probably do this better, ie actually marching the gradient
    config.scatter_config.mean_distance = 56.0;
    config.scatter_config.intensity = 0.4;

    // throwaway rn
    std::shared_ptr<sampler::SimplexNoiseSampler> terrainsampler = std::make_shared<sampler::SimplexNoiseSampler>(glm::vec3(1.0), 4);

    BruteForceCourseGenerator gen;
    gen.seed = seed;
    gen.yardage = 525.0f;
    path = gen.GenerateCourse(terrainsampler, glm::vec2(1024, 1024));
    path.Recenter(glm::vec2(0, 0), glm::vec2(1024, 1024));

    // path.Recenter(glm::vec2(0, 0), glm::vec2(1024, 1024));

    curve = CoursePathToCurve(path, 0.5);
    auto fairway_sampler = std::make_shared<GaussianFairwaySampler>(
      path,
      curve
    );

    // - path is bvad
    // - sampling range is wrong
    // -

    fairway_sampler->Generate(config);

    auto sand_sampler = std::make_shared<GaussianSandSampler>(
      fairway_sampler,
      path,
      curve
    );

    sand_sampler->Generate(config);

    course_sampler->Merge(*fairway_sampler, 1.0);
    course_sampler->Merge(*sand_sampler, -1.0);

    feature_sampler->Merge(*fairway_sampler, 1.0);
    feature_sampler->Merge(*sand_sampler, 1.0);

    tree_sampler = std::make_shared<FillSamplerType>(*feature_sampler, -3.0, -55.0);


    this->fairway_sampler = std::make_shared<ThresholdSampler<GaussianMetaballSampler>>(0.4, 16.0, 0.1, course_sampler);
    this->green_sampler = std::make_shared<ThresholdSampler<GaussianMetaballSampler>>(15.9, 1000.0, 0.2, course_sampler);
    auto sand_threshold = std::make_shared<ThresholdSampler<GaussianMetaballSampler>>(0.2, 500.0, 0.2, std::dynamic_pointer_cast<GaussianMetaballSampler>(sand_sampler));
    auto course_threshold = std::make_shared<ThresholdSampler<GaussianMetaballSampler>>(-1000.0, -0.1, 0.05, course_sampler);
    this->sand_sampler = std::make_shared<ArithmeticSampler<ThresholdSampler<GaussianMetaballSampler>, ThresholdSampler<GaussianMetaballSampler>>>(sand_threshold, course_threshold);
    this->rough_sampler = std::make_shared<ThresholdSampler<GaussianMetaballSampler>>(-1000.4, 1000.0, 0.1, course_sampler);
    // decrease intensity to affect sampling - ensure hazards override it
    this->fairway_sampler->intensity = 1.0;
    this->rough_sampler->intensity = 0.2;

    std::mt19937_64 engine;
    engine.seed(142758UL);
    std::uniform_real_distribution<double> test(-32768.0, 32768.0);

    base_terrain = std::make_shared<gdterrain::CourseSmoother<_impl::SimplexHeightFunc, _impl::SimplexHeightFunc, _impl::SimplexHeightFunc>>(
      std::make_shared<_impl::SimplexHeightFunc>(2, 4.0, 8.0),
      std::make_shared<_impl::SimplexHeightFunc>(2, 4.0, 8.0),
      std::make_shared<_impl::SimplexHeightFunc>(2, 4.0, 8.0),
      feature_sampler,
      curve
    );

    // this should be it! just need to define the helpers
  }

  // when i wake up

  CourseGen::CourseTerrainPtr CourseGen::GetFairwaySampler() { return fairway_sampler; }
  CourseGen::CourseTerrainPtr CourseGen::GetGreenSampler() { return green_sampler; }
  CourseGen::SandSampler      CourseGen::GetSandSampler() { return sand_sampler; }
  CourseGen::CourseTerrainPtr CourseGen::GetRoughSampler() { return rough_sampler; }
  std::shared_ptr<CourseGen::HeightMapType> CourseGen::GetHeightMap() { return std::make_shared<CourseGen::HeightMapType>(*base_terrain); }   // tba: return unique ptr instead???
  std::shared_ptr<CourseGen::FillSamplerType>  CourseGen::GetTreeFillSampler() { return tree_sampler; }

  // return a tree fill map??

  glm::dvec2 CourseGen::GetCourseOrigin() {
    return base_terrain->GetCourseOrigin();
  }

  glm::dvec2 CourseGen::GetCourseSize() {
    // tba: need some tests to confirm that these numbers are roughly correct
    return base_terrain->GetCourseSize();
  }

  glm::dvec3 CourseGen::GetCourseCenter() {
    return base_terrain->GetCourseCenter();

  }

  course::path::CoursePath CourseGen::GetCoursePath() { return path; }
  course::path::CompoundCurve CourseGen::GetCourseCurve() { return curve; }
}
