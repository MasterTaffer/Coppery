#pragma once

enum SceneGraphOrder
{
    SceneGraphMesh = 1000,

    SceneGraphFillLight = 2000,
    SceneGraphLights = 2100,

    SceneGraphLightlessMesh = 2500,

    SceneGraphAlpha = 3000,
    SceneGraphAdditive = 3100,

    SceneGraphUI = 4000,
    
    SceneGraphPostProcess = 5000
};
