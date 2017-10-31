
/*
    Basically the unittest tests
*/

[Test]
void AlwaysSuccessTest()
{
}

[Test]
void Vector2SanityCheck()
{
    Vector2 a, b;
    a = Vector2(0,4);
    b = Vector2(3,2);

    auto c = a + b;

    Assert(c.x == a.x + b.x);
    Assert(c.y == a.y + b.y);
}


namespace GameVar
{

/*
    Tests for the engine GameVar getter
*/

void CheckVarExistence(string &in varname)
{
    hash_t h = Hash(varname);

    GameVar@ var = GameVar::GetVar(h);
    Assert(var !is null);

    GameVar@ var2 = GameVar::GetVar(varname);
    Assert(var2 !is null);

    Assert(var is var2);
}

/*
    GameVar .Exit is internal engine variable: it is used to exit the program
*/

[Test]
void ExitVarCheck()
{
    CheckVarExistence(".Exit");
}

/*
    GameVar .FullRestart is internal engine variable: it is used to perform
    full program reinitialization
*/

[Test]
void FullRestartVarCheck()
{
    CheckVarExistence(".FullRestart");
}

}
