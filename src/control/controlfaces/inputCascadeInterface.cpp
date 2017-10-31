
#include "inputCascadeInterface.hpp"
#include "window/window.hpp"
#include "window/inputCascade.hpp"

static int addLayer(InputCascade* ic)
{
    if (!ic)
        return -1;
    return ic->addLayer();
}

static void setStopFlow(InputCascade* ic, int layer, bool value)
{
    if (!ic)
        return;
    auto ptr = ic->getInputLayer(layer);
    if (!ptr)
        return;
    ptr->stopFlow = value;
}

static void setActive(InputCascade* ic, int layer, bool value)
{
    if (!ic)
        return;
    auto ptr = ic->getInputLayer(layer);
    if (!ptr)
        return;
    ptr->active = value;
}


luaL_Reg inputCascade_functions[] =
{
    {"AddLayer", LuaClosureWrap(addLayer, 1)},
    {"SetLayerStopFlow", LuaClosureWrap(setStopFlow, 1)},
    {"SetLayerActive", LuaClosureWrap(setActive, 1)},
    {0,0}
};
