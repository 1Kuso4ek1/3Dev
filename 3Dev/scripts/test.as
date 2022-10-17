#include 'Player.as'

int selectedPlayer = 0;
bool sphereHavePhysics = false;

array<Player> players;

void Start()
{
    Game::manageCameraMovement = false;

    sphere = Game::scene.GetModel("sphere");
    sphere().GetRigidBody().setIsActive(false);

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
        sphere().GetRigidBody().setIsActive(true);
        sphere().SetPosition(Vector3(0.0, 40.0, 0.0));
        sphere().GetRigidBody().setLinearVelocity(Vector3(0.0, 0.0, 0.0));
    }
}
