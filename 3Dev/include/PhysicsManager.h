#pragma once
#include "3Dev.h"

/*
 * This class simplifies the work with reactphysics3d library
 */
class PhysicsManager
{
public:
    /*
     * Basic constructor
     * @param settings settings of your physics world
     */
    PhysicsManager(rp3d::PhysicsWorld::WorldSettings settings);

    /*
     * Updates the physics
     * @param t time step
     */
    void Update(float t);
    
    /*
     * Create a box collision shape
     * NOTE: to create, for example, box shape with size (10, 10, 10)
     * you need to pass (5, 5, 5) in the function
     * @param size shape extent
     * @return pointer to the box shape object
     */
    rp3d::BoxShape* CreateBoxShape(rp3d::Vector3 size);

    /*
     * Create a sphere collision shape
     * @param radius sphere shape radius
     * @return pointer to the sphere shape object
     */
    rp3d::SphereShape* CreateSphereShape(float radius);

    /*
     * Create a capsule collision shape
     * @param radius radius of the top and down parts
     * @param height capsule shape height
     * @return pointer to the capsule shape object
     */
    rp3d::CapsuleShape* CreateCapsuleShape(float radius, float height);
    
    /*
     * Create a rigid body
     * @param transform initial transform of the body
     * @return pointer to the rigid body object
     */
    rp3d::RigidBody* CreateRigidBody(const rp3d::Transform& transform);
    
    rp3d::BallAndSocketJoint* CreateBallAndSocketJoint(rp3d::BallAndSocketJointInfo info);
    rp3d::HingeJoint* CreateHingeJoint(rp3d::HingeJointInfo info);
    rp3d::SliderJoint* CreateSliderJoint(rp3d::SliderJointInfo info);

    rp3d::PhysicsWorld::WorldSettings GetSettings();
    rp3d::PhysicsWorld* GetWorld();

private:
    rp3d::PhysicsCommon common;
    rp3d::PhysicsWorld::WorldSettings settings;
    rp3d::PhysicsWorld* world;
};
