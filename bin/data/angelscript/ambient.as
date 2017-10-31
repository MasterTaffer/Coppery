

void ConstrainCameraPosition()
{
    Vector2 ms = WorldSize;
    Vector2 pos = WorldSize / 2.0;

    SetCamera(pos);
    GV_FillLight_R.set(1.0);
    GV_FillLight_G.set(1.0);
    GV_FillLight_B.set(1.0);
    GV_FillLight_Power.set(200000);
    GV_FillLight_Height.set(400);
    GV_SpecularIntensity.set(1.0);
    GV_SpecularPower.set(1.0);
    GV_FillLight_X.set(pos.x);
    GV_FillLight_Y.set(pos.y);
}

GameVar@ GV_FillLight_X = GameVar::GetVar("Effect.FillLight.X");
GameVar@ GV_FillLight_Y = GameVar::GetVar("Effect.FillLight.Y");

GameVar@ GV_FillLight_R = GameVar::GetVar("Effect.FillLight.R");
GameVar@ GV_FillLight_G = GameVar::GetVar("Effect.FillLight.G");
GameVar@ GV_FillLight_B = GameVar::GetVar("Effect.FillLight.B");

GameVar@ GV_FillLight_Power = GameVar::GetVar("Effect.FillLight.Power");
GameVar@ GV_FillLight_Height = GameVar::GetVar("Effect.FillLight.Height");

GameVar@ GV_SpecularIntensity = GameVar::GetVar("Effect.SpecularIntensity");
GameVar@ GV_SpecularPower = GameVar::GetVar("Effect.SpecularPower");

