ModelPtr sphere;

void Start()
{
    sphere = scene.GetModel("sphere");
}

void Loop()
{
    sphere.get().Move(Vector3(0.1, 0.1, 0.1));
}
