ModelPtr sphere;
int selectedPlayer = 0;
bool sphereHavePhysics = false;

class Player
{
    Player() {}

    Player(ShapePtr shape)
    {
        this.shape = shape;
    }

    void Move()
    {
        if(isAlive)
        {
            Game::camera.AlwaysUp(true);
            Vector3 v = Game::camera.Move(1.0); v *= 8.0;
            if(v != Vector3(0.0, 0.0, 0.0))
                shape.get().GetRigidBody().setLinearVelocity(v);

            if(Keyboard::isKeyPressed(Keyboard::Space))
                shape.get().GetRigidBody().setLinearVelocity(Vector3(0.0, 9.0, 0.0));

            if(shape.get().GetRigidBody().testAABBOverlap(sphere.get().GetRigidBody().getAABB()))
            {
                isAlive = false;
                shape.get().GetRigidBody().setLinearVelocity(Vector3(6.0, 15.0, 6.0));
                shape.get().GetRigidBody().setAngularVelocity(Vector3(6.0, 10.0, 6.0));
            }
        }
        else
        {
            Game::camera.AlwaysUp(false);
            Game::camera.SetOrientation(shape.get().GetOrientation());
        }
        Game::camera.SetPosition(shape.get().GetPosition());
    }

    private ShapePtr shape;
    private bool isAlive = true;
};

array<Player> players;

void Start()
{
    Game::manageCameraMovement = false;

    sphere = Game::scene.GetModel("sphere");

    for(int i = 0; i < 3; i++)
        players.insertLast(Player(Game::scene.GetShape("shape" + (i > 0 ? to_string(i) : ""))));
}

void Loop()
{
    for(int i = 0; i < 3; i++)
        if(Keyboard::isKeyPressed(Keyboard::Num1 + i))
            selectedPlayer = i;
    
    players[selectedPlayer].Move();

    if(Keyboard::isKeyPressed(Keyboard::R))
    {
        sphere.get().GetRigidBody().setIsActive(true);
        sphere.get().SetPosition(Vector3(10.0, 10.0, 10.0));
        sphere.get().GetRigidBody().setLinearVelocity(Vector3(0.0, 0.0, 0.0));
    }
}
