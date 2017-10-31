Vars = require("core.vars")

local i = Vars["CL.SuccessfulInit"]
if i ~= 1 then
    print("CL.SuccessfulInit ~= 1, Exiting...")
    Vars[".Exit"] = 1
    return
end

InitScriptEngine = function()
    ScriptEngine.CallInitFunctions()
    Game.On()
end

GameIsPaused = false

PauseGame = function ()
    GameIsPaused = true
    Game.SetIsPaused(true)
end

UnpauseGame = function ()
    GameIsPaused = false
    Game.SetIsPaused(false)
end

TogglePaused = function ()
    if Vars["ConsoleActive"] ~= 0 then
        return
    end
    
    if not GameIsPaused then
        PauseGame()
    else
        UnpauseGame()
    end
end

PrintPerf = function ()
    
    local reverseNodes = {}
    for k, v in pairs(NodeOrder) do
        reverseNodes[v] = k
    end
    
    local perf = GraphicsDrawing.QueryPerformance()
    
    for k, v in pairs(perf) do
        
        local n = reverseNodes[k]
        if n == nil then
            n = tostring(k)
        end
        
        print (n .. " - " .. tostring(v) .. "ns")
        
    end
end

ToggleConsole = function ()
    if Vars["ConsoleActive"] == 0 then
        PauseGame()
        Vars["ConsoleActive"] = 1
    end
end

EscapePressed = function ()
    if Vars["ConsoleActive"] == 1 then
        UnpauseGame()
        Vars["ConsoleActive"] = 0
    end
end

NewGame = function ()
    Game.Restart()
    InitScriptEngine()
    Game.On()
end

local cnt = false

for k, v in pairs(Args) do
    if cnt == false then
        print ("Args: ")
        cnt = true
    end
    if #v == 0 then
        print ("    " .. k)
    else
        print ("    " .. k .. " = '" .. v .. "'")
    end
end

-- Performance statistic printer
Control.AddInputHandler("sys_backspace",1,PrintPerf)

Control.AddInputHandler("sys_escape",0,EscapePressed)

Control.AddInputHandler("developerConsole",1,ToggleConsole)
Control.AddInputHandler("pause",1,TogglePaused)
Control.AddInputHandler("newgame",1,NewGame)
Control.AddInputHandler("quit",1, function () Application.Close() end)

InitScriptEngine()
