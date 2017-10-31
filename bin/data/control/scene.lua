
function f_load()

-- add scene layers

local lightSpecularPower = GameVar.NewNumber("Effect.SpecularPower", 4.0)
local lightSpecularIntensity = GameVar.NewNumber("Effect.SpecularIntensity", 0.5)
    
    
NodeOrder.UserLight = NodeOrder.Lights + 50
--[[ add new node to scene graph at depth ScenePostProcessing - 1 --]]
local node = GraphicsNode.New(NodeOrder.UserLight)

local nodeTraverse = function (self)
    
    local tex = GraphicsDrawing.GetFramebufferTexture()
    local depth = GraphicsDrawing.GetFramebufferDepthTexture()

    GraphicsDrawing.BindTexture(tex)
    GraphicsDrawing.BindTextureToUnit(depth, 3)

    GraphicsNode.Traverse(self)
end
--[[ use blending, SFactor, DFactor, Equation, use depth buffer --]]
GraphicsNode.SetShadingSettings(node, true, GL.SrcAlpha, GL.One, GL.FuncAdd, false)
GraphicsNode.SetTraverseMethod(node, nodeTraverse)


-- Replacement for hardcoded point lights node
local nodeLights = GraphicsNode.New(NodeOrder.Lights)

local pointLightShader = Assets.GetShader("deferredPointLight")
local pointLightShader_SpecularData = Assets.GetShaderUniform(pointLightShader, "SpecularData")

local nodeLightsTraverse = function (self)

    GraphicsDrawing.WithShader(pointLightShader, function(shader)
        local tex = GraphicsDrawing.GetFramebufferTexture()
        local depth = GraphicsDrawing.GetFramebufferDepthTexture()
        
        local spower = GameVar.GetNumber(lightSpecularPower)
        local sint = GameVar.GetNumber(lightSpecularIntensity)
        
        GraphicsDrawing.SetUniform2f(pointLightShader_SpecularData, spower, sint)
        GraphicsDrawing.BindTexture(tex)
        GraphicsDrawing.BindTextureToUnit(depth, 3)

        GraphicsNode.Traverse(self)
    end)    
end

--[[ use blending, SFactor, DFactor, Equation, use depth buffer --]]
GraphicsNode.SetShadingSettings(nodeLights, true, GL.SrcAlpha, GL.One, GL.FuncAdd, false)
GraphicsNode.SetTraverseMethod(nodeLights, nodeLightsTraverse)


local nodeFillLights = GraphicsNode.New(NodeOrder.FillLight)

local fillLightShader = Assets.GetShader("deferred")
local fillLightShader_SpecularData = Assets.GetShaderUniform(fillLightShader, "SpecularData")
local fillLightShader_LightColor = Assets.GetShaderUniform(fillLightShader, "LightColor")
local fillLightShader_LightPower = Assets.GetShaderUniform(fillLightShader, "LightPower")
local fillLightShader_LightPos = Assets.GetShaderUniform(fillLightShader, "LightPos")
local fillLightShader_FOVCaster = Assets.GetShaderUniform(fillLightShader, "FOVCaster")



local fillX = GameVar.NewNumber("Effect.FillLight.X", 0.0)
local fillY = GameVar.NewNumber("Effect.FillLight.Y", 0.0)
local fillR = GameVar.NewNumber("Effect.FillLight.R", 1.0)
local fillG = GameVar.NewNumber("Effect.FillLight.G", 1.0)
local fillB = GameVar.NewNumber("Effect.FillLight.B", 1.0)
local fillPower = GameVar.NewNumber("Effect.FillLight.Power", 0.0)
local fillHeight = GameVar.NewNumber("Effect.FillLight.Height", 1.0)

