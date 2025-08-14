#include "Animation/SkeletalAnimation.h"
#include "Core/Math.h"
#include <iostream>

int main() {
    GameEngine::Animation::SkeletalAnimation animation("DebugTest");
    
    Math::Vec3 startPos(0.0f, 0.0f, 0.0f);
    Math::Vec3 endPos(10.0f, 0.0f, 0.0f);
    
    std::cout << "Initial duration: " << animation.GetDuration() << std::endl;
    
    animation.AddPositionKeyframe("Bone", 0.0f, startPos);
    std::cout << "Duration after first keyframe: " << animation.GetDuration() << std::endl;
    
    animation.AddPositionKeyframe("Bone", 2.0f, endPos);
    std::cout << "Duration after second keyframe: " << animation.GetDuration() << std::endl;
    
    auto pose1 = animation.SampleBone("Bone", 1.0f);
    std::cout << "Sampled position at t=1.0: (" << pose1.position.x << ", " << pose1.position.y << ", " << pose1.position.z << ")" << std::endl;
    std::cout << "Has position: " << (pose1.hasPosition ? "true" : "false") << std::endl;
    
    return 0;
}