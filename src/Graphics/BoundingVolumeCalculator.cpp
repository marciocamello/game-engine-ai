#include "Graphics/BoundingVolumeCalculator.h"
#include "Graphics/Mesh.h"
#include "Graphics/ModelNode.h"
#include "Core/Logger.h"
#include <algorithm>
#include <random>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GameEngine {

    BoundingBox BoundingVolumeCalculator::CalculateAABB(const std::vector<Math::Vec3>& points) {
        if (points.empty()) {
            return BoundingBox();
        }

        BoundingBox box;
        for (const auto& point : points) {
            box.Expand(point);
        }
        return box;
    }

    BoundingBox BoundingVolumeCalculator::CalculateAABBFromMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) {
        BoundingBox combinedBox;
        
        for (const auto& mesh : meshes) {
            if (mesh) {
                combinedBox.Expand(mesh->GetBoundingBox());
            }
        }
        
        return combinedBox;
    }

    BoundingBox BoundingVolumeCalculator::CalculateHierarchicalAABB(std::shared_ptr<ModelNode> rootNode,
                                                                   const std::vector<std::shared_ptr<Mesh>>& meshes) {
        if (!rootNode) {
            return BoundingBox();
        }

        BoundingBox hierarchicalBox;

        // Traverse the node hierarchy and accumulate bounds
        rootNode->Traverse([&](std::shared_ptr<ModelNode> node) {
            if (!node) return;

            // Get bounds from meshes associated with this node
            BoundingBox nodeBounds = node->CalculateCombinedBounds(meshes);
            
            // Transform bounds to world space
            if (nodeBounds.IsValid()) {
                BoundingBox worldBounds = nodeBounds.Transform(node->GetWorldTransform());
                hierarchicalBox.Expand(worldBounds);
            }
        });

        return hierarchicalBox;
    }

    BoundingSphere BoundingVolumeCalculator::CalculateNaiveSphere(const std::vector<Math::Vec3>& points) {
        if (points.empty()) {
            return BoundingSphere();
        }

        // Calculate center as average of all points
        Math::Vec3 center(0.0f);
        for (const auto& point : points) {
            center += point;
        }
        center /= static_cast<float>(points.size());

        // Find maximum distance from center
        float maxRadius = 0.0f;
        for (const auto& point : points) {
            float distance = glm::length(point - center);
            maxRadius = std::max(maxRadius, distance);
        }

        return BoundingSphere(center, maxRadius);
    }

    BoundingSphere BoundingVolumeCalculator::CalculateRitterSphere(const std::vector<Math::Vec3>& points) {
        if (points.empty()) {
            return BoundingSphere();
        }

        if (points.size() == 1) {
            return BoundingSphere(points[0], 0.0f);
        }

        // Find the two points that are farthest apart
        float maxDistance = 0.0f;
        size_t point1Index = 0, point2Index = 1;

        for (size_t i = 0; i < points.size(); ++i) {
            for (size_t j = i + 1; j < points.size(); ++j) {
                float distance = glm::length(points[i] - points[j]);
                if (distance > maxDistance) {
                    maxDistance = distance;
                    point1Index = i;
                    point2Index = j;
                }
            }
        }

        // Initial sphere from the two farthest points
        Math::Vec3 center = (points[point1Index] + points[point2Index]) * 0.5f;
        float radius = maxDistance * 0.5f;
        BoundingSphere sphere(center, radius);

        // Expand sphere to include all other points
        for (const auto& point : points) {
            sphere.Expand(point);
        }

        return sphere;
    }

    BoundingSphere BoundingVolumeCalculator::CalculateWelzlSphere(const std::vector<Math::Vec3>& points) {
        if (points.empty()) {
            return BoundingSphere();
        }

        // Create a copy of points for the recursive algorithm
        std::vector<Math::Vec3> pointsCopy = points;
        std::vector<Math::Vec3> boundary;

        // Shuffle points for better average performance
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(pointsCopy.begin(), pointsCopy.end(), g);

        return WelzlRecursive(pointsCopy, boundary, pointsCopy.size());
    }

    BoundingSphere BoundingVolumeCalculator::WelzlRecursive(std::vector<Math::Vec3>& points, 
                                                           std::vector<Math::Vec3>& boundary, 
                                                           size_t n) {
        // Base cases
        if (n == 0 || boundary.size() == 4) {
            return SphereFromPoints(boundary);
        }

        // Take the last point
        Math::Vec3 p = points[n - 1];

        // Recursively find sphere without this point
        BoundingSphere sphere = WelzlRecursive(points, boundary, n - 1);

        // If point is inside sphere, return the sphere
        if (PointInSphere(p, sphere)) {
            return sphere;
        }

        // Point is outside, so it must be on the boundary
        boundary.push_back(p);
        sphere = WelzlRecursive(points, boundary, n - 1);
        boundary.pop_back();

        return sphere;
    }

    BoundingSphere BoundingVolumeCalculator::SphereFromPoints(const std::vector<Math::Vec3>& points) {
        switch (points.size()) {
            case 0:
                return BoundingSphere();
            case 1:
                return BoundingSphere(points[0], 0.0f);
            case 2:
                return SphereFrom2Points(points[0], points[1]);
            case 3:
                return SphereFrom3Points(points[0], points[1], points[2]);
            case 4: {
                // For 4 points, we need to solve for the circumsphere
                // This is complex, so we'll use a simplified approach
                BoundingSphere sphere = SphereFrom3Points(points[0], points[1], points[2]);
                sphere.Expand(points[3]);
                return sphere;
            }
            default:
                // Fallback to naive approach for more than 4 points
                return CalculateNaiveSphere(points);
        }
    }

    BoundingSphere BoundingVolumeCalculator::SphereFrom2Points(const Math::Vec3& p1, const Math::Vec3& p2) {
        Math::Vec3 center = (p1 + p2) * 0.5f;
        float radius = glm::length(p2 - p1) * 0.5f;
        return BoundingSphere(center, radius);
    }

    BoundingSphere BoundingVolumeCalculator::SphereFrom3Points(const Math::Vec3& p1, const Math::Vec3& p2, const Math::Vec3& p3) {
        // Calculate circumcenter of triangle
        Math::Vec3 a = p2 - p1;
        Math::Vec3 b = p3 - p1;
        Math::Vec3 cross_ab = glm::cross(a, b);
        
        float denominator = 2.0f * glm::dot(cross_ab, cross_ab);
        if (std::abs(denominator) < 1e-6f) {
            // Points are collinear, fall back to 2-point sphere
            return SphereFrom2Points(p1, p3);
        }

        Math::Vec3 center = p1 + (glm::cross(cross_ab, a) * glm::dot(b, b) + glm::cross(b, cross_ab) * glm::dot(a, a)) / denominator;
        float radius = glm::length(center - p1);

        return BoundingSphere(center, radius);
    }

    bool BoundingVolumeCalculator::PointInSphere(const Math::Vec3& point, const BoundingSphere& sphere, float epsilon) {
        if (!sphere.IsValid()) {
            return false;
        }
        
        float distance = glm::length(point - sphere.center);
        return distance <= sphere.radius + epsilon;
    }

    BoundingSphere BoundingVolumeCalculator::CalculateOptimalSphere(const std::vector<Math::Vec3>& points) {
        if (points.empty()) {
            return BoundingSphere();
        }

        // Try different algorithms and pick the best one
        BoundingSphere naiveSphere = CalculateNaiveSphere(points);
        BoundingSphere ritterSphere = CalculateRitterSphere(points);
        BoundingSphere welzlSphere = CalculateWelzlSphere(points);

        // Choose the sphere with the smallest radius (most efficient)
        BoundingSphere bestSphere = naiveSphere;
        if (ritterSphere.IsValid() && ritterSphere.radius < bestSphere.radius) {
            bestSphere = ritterSphere;
        }
        if (welzlSphere.IsValid() && welzlSphere.radius < bestSphere.radius) {
            bestSphere = welzlSphere;
        }

        return bestSphere;
    }

    BoundingSphere BoundingVolumeCalculator::CalculateSphereFromMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) {
        BoundingSphere combinedSphere;
        
        for (const auto& mesh : meshes) {
            if (mesh) {
                combinedSphere.Expand(mesh->GetBoundingSphere());
            }
        }
        
        return combinedSphere;
    }

    BoundingSphere BoundingVolumeCalculator::CalculateHierarchicalSphere(std::shared_ptr<ModelNode> rootNode,
                                                                         const std::vector<std::shared_ptr<Mesh>>& meshes) {
        if (!rootNode) {
            return BoundingSphere();
        }

        BoundingSphere hierarchicalSphere;

        // Traverse the node hierarchy and accumulate bounds
        rootNode->Traverse([&](std::shared_ptr<ModelNode> node) {
            if (!node) return;

            // Get sphere from meshes associated with this node
            BoundingSphere nodeSphere = node->CalculateCombinedBoundingSphere(meshes);
            
            // Transform sphere to world space
            if (nodeSphere.IsValid()) {
                BoundingSphere worldSphere = TransformBoundingSphere(nodeSphere, node->GetWorldTransform());
                hierarchicalSphere.Expand(worldSphere);
            }
        });

        return hierarchicalSphere;
    }

    std::vector<Math::Vec3> BoundingVolumeCalculator::ExtractVertexPositions(const std::vector<std::shared_ptr<Mesh>>& meshes) {
        std::vector<Math::Vec3> positions;
        
        for (const auto& mesh : meshes) {
            if (!mesh) continue;
            
            const auto& vertices = mesh->GetVertices();
            for (const auto& vertex : vertices) {
                positions.push_back(vertex.position);
            }
        }
        
        return positions;
    }

    BoundingBox BoundingVolumeCalculator::TransformBoundingBox(const BoundingBox& box, const Math::Mat4& transform) {
        return box.Transform(transform);
    }

    BoundingSphere BoundingVolumeCalculator::TransformBoundingSphere(const BoundingSphere& sphere, const Math::Mat4& transform) {
        if (!sphere.IsValid()) {
            return sphere;
        }

        // Transform center
        Math::Vec4 transformedCenter = transform * Math::Vec4(sphere.center, 1.0f);
        
        // Calculate scale factor from transform matrix
        Math::Vec3 scale = Math::Vec3(
            glm::length(Math::Vec3(transform[0])),
            glm::length(Math::Vec3(transform[1])),
            glm::length(Math::Vec3(transform[2]))
        );
        float maxScale = std::max({scale.x, scale.y, scale.z});
        
        return BoundingSphere(Math::Vec3(transformedCenter), sphere.radius * maxScale);
    }

    bool BoundingVolumeCalculator::ValidateBoundingVolume(const BoundingBox& box, const std::vector<Math::Vec3>& points) {
        if (!box.IsValid()) {
            return points.empty();
        }

        for (const auto& point : points) {
            if (point.x < box.min.x || point.x > box.max.x ||
                point.y < box.min.y || point.y > box.max.y ||
                point.z < box.min.z || point.z > box.max.z) {
                return false;
            }
        }
        return true;
    }

    bool BoundingVolumeCalculator::ValidateBoundingVolume(const BoundingSphere& sphere, const std::vector<Math::Vec3>& points) {
        if (!sphere.IsValid()) {
            return points.empty();
        }

        for (const auto& point : points) {
            if (glm::length(point - sphere.center) > sphere.radius + 1e-6f) {
                return false;
            }
        }
        return true;
    }

    float BoundingVolumeCalculator::CalculateBoundingVolumeEfficiency(const BoundingSphere& sphere, const std::vector<Math::Vec3>& points) {
        if (!sphere.IsValid() || points.empty()) {
            return 0.0f;
        }

        // Calculate the volume of the sphere
        float sphereVolume = (4.0f / 3.0f) * M_PI * sphere.radius * sphere.radius * sphere.radius;
        
        // Estimate the volume occupied by the points (very rough approximation)
        // This is a simplified metric - in practice, you might want a more sophisticated measure
        BoundingBox pointsBox = CalculateAABB(points);
        if (!pointsBox.IsValid()) {
            return 0.0f;
        }
        
        Math::Vec3 size = pointsBox.GetSize();
        float pointsVolume = size.x * size.y * size.z;
        
        // Efficiency is the ratio of actual content to bounding volume
        return (pointsVolume > 0.0f) ? (pointsVolume / sphereVolume) : 0.0f;
    }

    BoundingBox BoundingVolumeCalculator::InterpolateBoundingBox(const BoundingBox& box1, const BoundingBox& box2, float t) {
        if (!box1.IsValid() || !box2.IsValid()) {
            return box1.IsValid() ? box1 : box2;
        }
        
        t = std::clamp(t, 0.0f, 1.0f);
        
        BoundingBox result;
        result.min = box1.min + t * (box2.min - box1.min);
        result.max = box1.max + t * (box2.max - box1.max);
        
        return result;
    }

    BoundingSphere BoundingVolumeCalculator::InterpolateBoundingSphere(const BoundingSphere& sphere1, const BoundingSphere& sphere2, float t) {
        if (!sphere1.IsValid() || !sphere2.IsValid()) {
            return sphere1.IsValid() ? sphere1 : sphere2;
        }
        
        t = std::clamp(t, 0.0f, 1.0f);
        
        BoundingSphere result;
        result.center = sphere1.center + t * (sphere2.center - sphere1.center);
        result.radius = sphere1.radius + t * (sphere2.radius - sphere1.radius);
        
        return result;
    }

    std::vector<BoundingBox> BoundingVolumeCalculator::GenerateAnimatedBoundingBoxes(const std::vector<std::vector<Math::Vec3>>& animatedVertices) {
        std::vector<BoundingBox> result;
        result.reserve(animatedVertices.size());
        
        for (const auto& vertices : animatedVertices) {
            result.push_back(CalculateAABB(vertices));
        }
        
        return result;
    }

    std::vector<BoundingSphere> BoundingVolumeCalculator::GenerateAnimatedBoundingSpheres(const std::vector<std::vector<Math::Vec3>>& animatedVertices) {
        std::vector<BoundingSphere> result;
        result.reserve(animatedVertices.size());
        
        for (const auto& vertices : animatedVertices) {
            result.push_back(CalculateOptimalSphere(vertices));
        }
        
        return result;
    }

    bool BoundingVolumeCalculator::IsPointInBoundingBox(const Math::Vec3& point, const BoundingBox& box) {
        if (!box.IsValid()) {
            return false;
        }
        
        return point.x >= box.min.x && point.x <= box.max.x &&
               point.y >= box.min.y && point.y <= box.max.y &&
               point.z >= box.min.z && point.z <= box.max.z;
    }

    bool BoundingVolumeCalculator::IsPointInBoundingSphere(const Math::Vec3& point, const BoundingSphere& sphere) {
        if (!sphere.IsValid()) {
            return false;
        }
        
        float distanceSquared = glm::dot(point - sphere.center, point - sphere.center);
        return distanceSquared <= sphere.radius * sphere.radius;
    }

    bool BoundingVolumeCalculator::DoBoundingBoxesIntersect(const BoundingBox& box1, const BoundingBox& box2) {
        if (!box1.IsValid() || !box2.IsValid()) {
            return false;
        }
        
        return box1.min.x <= box2.max.x && box1.max.x >= box2.min.x &&
               box1.min.y <= box2.max.y && box1.max.y >= box2.min.y &&
               box1.min.z <= box2.max.z && box1.max.z >= box2.min.z;
    }

    bool BoundingVolumeCalculator::DoBoundingSpheresIntersect(const BoundingSphere& sphere1, const BoundingSphere& sphere2) {
        if (!sphere1.IsValid() || !sphere2.IsValid()) {
            return false;
        }
        
        float distanceSquared = glm::dot(sphere1.center - sphere2.center, sphere1.center - sphere2.center);
        float radiusSum = sphere1.radius + sphere2.radius;
        
        return distanceSquared <= radiusSum * radiusSum;
    }
}