local nodeFillLightTraverse = function (self)
    GraphicsDrawing.BindFramebuffer(1)
    
    GraphicsDrawing.WithShader(fillLightShader, function(shader)
        local tex = GraphicsDrawing.GetFramebufferTexture()
        local depth = GraphicsDrawing.GetFramebufferDepthTexture()
        GraphicsDrawing.BindTexture(tex)
        GraphicsDrawing.BindTextureToUnit(depth, 3)

        local spower = GameVar.GetNumber(lightSpecularPower)
        local sint = GameVar.GetNumber(lightSpecularIntensity)
        
        GraphicsDrawing.SetUniform2f(pointLightShader_SpecularData, spower, sint)
        
        local r = GameVar.GetNumber(fillR)
        local g = GameVar.GetNumber(fillG)
        local b = GameVar.GetNumber(fillB)
        
        local x = GameVar.GetNumber(fillX)
        local y = GameVar.GetNumber(fillY)
        
        local power = GameVar.GetNumber(fillPower)
        local height = GameVar.GetNumber(fillHeight)
        
        GraphicsDrawing.SetUniform3f(fillLightShader_LightColor, r, g, b)
        GraphicsDrawing.SetUniform1f(fillLightShader_LightPower, power)
        GraphicsDrawing.SetUniform3f(fillLightShader_LightPos, 0.0, 0.0, height)
        GraphicsDrawing.SetUniform2f(fillLightShader_FOVCaster, x, y)
        GraphicsDrawing.DrawFramebuffer()
    end)    
end

--[[ use blending, SFactor, DFactor, Equation, use depth buffer --]]
GraphicsNode.SetShadingSettings(nodeFillLights, true, GL.SrcAlpha, GL.Zero, GL.FuncAdd, false)
GraphicsNode.SetTraverseMethod(nodeFillLights, nodeFillLightTraverse)


local nodePostProcess = GraphicsNode.New(NodeOrder.PostProcess)

local fbCopyShader = Assets.GetShader("fbCopy")



local nodePostProcessTraverse = function (self)
    GraphicsDrawing.BindFramebuffer(0)
    
    local dim = Application.GetOutputDimensions();
    
    local shaderToUsePP = fbCopyShader
    
    GraphicsDrawing.SetViewport(0,0, dim[1], dim[2])
    
    GraphicsDrawing.WithShader(shaderToUsePP, function(shader)
        local tex = GraphicsDrawing.GetPPFramebufferTexture()
        local depth = GraphicsDrawing.GetFramebufferDepthTexture()
        GraphicsDrawing.BindTexture(tex)
        GraphicsDrawing.BindTextureToUnit(depth, 3)

        GraphicsDrawing.DrawFramebuffer()
    end)    
end

--[[ use blending, SFactor, DFactor, Equation, use depth buffer --]]
GraphicsNode.SetShadingSettings(nodePostProcess, true, GL.One, GL.Zero, GL.FuncAdd, false)
GraphicsNode.SetTraverseMethod(nodePostProcess, nodePostProcessTraverse)


local genericNodeTraverse = GraphicsNode.Traverse

local nodeAdditive = GraphicsNode.New(NodeOrder.Additive)
GraphicsNode.SetShadingSettings(nodeAdditive, true, GL.SrcAlpha, GL.One, GL.FuncAdd, false)
GraphicsNode.SetTraverseMethod(nodeAdditive, genericNodeTraverse)

local nodeAlpha = GraphicsNode.New(NodeOrder.Alpha)
GraphicsNode.SetShadingSettings(nodeAlpha, true, GL.SrcAlpha, GL.OneMinusSrcAlpha, GL.FuncAdd, false)
GraphicsNode.SetTraverseMethod(nodeAlpha, genericNodeTraverse)

local nodeMesh = GraphicsNode.New(NodeOrder.Mesh)
GraphicsNode.SetShadingSettings(nodeMesh, false, GL.One, GL.Zero, GL.FuncAdd, true)
GraphicsNode.SetTraverseMethod(nodeMesh, genericNodeTraverse)

local nodeMesh = GraphicsNode.New(NodeOrder.LightlessMesh)
GraphicsNode.SetShadingSettings(nodeMesh, false, GL.One, GL.Zero, GL.FuncAdd, true)
GraphicsNode.SetTraverseMethod(nodeMesh, genericNodeTraverse)

local nodeMesh = GraphicsNode.New(NodeOrder.UI)
GraphicsNode.SetShadingSettings(nodeMesh, false, GL.One, GL.Zero, GL.FuncAdd, true)
GraphicsNode.SetTraverseMethod(nodeMesh, genericNodeTraverse)


end

return {load = f_load}
