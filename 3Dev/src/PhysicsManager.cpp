#include <PhysicsManager.hpp>

PhysicsManager::PhysicsManager(rp3d::PhysicsWorld::WorldSettings settings) : settings(settings)
{
    world = common.createPhysicsWorld(settings);
}

PhysicsManager::~PhysicsManager()
{
    common.destroyPhysicsWorld(world);
}

void PhysicsManager::Update(float& time)
{
    while(time >= timeStep)
    {
        world->update(timeStep);
        time -= timeStep;
    }
}

void PhysicsManager::SetTimeStep(float timeStep)
{
    this->timeStep = timeStep;
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

rp3d::ConvexMeshShape* PhysicsManager::CreateConvexMeshShape(rp3d::PolyhedronMesh* mesh, rp3d::Vector3 size)
{
    return common.createConvexMeshShape(mesh, size);
}

rp3d::ConcaveMeshShape* PhysicsManager::CreateConcaveMeshShape(rp3d::TriangleMesh* mesh, rp3d::Vector3 size)
{
    return common.createConcaveMeshShape(mesh, size);
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

rp3d::FixedJoint* PhysicsManager::CreateFixedJoint(rp3d::FixedJointInfo info)
{
    return dynamic_cast<rp3d::FixedJoint*>(world->createJoint(info));
}

rp3d::SliderJoint* PhysicsManager::CreateSliderJoint(rp3d::SliderJointInfo info)
{
    return dynamic_cast<rp3d::SliderJoint*>(world->createJoint(info));
}

rp3d::TriangleMesh* PhysicsManager::CreateTriangleMesh()
{
    return common.createTriangleMesh();
}

rp3d::PolyhedronMesh* PhysicsManager::CreatePolyhedronMesh(rp3d::PolygonVertexArray* array)
{
    return common.createPolyhedronMesh(array);
}

rp3d::PhysicsWorld::WorldSettings PhysicsManager::GetSettings()
{
    return settings;
}

rp3d::PhysicsWorld* PhysicsManager::GetWorld()
{
    return world;
}
