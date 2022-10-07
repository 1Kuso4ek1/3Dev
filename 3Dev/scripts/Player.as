ModelPtr sphere;

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
            Game::manageCameraLook = true;
            Game::manageCameraMouse = true;
            Vector3 v = Game::camera.Move(1.0); v *= 8.0;
            if(v != Vector3(0.0, 0.0, 0.0))
                shape().GetRigidBody().setLinearVelocity(v);

            if(Keyboard::isKeyPressed(Keyboard::Space))
                shape().GetRigidBody().setLinearVelocity(Vector3(0.0, 9.0, 0.0));

            if(shape().GetRigidBody().testAABBOverlap(sphere().GetRigidBody().getAABB()))
            {
                isAlive = false;
                shape().GetRigidBody().setLinearVelocity(Vector3(6.0, 15.0, 6.0));
                shape().GetRigidBody().setAngularVelocity(Vector3(6.0, 6.0, 6.0));
            }
            Game::camera.SetPosition(shape().GetPosition());
        }
        else
        {
            Game::manageCameraLook = false;
            Game::manageCameraMouse = false;
            Game::camera.Look(shape().GetPosition());
        }
    }

    private ShapePtr shape;
    private bool isAlive = true;
};
