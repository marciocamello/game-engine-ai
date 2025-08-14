#include "Animation/SkeletalAnimation.h"
#include "Core/Math.h"
#include <iostream>

int main() {
    GameEngine::Animation::SkeletalAnimation animation("DebugTest");
    
    Math::Vec3 startPos(0.0f, 0.0f, 0.0f);
    Math::Vec3 endPos(10.0f, 0.0f, 0.0f);
    
    std::cout << "Adding keyframes..." << std::endl;
    animation.AddPositionKeyframe("Bone", 0.0f, startPos);
    animation.AddPositionKeyframe("Bone", 2.0f, endPos);
    
    auto* boneAnim = animation.GetBoneAnimation("Bone");
    if (boneAnim) {
        std::cout << "BoneAnimation found" << std::endl;
        std::cout << "HasPositionTrack: " << (boneAnim->HasPositionTrack() ? "true" : "false") << std::endl;
        
        if (boneAnim->positionTrack) {
            std::cout << "Position track exists" << std::endl;
            std::cout << "Keyframe count: " << boneAnim->positionTrack->GetKeyframeCount() << std::endl;
            std::cout << "IsEmpty: " << (boneAnim->positionTrack->IsEmpty() ? "true" : "false") << std::endl;
        } else {
            std::cout << "Position track is null" << std::endl;
        }
    } else {
        std::cout << "BoneAnimation not found" << std::endl;
    }
    
    return 0;
}