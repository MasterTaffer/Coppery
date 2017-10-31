
namespace Random
{

/*
    Testcases for the (deterministic) pseudorandom number generator

*/

[Test]
void RandomSeededTest()
{
    RandomGenerator a, b;

    a.seed(24);
    b.seed(24);
    for (int i = 0; i < 10; i++)
    {
        Assert(a.getI() == b.getI());
        Assert(a.getU() == b.getU());
    }
}

[Test]
void RandomCopy()
{
    RandomGenerator a;
    a.seed(24);
    a.getI();
    a.getI();
    a.getI();

    RandomGenerator b = a;
    Assert(a.getI() == b.getI());
    Assert(a.getI() == b.getI());
    Assert(a.getI() == b.getI());
    Assert(a.getI() == b.getI());
}


[Test]
void RandomLongSeed()
{
    uint[] lseed1 = {2,5,7,8,9,1,2,5,4};

    RandomGenerator a;
    a.seed(2345);
    a.seed(lseed1);
    int v1 = a.getI();
    int v2 = a.getI();
    int v3 = a.getI();
    
    uint[] lseed2 = {2,5,7,8,9,1,2,5,4};
    RandomGenerator b;
    b.seed(1234);
    b.seed(lseed2);
    Assert(v1 == b.getI());
    Assert(v2 == b.getI());
    Assert(v3 == b.getI());
}

}
