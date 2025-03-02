#ifndef CPP_CAPSULE_H_
#define CPP_CAPSULE_H_

#include <glm/glm.hpp>

#include <vector>

namespace mgc {
  class CPPCapsule {
   public:
    CPPCapsule();

    CPPCapsule(
      const std::vector<glm::dvec2>& elements,
      double radius
    );

    CPPCapsule(
      const std::vector<glm::dvec2>& elements,
      const std::vector<double>& radii
    );

    double Sample(double x, double y) const;
    double Sample(const glm::dvec2& point) const;

    double Sample(const glm::dvec3& point) const;

    glm::dvec4 GetBoundingBox() const;
   private:
    inline static double SampleCapsule(
      const glm::dvec2& point,
      const glm::dvec2& pa,
      const double& radius_a,
      const glm::dvec2& pb,
      const double& radius_b
    );

    inline static double SampleCapsule3D(
      const glm::dvec3& point,
      const glm::dvec3& pa,
      const double& radius_a,
      const glm::dvec3& pb,
      const double& radius_b
    );

    const std::vector<glm::dvec2> elements;
    const std::vector<double> radii;
  };
}

#endif // CPP_CAPSULE_H_
