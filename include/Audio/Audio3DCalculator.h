#pragma once

#include "Core/Math.h"
#include <vector>

namespace GameEngine {

    // Optimized 3D audio calculation algorithms
    class Audio3DCalculator {
    public:
        // Distance attenuation models
        enum class AttenuationModel {
            Linear,
            Inverse,
            Exponential,
            InverseSquare
        };

        // 3D audio parameters
        struct AudioParams3D {
            Math::Vec3 sourcePosition{0.0f};
            Math::Vec3 sourceVelocity{0.0f};
            Math::Vec3 listenerPosition{0.0f};
            Math::Vec3 listenerVelocity{0.0f};
            Math::Vec3 listenerForward{0.0f, 0.0f, -1.0f};
            Math::Vec3 listenerUp{0.0f, 1.0f, 0.0f};
            
            float referenceDistance = 1.0f;
            float maxDistance = 100.0f;
            float rolloffFactor = 1.0f;
            AttenuationModel attenuationModel = AttenuationModel::InverseSquare;
        };

        // Calculated audio properties
        struct AudioResult3D {
            float gain = 1.0f;
            float pitch = 1.0f; // For Doppler effect
            Math::Vec3 relativePosition{0.0f};
            float distance = 0.0f;
            bool audible = true;
        };

        Audio3DCalculator();
        ~Audio3DCalculator();

        // Single source calculation
        AudioResult3D Calculate3DAudio(const AudioParams3D& params);
        
        // Batch calculation for multiple sources (more efficient)
        void CalculateBatch3DAudio(const std::vector<AudioParams3D>& paramsList, 
                                  std::vector<AudioResult3D>& results);
        
        // Configuration
        void SetSpeedOfSound(float speed) { m_speedOfSound = speed; }
        void SetDopplerFactor(float factor) { m_dopplerFactor = factor; }
        void SetDistanceCulling(bool enabled) { m_distanceCulling = enabled; }
        void SetMaxAudibleDistance(float distance) { m_maxAudibleDistance = distance; }
        
        // Performance optimization settings
        void SetUpdateFrequency(float hz) { m_updateInterval = 1.0f / hz; }
        void EnableFastMath(bool enabled) { m_fastMath = enabled; }
        
        // Statistics
        int GetCalculationsPerSecond() const { return m_calculationsPerSecond; }
        void ResetStatistics();

    private:
        float m_speedOfSound = 343.0f; // m/s at 20°C
        float m_dopplerFactor = 1.0f;
        bool m_distanceCulling = true;
        float m_maxAudibleDistance = 100.0f;
        float m_updateInterval = 1.0f / 60.0f; // 60 Hz default
        bool m_fastMath = true;
        
        // Statistics
        mutable int m_calculationsPerSecond = 0;
        mutable int m_calculationCount = 0;
        mutable float m_statisticsTimer = 0.0f;
        
        // Optimized calculation methods
        float CalculateDistanceAttenuation(float distance, const AudioParams3D& params);
        float CalculateDopplerShift(const AudioParams3D& params, float distance);
        Math::Vec3 CalculateRelativePosition(const AudioParams3D& params);
        
        // Fast math approximations
        float FastInverseSqrt(float x);
        float FastDistance(const Math::Vec3& a, const Math::Vec3& b);
        
        // Batch optimization helpers
        void PrecomputeListenerData(const AudioParams3D& listenerParams);
        
        // Cached listener data for batch processing
        Math::Vec3 m_cachedListenerPos{0.0f};
        Math::Vec3 m_cachedListenerVel{0.0f};
        Math::Vec3 m_cachedListenerForward{0.0f, 0.0f, -1.0f};
        Math::Vec3 m_cachedListenerUp{0.0f, 1.0f, 0.0f};
        bool m_listenerDataCached = false;
    };

} // namespace GameEngine