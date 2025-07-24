#include "Audio/Audio3DCalculator.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {

    Audio3DCalculator::Audio3DCalculator() {
        LOG_DEBUG("Audio3DCalculator initialized");
    }

    Audio3DCalculator::~Audio3DCalculator() {
    }

    Audio3DCalculator::AudioResult3D Audio3DCalculator::Calculate3DAudio(const AudioParams3D& params) {
        m_calculationCount++;
        
        AudioResult3D result;
        
        // Calculate relative position and distance
        result.relativePosition = CalculateRelativePosition(params);
        
        if (m_fastMath) {
            result.distance = FastDistance(params.sourcePosition, params.listenerPosition);
        } else {
            result.distance = glm::distance(params.sourcePosition, params.listenerPosition);
        }
        
        // Distance culling
        if (m_distanceCulling && result.distance > m_maxAudibleDistance) {
            result.audible = false;
            result.gain = 0.0f;
            result.pitch = 1.0f;
            return result;
        }
        
        // Calculate distance attenuation
        result.gain = CalculateDistanceAttenuation(result.distance, params);
        
        // Calculate Doppler effect
        result.pitch = CalculateDopplerShift(params, result.distance);
        
        // Ensure reasonable bounds
        result.gain = Math::Clamp(result.gain, 0.0f, 1.0f);
        result.pitch = Math::Clamp(result.pitch, 0.1f, 4.0f);
        
        return result;
    }

    void Audio3DCalculator::CalculateBatch3DAudio(const std::vector<AudioParams3D>& paramsList, 
                                                 std::vector<AudioResult3D>& results) {
        if (paramsList.empty()) {
            results.clear();
            return;
        }
        
        results.resize(paramsList.size());
        
        // Cache listener data for batch processing efficiency
        if (!paramsList.empty()) {
            PrecomputeListenerData(paramsList[0]);
        }
        
        // Process all sources with cached listener data
        for (size_t i = 0; i < paramsList.size(); ++i) {
            results[i] = Calculate3DAudio(paramsList[i]);
        }
        
        m_listenerDataCached = false; // Reset cache
    }

    float Audio3DCalculator::CalculateDistanceAttenuation(float distance, const AudioParams3D& params) {
        if (distance <= params.referenceDistance) {
            return 1.0f; // No attenuation within reference distance
        }
        
        if (distance >= params.maxDistance) {
            return 0.0f; // Complete attenuation beyond max distance
        }
        
        float attenuation = 1.0f;
        float normalizedDistance = distance / params.referenceDistance;
        
        switch (params.attenuationModel) {
            case AttenuationModel::Linear:
                attenuation = 1.0f - ((distance - params.referenceDistance) / 
                                     (params.maxDistance - params.referenceDistance));
                break;
                
            case AttenuationModel::Inverse:
                attenuation = params.referenceDistance / 
                             (params.referenceDistance + params.rolloffFactor * (distance - params.referenceDistance));
                break;
                
            case AttenuationModel::Exponential:
                attenuation = std::pow(normalizedDistance, -params.rolloffFactor);
                break;
                
            case AttenuationModel::InverseSquare:
            default:
                if (m_fastMath) {
                    // Fast approximation using bit manipulation
                    attenuation = FastInverseSqrt(normalizedDistance * normalizedDistance);
                } else {
                    attenuation = 1.0f / (normalizedDistance * normalizedDistance);
                }
                break;
        }
        
        return Math::Clamp(attenuation, 0.0f, 1.0f);
    }

    float Audio3DCalculator::CalculateDopplerShift(const AudioParams3D& params, float distance) {
        if (m_dopplerFactor <= 0.0f || distance <= 0.0f) {
            return 1.0f; // No Doppler effect
        }
        
        // Calculate relative velocity along the line between source and listener
        Math::Vec3 direction = (params.sourcePosition - params.listenerPosition) / distance;
        
        float sourceVelocityComponent = glm::dot(params.sourceVelocity, direction);
        float listenerVelocityComponent = glm::dot(params.listenerVelocity, direction);
        
        // Doppler shift calculation
        float relativeVelocity = sourceVelocityComponent - listenerVelocityComponent;
        
        // Prevent division by zero and extreme values
        float denominator = m_speedOfSound - relativeVelocity;
        if (std::abs(denominator) < 0.1f) {
            return 1.0f; // Avoid extreme Doppler shifts
        }
        
        float dopplerShift = (m_speedOfSound + listenerVelocityComponent) / denominator;
        
        // Apply Doppler factor for artistic control
        dopplerShift = 1.0f + (dopplerShift - 1.0f) * m_dopplerFactor;
        
        return Math::Clamp(dopplerShift, 0.1f, 4.0f);
    }

    Math::Vec3 Audio3DCalculator::CalculateRelativePosition(const AudioParams3D& params) {
        // Transform source position to listener's coordinate system
        Math::Vec3 relativePos = params.sourcePosition - params.listenerPosition;
        
        // Create listener coordinate system
        Math::Vec3 forward = glm::normalize(params.listenerForward);
        Math::Vec3 up = glm::normalize(params.listenerUp);
        Math::Vec3 right = glm::cross(forward, up);
        
        // Transform to listener space
        Math::Vec3 listenerSpacePos;
        listenerSpacePos.x = glm::dot(relativePos, right);
        listenerSpacePos.y = glm::dot(relativePos, up);
        listenerSpacePos.z = glm::dot(relativePos, forward);
        
        return listenerSpacePos;
    }

    float Audio3DCalculator::FastInverseSqrt(float x) {
        if (!m_fastMath) {
            return 1.0f / std::sqrt(x);
        }
        
        // Fast inverse square root approximation (Quake III algorithm)
        union {
            float f;
            uint32_t i;
        } conv = { .f = x };
        
        conv.i = 0x5f3759df - (conv.i >> 1);
        conv.f *= 1.5f - (x * 0.5f * conv.f * conv.f);
        
        return conv.f;
    }

    float Audio3DCalculator::FastDistance(const Math::Vec3& a, const Math::Vec3& b) {
        if (!m_fastMath) {
            return glm::distance(a, b);
        }
        
        Math::Vec3 diff = a - b;
        float squaredDistance = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        
        // Use fast inverse square root for distance calculation
        return squaredDistance * FastInverseSqrt(squaredDistance);
    }

    void Audio3DCalculator::PrecomputeListenerData(const AudioParams3D& listenerParams) {
        m_cachedListenerPos = listenerParams.listenerPosition;
        m_cachedListenerVel = listenerParams.listenerVelocity;
        m_cachedListenerForward = glm::normalize(listenerParams.listenerForward);
        m_cachedListenerUp = glm::normalize(listenerParams.listenerUp);
        m_listenerDataCached = true;
    }

    void Audio3DCalculator::ResetStatistics() {
        m_calculationsPerSecond = 0;
        m_calculationCount = 0;
        m_statisticsTimer = 0.0f;
    }

} // namespace GameEngine