#include "Animation/Keyframe.h"
#include "Core/Math.h"
#include <iostream>

int main() {
    GameEngine::Animation::PositionTrack track("TestBone", "position");
    
    Math::Vec3 startPos(0.0f, 0.0f, 0.0f);
    Math::Vec3 endPos(10.0f, 0.0f, 0.0f);
    
    std::cout << "Adding keyframes to track..." << std::endl;
    track.AddKeyframe(0.0f, startPos);
    track.AddKeyframe(2.0f, endPos);
    
    std::cout << "Keyframe count: " << track.GetKeyframeCount() << std::endl;
    std::cout << "IsEmpty: " << (track.IsEmpty() ? "true" : "false") << std::endl;
    std::cout << "Start time: " << track.GetStartTime() << std::endl;
    std::cout << "End time: " << track.GetEndTime() << std::endl;
    
    auto sample0 = track.SampleAt(0.0f);
    auto sample1 = track.SampleAt(1.0f);
    auto sample2 = track.SampleAt(2.0f);
    
    std::cout << "Sample at t=0.0: (" << sample0.x << ", " << sample0.y << ", " << sample0.z << ")" << std::endl;
    std::cout << "Sample at t=1.0: (" << sample1.x << ", " << sample1.y << ", " << sample1.z << ")" << std::endl;
    std::cout << "Sample at t=2.0: (" << sample2.x << ", " << sample2.y << ", " << sample2.z << ")" << std::endl;
    
    return 0;
}