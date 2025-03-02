#ifndef SEED_GROWER_H_
#define SEED_GROWER_H_

#include "seed/GrowConfig.hpp"
#include "seed/direct/DirectorBase.hpp"

#include "seed/path/SeedPath.hpp"
#include "seed/path/SeedPathBuilder.hpp"

#include "direct/DirectorBase.hpp"

#include <memory>
#include <type_traits>
#include <vector>

#include <random>

namespace mgc {
  /// eyup
  class SeedGrower {
   public:
    typedef std::shared_ptr<SeedPath> path_ptr;
    SeedGrower();

    template <typename DirectorType, class... Args>
    void AddDirector(Args... args) {
      static_assert(std::is_base_of<DirectorBase, DirectorType>::value);
      // create raw ptr
      DirectorType* d = new DirectorType(args...);
      // delegate ownership to this unique ptr instance
      directors.push_back(std::unique_ptr<DirectorBase>(d));
    }

    /**
     * @brief Generates a path
     *
     * @param init_point - start point
     * @param config - configuration for this generations
     * @return std::vector<std::shared_ptr<SeedPath>>
     */
    std::vector<path_ptr> GeneratePaths(
      const glm::dvec2& init_point,
      const GrowConfig& config,
      std::seed_seq& sseq
    ) const;

    // return gradient as sampled/weighted from components. mag >= 1.0.
    glm::dvec2 SampleDirection(const DirectorInfo& info) const;

    // returns gradient after accounting for configuration rigidity. normalized.
    glm::dvec2 SampleDirection_WithRigidity(const DirectorInfo& info, const GrowConfig& config) const;


   private:
    // list of dirs to fetch from
    std::vector<std::unique_ptr<DirectorBase>> directors;

    // engine which powers any random action
    std::mt19937_64 engine;


    /// @brief Recursive path generator
    /// @param builder - builder for this path
    /// @param init_point - point it is starting at
    /// @param init_direction - direction it is pointing in
    /// @param init_energy - initial energy supplied to path
    /// @param global_config - global config
    void GeneratePaths_recurse(
      SeedPathBuilder& builder,
      const glm::dvec2& init_point,
      const glm::dvec2& init_direction,
      double init_energy,
      const GrowConfig& global_config,
      std::mt19937_64& local_engine
    ) const;


    std::uniform_real_distribution<double> len_dist = std::uniform_real_distribution(150.5, 675.0);
  };
}

// ex: dealing with water levels?
// - if we do something ewquiv: water level will be scripted in
// - ...and we'll add some component telling courses to "avoid water" or treat it as a "height floor" when generating terrain

// course gen
// - rationale for SDFs is that bounding boxes are pretty simple to calculate, and determinate
// - math should also (theoretically) be simpler
//   - (capsules will be a PITA for point lists :[)
//
// sdfs are written, seeds are written - where do we go?
// - idea1: create courses greedily along path nodeds
//   - issue: guarantees on course build are a bit looser?
// - idea2: one hole per path node
//   - a bit stiff, and requires a bit more work
// - idea3: virtualize it, just pass the root and let implementer handle it
//   - iterator-like - pass the path, and get back holes
//   - interface: iterator-like, or generate from path all at once?
//   - would like to split up, so that we can thread it somehow?
//   - alt: greedy is necc. iterative
// - idea3.1: generate hole by hole and store - return shared ptr to internal copy
//   - like this the most

// i think for gen: i want to go "path node" by "path node" since we're already handling it
// - retool iterator to return paths, instead of points
// - step down each path to build courses
// tba:
// - virtual call to generate a single hole
// - return ptr if generated, otherwise nullptr
// - generate hole-by-hole for the sake of flexibility i'm thinking
//
// hole spec
// - go with an sdf in each case
//   - issue: we have two sdf types and no unifying interface
// - thinking: we could just vir wrap it?? might be costly
// - alt2: type it as-needed
// - alt3: template the featurebox - we'll put it into a splatwriter anyway
// better: each "feature" is just a box spec
// - doesn't really work for mixing bits? at least for doing smoothing
//
// - type and forget:
//   - for splats: doesn't matter, we just want to plug them into a manager
//   - for height: just need it to fulfill height spec
//
// arrangement
// - height: everything gets added together
// - splat: maintain a splat manager for each hole - plug our SDFs into an impl on ctor
//   - need to know green indices beforehand
// - tree: figure out down the line
//
// result
// - return a smoothing sampler box containing course data
// - same pos as the starter with our sdf
// generic should store
// - course path
// - Smooth for fairway
//   - carve sand bundle out of fairway
// - Bundle for green
// - Bundle for bunker
// who should handle conversion?
// - I think our linking logic should do it
// - hole box: wrap up any terrain types (grass, bunker, dirt, etc)
// - converter: map them to splat indices and wrap as samplers
// hows smooth going to work?
// - in almost all cases: want to smooth together a whole bunch of things
// - see a guy who merges them one-by-one (i think that works, yes it def does)

// as long as we can smin at compile time, i'm fine with that
// (tba: "smoothing bundle" to smooth every element in a bundle by some factor K?)
// https://www.shadertoy.com/view/ws3XRf
// just make it a slightly different "bundle" sampler which smins after every sample

// eff: "holes" are sampling types
// - i think we want to "temporarily" store anything as a virt, and then convert down the line
// - ie: working in "hole space" then converting to global space - makes more sense to me

#endif // SEED_GROWER_H_
