

namespace Graphics
{

[Test]
void TestCamera()
{
    Vector2 camPos = {4, 60};
    SetCamera(camPos);

    Vector2 newPos = GetCamera();

    Assert(EqualsDelta(camPos.x, newPos.x, 0.0001));
    Assert(EqualsDelta(camPos.y, newPos.y, 0.0001));
}

}
