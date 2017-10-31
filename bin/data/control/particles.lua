--Define and load all the particle systems


function f_load()

local psystem = {}
psystem.name = "sparks"
psystem.layer = NodeOrder.Additive
psystem.drawType = 0
psystem.updateShader = {fragment = "dummy.fs", vertex = "fragParticle/update.vs", geometry = "fragParticle/update.gs"}
psystem.particleLayout = {}
table.insert(psystem.particleLayout, {size = 2, binding = ParticleLayout.Position})
table.insert(psystem.particleLayout, {size = 2, binding = ParticleLayout.Velocity})
table.insert(psystem.particleLayout, {size = 1, binding = ParticleLayout.Random, randomMin = 0.0, randomMax = 50.0})

psystem.feedbackVaryings = {"out_position", "out_velocity", "out_time"}

local regsystem = ParticleSystem.Register(psystem)

ParticleSystem.AddDrawPass(regsystem, Assets.GetShader("fragParticleDraw"), NodeOrder.Additive)

end

return {load = f_load}
