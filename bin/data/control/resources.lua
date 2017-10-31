--[[
Doc:
    Assets.CreateShader(shader name, fragmentShader, vertexShader, geometryShader (use "" for no geometry shader))
--]]

function f_load()
    --
    Assets.CreateShader("textured", "textured.fs", "defaultQuad/defaultQuad.vs", "")
    Assets.CreateShader("texturedCameraSpace", "textured.fs", "defaultQuad/defaultQuadCameraSpace.vs", "")

    Assets.CreateShader("texturedColor", "texturedColor.fs", "defaultQuad/defaultQuad.vs", "")
    Assets.CreateShader("texturedShaded", "texturedShaded.fs", "defaultQuad/defaultQuad.vs", "")
    
    Assets.CreateShader("staticQuadTexturedCameraSpace", "textured.fs", "defaultQuad/staticQuadCameraSpace.vs", "")


    Assets.CreateShader("colored", "colored.fs", "defaultQuad/defaultQuad.vs", "")
    Assets.CreateShader("coloredCameraSpace", "colored.fs", "defaultQuad/defaultQuadCameraSpace.vs", "")
    
    Assets.CreateShader("deferred", "deferred.fs", "deferred.vs", "")
    Assets.CreateShader("deferredPointLight", "deferredPointLight.fs", "deferredPointLight.vs", "")

    Assets.CreateShader("fbCopy", "fbCopy.fs", "deferred.vs", "")

    Assets.CreateShader("fragParticleDraw", "fragParticle/draw.fs", "fragParticle/draw.vs", "fragParticle/draw.gs")
    Assets.LoadShaders()
    --]]
end

return {load = f_load}
