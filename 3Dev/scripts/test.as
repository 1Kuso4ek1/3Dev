ModelPtr sphere;
array<ShapePtr> shapes;
int selectedShape = 0;

void Start()
{
    sphere = scene.GetModel("sphere");
    sphere.get().SetPosition(Vector3(0.0, 15.0, 0.0));

    shapes.resize(3);

    for(int i = 0; i < 3; i++)
        shapes[i] = scene.GetShape("shape" + (i > 0 ? to_string(i) : ""));
}

void Loop()
{
    for(int i = 0; i < 3; i++)
        if(Keyboard::isKeyPressed(Keyboard::Num1 + i))
            selectedShape = i;

    if(Keyboard::isKeyPressed(Keyboard::Space))
        shapes[selectedShape].get().Move(Vector3(0.0, 5.0, 0.0));
}
