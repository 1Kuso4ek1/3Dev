array<ShapePtr> shapes;
ModelPtr sphere;
int selectedShape = 0;
bool sphereHavePhysics = false;

void Start()
{
    Game::manageCameraMovement = false;

    sphere = Game::scene.GetModel("sphere");

    for(int i = 0; i < 3; i++)
        shapes.insertLast(Game::scene.GetShape("shape" + (i > 0 ? to_string(i) : "")));
}

void Loop()
{
    for(int i = 0; i < 3; i++)
        if(Keyboard::isKeyPressed(Keyboard::Num1 + i))
            selectedShape = i;

    Vector3 v = Game::camera.Move(1.0); v *= 8.0;
    if(v != Vector3(0.0, 0.0, 0.0))
        shapes[selectedShape].get().GetRigidBody().setLinearVelocity(v);
    Game::camera.SetPosition(shapes[selectedShape].get().GetPosition());

    if(Keyboard::isKeyPressed(Keyboard::Space))
    {
        shapes[selectedShape].get().GetRigidBody().setLinearVelocity(Vector3(0.0, 9.0, 0.0));
    }

    if(Keyboard::isKeyPressed(Keyboard::R))
    {
        sphere.get().GetRigidBody().setIsActive(true);
        sphere.get().SetPosition(Vector3(10.0, 10.0, 10.0));
        sphere.get().GetRigidBody().setLinearVelocity(Vector3(0.0, 0.0, 0.0));
    }
}
