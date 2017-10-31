
Util = require("core.util")
Vars = require("core.vars")


Application.SetWindowIcon({
	"assets/icon/16.png",
	"assets/icon/32.png",
	"assets/icon/48.png",
	"assets/icon/64.png",
	"assets/icon/128.png"})


-- Initialize all the InputLayers used in the application
-- One input layer is automatically created by the engine

local inputLayerCnt = 2

-- So we end up with (inputLayerCnt + 1) InputLayers

for i = 1,inputLayerCnt do
    InputCascade.AddLayer()
end

-- InputLayers
-- 0: extremely low level, only used for stuff like Escape, Backspace etc.
-- 1: low level
-- 2: Game level

require("resources").load();

require("scene").load();

require("console").load();
require("particles").load();

--[[

-- Testing functionality

local csv = require("core.csv")
local vlist = csv.parse("assets/def/sounds.sound")

for k,v in ipairs(vlist) do
	local str = tostring(k) .. ": "

	for k2, v2 in ipairs(v) do
		str = str .. " " .. v2
	end
	print (str)
end

-- recursively search directory for files with extension
-- extension "" means all files
list = File.DirEntries("user/config", "")
for k,v in ipairs(list) do
	
	if not File.Exists(v) then
		print (v .. " DOESN'T EXIST????")
	else
		print (v .. " exists")
	end
end
--]]


-- Initialize a few GameVars used in the Engine

-- How many collision passes to do in the collision detection
-- Will prevent fast objects moving through other things sometimes
GameVar.NewInteger("Collision.Passes", 1)


-- At what point the engine starts sleeping using a spinlock.
-- The value is in microseconds: if the time to sleep is greater than the
-- value, the system sleep function is used instead.
GameVar.NewInteger("Engine.SpinlockThreshold", 1500)

-- Engine ticks per second, determines the delta time
GameVar.NewInteger("Engine.Tickrate", 60)

-- Forces engine step rate to equal the tick rate, effectively keeping delta
-- time at 1.0. However, this may vary due to frame pacing issues.
GameVar.NewInteger("Engine.ForceTickRate", 1)

-- Forces the delta time to one regardless of time passed between engine ticks
GameVar.NewInteger("Engine.ForceDeltaTimeToOne", 1) 


-- The time multiplier, can be used as a simple way to slow things down
GameVar.NewNumber("Engine.TimeMultiplier", 1.0)

-- Scripts recompilation behaviour
-- 	0: never compile scripts, always load bytecode
--	1: compile scripts when no bytecode is present
-- 	2: always compile scripts
GameVar.NewIntegerLimits("Script.Compile", 2, 0, 2);

-- Custom GameVars

-- Used to signify that a successful initialization occurred
-- If the init.lua crashes or fails to compile this will never be set
GameVar.NewInteger("CL.SuccessfulInit", 1);



-- Tilemap layer brightness
GameVar.NewNumber("Map.FgShade", 1.0)
GameVar.NewNumber("Map.BgShade", 0.3)

-- Tilemap layer rendering depths
GameVar.NewNumber("Map.BgDepth", 65)
GameVar.NewNumber("Map.FgDepth", 15)
GameVar.NewNumber("Map.TfgDepth", -15)


