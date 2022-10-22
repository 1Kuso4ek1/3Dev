random_device d;
default_random_engine eng(d());

class Object
{
    void Reset()
    {
        shape().SetPosition(startPos);
    }

    ShapePtr shape;
    Vector3 startPos;
};

Object bird, obstacle, obstacle1;

bool gameRunning = false;

void Start()
{
    Game::manageCameraMouse = false;
    Game::manageCameraMovement = false;

    bird.shape = Game::scene.GetShape("bird");
    obstacle.shape = Game::scene.GetShape("obstacle");
    obstacle1.shape = Game::scene.GetShape("obstacle1");

    bird.shape().GetRigidBody().setIsActive(false);
    obstacle.shape().GetRigidBody().setIsActive(false);
    obstacle1.shape().GetRigidBody().setIsActive(false);

    bird.startPos = Vector3(0.0, 0.0, 0.0);
    obstacle.startPos = Vector3(0.0, -15.0, -25.0);
    obstacle1.startPos = Vector3(0.0, 15.0, -25.0);

    bird.Reset();
    obstacle.Reset();
    obstacle1.Reset();
}

void Loop()
{
    if(Keyboard::isKeyPressed(Keyboard::Space))
    {
        if(!gameRunning)
        {
            gameRunning = true;
            bird.shape().GetRigidBody().setIsActive(true);
        }
        bird.shape().GetRigidBody().setLinearVelocity(Vector3(0.0, 8.0, 0.0));
    }

    bird.shape().SetPosition(Vector3(0.0, bird.shape().GetPosition().y, 0.0));
    
    if(gameRunning)
    {
        obstacle.shape().Move(Vector3(0.0, 0.0, 0.5));
        obstacle1.shape().Move(Vector3(0.0, 0.0, 0.5));
    }

    if(obstacle.shape().GetPosition().z > 10)
    {
        float y = eng(-15, 5);
        obstacle.startPos.y = y;
        obstacle1.startPos.y = y + 30;

        obstacle.Reset();
        obstacle1.Reset();
    }
    if(obstacle.shape().GetRigidBody().testAABBOverlap(bird.shape().GetRigidBody().getAABB()) ||
       obstacle1.shape().GetRigidBody().testAABBOverlap(bird.shape().GetRigidBody().getAABB()))
    {
        Log::Write("You're dead!", Log::Info);
        bird.Reset();
        bird.shape().GetRigidBody().setLinearVelocity(Vector3(0.0, 0.0, 0.0));

        float y = eng(-15, 5);
        obstacle.startPos.y = y;
        obstacle1.startPos.y = y + 30;

        obstacle.Reset();
        obstacle1.Reset();
        
        gameRunning = false;
        bird.shape().GetRigidBody().setIsActive(false);
    }
}
