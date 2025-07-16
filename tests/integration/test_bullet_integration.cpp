#include <iostream>
#include <btBulletDynamicsCommon.h>

int main() {
    // Test basic Bullet Physics initialization
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    
    std::cout << "Bullet Physics integration test successful!" << std::endl;
    std::cout << "Gravity: " << dynamicsWorld->getGravity().getX() << ", " 
              << dynamicsWorld->getGravity().getY() << ", " 
              << dynamicsWorld->getGravity().getZ() << std::endl;
    
    // Cleanup
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    
    return 0;
}