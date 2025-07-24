#pragma once

#include "Core/Math.h"
#include <algorithm>

namespace GameEngine {
    struct BoundingBox {
        Math::Vec3 min = Math::Vec3(0.0f);
        Math::Vec3 max = Math::Vec3(0.0f);
        
        BoundingBox() = default;
        BoundingBox(const Math::Vec3& minPoint, const Math::Vec3& maxPoint) 
            : min(minPoint), max(maxPoint) {}
        
        Math::Vec3 GetCenter() const { return (min + max) * 0.5f; }
        Math::Vec3 GetSize() const { return max - min; }
        bool IsValid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }
        
        void Expand(const Math::Vec3& point) {
            if (!IsValid()) {
                min = max = point;
            } else {
                min = Math::Vec3(std::min(min.x, point.x), std::min(min.y, point.y), std::min(min.z, point.z));
                max = Math::Vec3(std::max(max.x, point.x), std::max(max.y, point.y), std::max(max.z, point.z));
            }
        }
        
        void Expand(const BoundingBox& other) {
            if (other.IsValid()) {
                Expand(other.min);
                Expand(other.max);
            }
        }
        
        BoundingBox Transform(const Math::Mat4& transform) const {
            if (!IsValid()) return *this;
            
            BoundingBox result;
            
            // Transform all 8 corners of the bounding box
            Math::Vec3 corners[8] = {
                Math::Vec3(min.x, min.y, min.z),
                Math::Vec3(max.x, min.y, min.z),
                Math::Vec3(min.x, max.y, min.z),
                Math::Vec3(max.x, max.y, min.z),
                Math::Vec3(min.x, min.y, max.z),
                Math::Vec3(max.x, min.y, max.z),
                Math::Vec3(min.x, max.y, max.z),
                Math::Vec3(max.x, max.y, max.z)
            };
            
            for (int i = 0; i < 8; ++i) {
                Math::Vec4 transformedCorner = transform * Math::Vec4(corners[i], 1.0f);
                result.Expand(Math::Vec3(transformedCorner));
            }
            
            return result;
        }
    };

    struct BoundingSphere {
        Math::Vec3 center = Math::Vec3(0.0f);
        float radius = 0.0f;
        
        BoundingSphere() = default;
        BoundingSphere(const Math::Vec3& centerPoint, float sphereRadius) 
            : center(centerPoint), radius(sphereRadius) {}
        
        bool IsValid() const { return radius > 0.0f; }
        
        void Expand(const Math::Vec3& point) {
            if (!IsValid()) {
                center = point;
                radius = 0.0f;
            } else {
                Math::Vec3 toPoint = point - center;
                float distance = glm::length(toPoint);
                if (distance > radius) {
                    // Expand sphere to include the point
                    float newRadius = (radius + distance) * 0.5f;
                    Math::Vec3 newCenter = center + toPoint * ((newRadius - radius) / distance);
                    center = newCenter;
                    radius = newRadius;
                }
            }
        }
        
        void Expand(const BoundingSphere& other) {
            if (!other.IsValid()) return;
            
            if (!IsValid()) {
                *this = other;
                return;
            }
            
            Math::Vec3 toOther = other.center - center;
            float distance = glm::length(toOther);
            
            if (distance + other.radius <= radius) {
                // Other sphere is inside this one
                return;
            }
            
            if (distance + radius <= other.radius) {
                // This sphere is inside the other one
                *this = other;
                return;
            }
            
            // Spheres overlap or are separate - create encompassing sphere
            float newRadius = (radius + distance + other.radius) * 0.5f;
            Math::Vec3 newCenter = center + toOther * ((newRadius - radius) / distance);
            center = newCenter;
            radius = newRadius;
        }
    };
}