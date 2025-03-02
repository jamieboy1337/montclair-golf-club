#ifndef HOLE_CHUNK_BOX_H_
#define HOLE_CHUNK_BOX_H_

#include <corrugate/FeatureBox.hpp>
#include <corrugate/MultiSampler.hpp>

#include "seed/hole/HoleBox.hpp"

#include "util/GC_AABB.hpp"

namespace mgc {

  class HoleChunkBox : public cg::FeatureBox {
   public:
    typedef std::vector<std::shared_ptr<const HoleBox>>::const_iterator iterator;
    HoleChunkBox(const std::vector<HoleBox>& holes, const glm::ivec2& chunk);

    iterator begin() const { return holes_ordered.begin(); }
    iterator end()   const { return holes_ordered.end();   }

    size_t chunk_count() const { return holes.size(); }

    // receive point in global space - return true if within padding range of holes
    // fiugh
    bool Test(const glm::dvec2& point) const;


    const glm::ivec2 chunk;
    // z index equiv - for sdf gen
    // how to gen?
    // - pass in
    // - hash from chunk index
    const size_t priority_hash;
   private:
    // priv call - receives pre-calculated origin and size
    HoleChunkBox(const GC_AABB& bb, const std::vector<HoleBox>& holes, const glm::ivec2& chunk);
    // contains holes, in global space
    static GC_AABB GetBoundingBox(const std::vector<HoleBox>& holes);
    cg::MultiSampler<HoleBox> holes;
    // ensure holes remain ordered
    std::vector<std::shared_ptr<const HoleBox>> holes_ordered;
    // thinking: pair this up with a side-by-side vector??
  };
}

#endif // HOLE_CHUNK_BOX_H_
