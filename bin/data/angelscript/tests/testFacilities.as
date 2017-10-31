
bool EqualsDelta(double a, double b, double delta)
{
    double diff = a - b;

    if (diff <= delta && diff >= -delta)
        return true;
    return false;
}

namespace TestFacilities
{

[Test]
void TestDelta()
{
    Assert(EqualsDelta(30.0, 31.0, 0.5) == false);
    Assert(EqualsDelta(30.0, 31.0, 2.0) == true);

    Assert(EqualsDelta(30.0, 31.0, -0.5) == false);
    Assert(EqualsDelta(30.0, 31.0, -2.0) == false);

    Assert(EqualsDelta(31.0, 30.0, 0.5) == false);
    Assert(EqualsDelta(31.0, 30.0, 1.9) == true);

    Assert(EqualsDelta(31.0, 30.0, -0.5) == false);
    Assert(EqualsDelta(31.0, 30.0, -1.9) == false);

}


}
