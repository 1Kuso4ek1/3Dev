#include <PhysicsManager.h>

PhysicsManager::PhysicsManager(rp3d::PhysicsWorld::WorldSettings settings) : settings(settings)
{
    world = common.createPhysicsWorld(settings);
}

PhysicsManager::~PhysicsManager()
{
    common.destroyPhysicsWorld(world);
}

void PhysicsManager::Update(float t)
{
    world->update(t);
}

rp3d::BoxShape* PhysicsManager::CreateBoxShape(rp3d::Vector3 size)
{
    return common.createBoxShape(size);
}

rp3d::SphereShape* PhysicsManager::CreateSphereShape(float radius)
{
    return common.createSphereShape(radius);
}

rp3d::CapsuleShape* PhysicsManager::CreateCapsuleShape(float radius, float height)
{
    return common.createCapsuleShape(radius, height);
}

rp3d::RigidBody* PhysicsManager::CreateRigidBody(const rp3d::Transform& transform)
{
    return world->createRigidBody(transform);
}

rp3d::BallAndSocketJoint* PhysicsManager::CreateBallAndSocketJoint(rp3d::BallAndSocketJointInfo info)
{
    return dynamic_cast<rp3d::BallAndSocketJoint*>(world->createJoint(info));
}

rp3d::HingeJoint* PhysicsManager::CreateHingeJoint(rp3d::HingeJointInfo info)
{
    return dynamic_cast<rp3d::HingeJoint*>(world->createJoint(info));
}

rp3d::SliderJoint* PhysicsManager::CreateSliderJoint(rp3d::SliderJointInfo info)
{
    return dynamic_cast<rp3d::SliderJoint*>(world->createJoint(info));
}

rp3d::PhysicsWorld::WorldSettings PhysicsManager::GetSettings()
{
    return settings;
}

rp3d::PhysicsWorld* PhysicsManager::GetWorld()
{
    return world;
}
