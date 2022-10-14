ShapePtr shape, obstacle;
Vector3 obstacleStartPos, playerStartPos;

random_device d;
default_random_engine eng(d());

void Start()
{
    shape = Game::scene.GetShape("shape");
    obstacle = Game::scene.GetShape("shape1");
    shape().GetRigidBody().setIsActive(true);
    obstacleStartPos = Vector3(0.0, -10.0, -25.0);
    playerStartPos = Vector3(0.0, 0.0, 0.0);
    obstacle().SetPosition(obstacleStartPos);
    shape().SetPosition(playerStartPos);
}

void Loop()
{
    if(Keyboard::isKeyPressed(Keyboard::Q))
    {
        shape().GetRigidBody().setLinearVelocity(Vector3(0.0, 8.0, 0.0));
        shape().GetRigidBody().setAngularVelocity(Vector3(4.0, 3.0, 2.0));
    }
    obstacle().Move(Vector3(0.0, 0.0, 0.5));
    shape().SetPosition(Vector3(0.0, shape().GetPosition().y, 0.0));
    if(obstacle().GetPosition().z > 10)
    {
        obstacleStartPos.y = eng(-10, 20);
        obstacle().SetPosition(obstacleStartPos);
    }
    if(obstacle().GetRigidBody().testAABBOverlap(shape().GetRigidBody().getAABB()))
    {
        Log::Write("You're dead!", Log::Info);
        shape().SetPosition(playerStartPos);
        shape().GetRigidBody().setLinearVelocity(Vector3(0.0, 0.0, 0.0));
        obstacle().SetPosition(obstacleStartPos);
    }
}
