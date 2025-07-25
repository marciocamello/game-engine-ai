#pragma once

#include "Graphics/BoundingVolumes.h"
#include "Core/Math.h"
#include <vector>
#include <memory>

namespace GameEngine {
    class Mesh;
    class ModelNode;

    /**
     * Utility class for advanced bounding volume calculations
     * Implements various algorithms for optimal bounding volume generation
     */
    class BoundingVolumeCalculator {
    public:
        // Axis-aligned bounding box calculations
        static BoundingBox CalculateAABB(const std::vector<Math::Vec3>& points);
        static BoundingBox CalculateAABBFromMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes);
        static BoundingBox CalculateHierarchicalAABB(std::shared_ptr<ModelNode> rootNode, 
                                                     const std::vector<std::shared_ptr<Mesh>>& meshes);

        // Bounding sphere calculations
        static BoundingSphere CalculateNaiveSphere(const std::vector<Math::Vec3>& points);
        static BoundingSphere CalculateRitterSphere(const std::vector<Math::Vec3>& points);
        static BoundingSphere CalculateWelzlSphere(const std::vector<Math::Vec3>& points);
        static BoundingSphere CalculateOptimalSphere(const std::vector<Math::Vec3>& points);
        static BoundingSphere CalculateSphereFromMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes);
        static BoundingSphere CalculateHierarchicalSphere(std::shared_ptr<ModelNode> rootNode,
                                                          const std::vector<std::shared_ptr<Mesh>>& meshes);

        // Utility methods
        static std::vector<Math::Vec3> ExtractVertexPositions(const std::vector<std::shared_ptr<Mesh>>& meshes);
        static BoundingBox TransformBoundingBox(const BoundingBox& box, const Math::Mat4& transform);
        static BoundingSphere TransformBoundingSphere(const BoundingSphere& sphere, const Math::Mat4& transform);

        // Validation and analysis
        static bool ValidateBoundingVolume(const BoundingBox& box, const std::vector<Math::Vec3>& points);
        static bool ValidateBoundingVolume(const BoundingSphere& sphere, const std::vector<Math::Vec3>& points);
        static float CalculateBoundingVolumeEfficiency(const BoundingSphere& sphere, const std::vector<Math::Vec3>& points);

        // Animated bounding volume support
        static BoundingBox InterpolateBoundingBox(const BoundingBox& box1, const BoundingBox& box2, float t);
        static BoundingSphere InterpolateBoundingSphere(const BoundingSphere& sphere1, const BoundingSphere& sphere2, float t);
        static std::vector<BoundingBox> GenerateAnimatedBoundingBoxes(const std::vector<std::vector<Math::Vec3>>& animatedVertices);
        static std::vector<BoundingSphere> GenerateAnimatedBoundingSpheres(const std::vector<std::vector<Math::Vec3>>& animatedVertices);
        
        // Culling and collision queries
        static bool IsPointInBoundingBox(const Math::Vec3& point, const BoundingBox& box);
        static bool IsPointInBoundingSphere(const Math::Vec3& point, const BoundingSphere& sphere);
        static bool DoBoundingBoxesIntersect(const BoundingBox& box1, const BoundingBox& box2);
        static bool DoBoundingSpheresIntersect(const BoundingSphere& sphere1, const BoundingSphere& sphere2);

    private:
        // Helper methods for Welzl algorithm
        static BoundingSphere WelzlRecursive(std::vector<Math::Vec3>& points, std::vector<Math::Vec3>& boundary, size_t n);
        static BoundingSphere SphereFromPoints(const std::vector<Math::Vec3>& points);
        static BoundingSphere SphereFrom2Points(const Math::Vec3& p1, const Math::Vec3& p2);
        static BoundingSphere SphereFrom3Points(const Math::Vec3& p1, const Math::Vec3& p2, const Math::Vec3& p3);
        static bool PointInSphere(const Math::Vec3& point, const BoundingSphere& sphere, float epsilon = 1e-6f);
    };
}