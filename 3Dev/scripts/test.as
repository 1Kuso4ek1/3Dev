array<ShapePtr> shapes;
int selectedShape = 0;

void Start()
{
    Game::manageCameraMovement = false;

    for(int i = 0; i < 3; i++)
        shapes.insertLast(Game::scene.GetShape("shape" + (i > 0 ? to_string(i) : "")));
}

void Loop()
{
    for(int i = 0; i < 3; i++)
        if(Keyboard::isKeyPressed(Keyboard::Num1 + i))
            selectedShape = i;

    Vector3 v = Game::camera.Move(1.0); v *= 5.0;
    if(v != Vector3(0.0, 0.0, 0.0))
        shapes[selectedShape].get().GetRigidBody().setLinearVelocity(v);
    Game::camera.SetPosition(shapes[selectedShape].get().GetPosition());

    if(Keyboard::isKeyPressed(Keyboard::Space))
        shapes[selectedShape].get().GetRigidBody().setLinearVelocity(shapes[selectedShape].get().GetRigidBody().getLinearVelocity() + Vector3(0.0, 5.0, 0.0));
}
