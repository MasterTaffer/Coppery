
function f_load()

function ripairs(t)
    local max = 1
    while t[max] ~= nil do
        max = max + 1
        end
        local function ripairs_it(t, i)
            i = i-1
            local v = t[i]
            if v ~= nil then
                return i,v
            else
                return nil
        end
    end
    return ripairs_it, t, max
end


NodeOrder.Console = 5700
local node = GraphicsNode.New(NodeOrder.Console)

local consoleActive = false
local consoleSound = Assets.GetSound("beep1")

local gv = GameVar.NewNumberHandle("ConsoleActive", 0, 1, 0, function (val)
    if (val == 0) then
        consoleActive = false
        InputCascade.SetLayerStopFlow(1, false)
    else
        consoleActive = true
        InputCascade.SetLayerStopFlow(1, true)
        Sound.Play(consoleSound)
    end
    return true
end)

local consoleFont = Assets.GetFont("fontMain")
local consoleFontText = Assets.GetTexture("fontMainTex")
local fontDimensions = GraphicsDrawing.GetFontDimensions(consoleFont)
local consoleFontHeight = fontDimensions[2] / GameVar.GetNumber(GameVar.Get("Graphics.RenderScale"))
local consoleFontWidth = fontDimensions[1] / GameVar.GetNumber(GameVar.Get("Graphics.RenderScale"))
local consoleShader = Assets.GetShader("texturedCameraSpace")

local consoleMessages = {}
local wsize = Graphics.GetScreenSize()
local consoleMessagesPerScreen = math.floor(wsize[2] / consoleFontHeight)
local consoleMessageWidth = math.floor(wsize[1] / consoleFontWidth) - 1

if consoleMessagesPerScreen >= 200 then
	consoleMessagesPerScreen = 200
end
if consoleMessageWidth >= 800 then
	consoleMessageWidth = 800
end


local consoleMessageBuffersOffset = 1;
local consoleMessageBuffers = {}
local newMessages = 0;
for i = 1,consoleMessagesPerScreen do
    local buf = GraphicsDrawing.NewBuffer()
    buf:setUsage(GL.DynamicDraw)
    -- buf:setTarget()
    -- buf.reserve(24 * 80);
    table.insert(consoleMessageBuffers, {buffer= buf, size= 0})
end

local maxConsoleMessages = consoleMessagesPerScreen

local commandHistory = {}
local commandHistoryIndex = 1
local currentCommand = 1

local consoleInput = ""
local consoleInputIndex = 1
local nodeTraverse = function (self)
    if consoleActive == false then
        return
    end

    if newMessages > 0 then
        if newMessages >= consoleMessagesPerScreen then
            newMessages = consoleMessagesPerScreen;
        end
        local start = (#consoleMessages) - newMessages;
        if start < 0 then
            start = 0
        end
        local buf

        local bufof = consoleMessageBuffersOffset
        for i = 1, newMessages do

            
            local buffer = GraphicsDrawing.GenerateTextBuffer(consoleFont, consoleMessages[start + i], 1.0)
            
            consoleMessageBuffers[bufof].buffer:load(buffer, 0)
            consoleMessageBuffers[bufof].size = #(consoleMessages[start + i]) * 6
            bufof = bufof + 1
            
            if bufof > consoleMessagesPerScreen then
                bufof = 1
            end
        end
        consoleMessageBuffersOffset = bufof
        newMessages = 0;
    end

    GraphicsDrawing.WithShader(Assets.GetShader("coloredCameraSpace"), function (s)
        local uid = Assets.GetShaderUniform(s, "FadeColor")
        GraphicsDrawing.SetUniform4f(uid, 0.1, 0, 0.1, 1.0)
        GraphicsDrawing.DrawQuad({wsize[1]/2, wsize[2]/2}, {wsize[1] , wsize[2] }, 0, 0)
    end)


    local padding = 4;

    local y = wsize[2] - consoleFontHeight * 2
    local scale = GameVar.GetNumber(GameVar.Get("Graphics.RenderScale"))
    
    GraphicsDrawing.WithShader(consoleShader, function (s)
        local uid = Assets.GetShaderUniform(s, "Size")
        
        
        GraphicsDrawing.SetUniform2f(uid, 2 / scale, 2 / scale)
        uid = Assets.GetShaderUniform(s, "Depth")
        GraphicsDrawing.SetUniform1f(uid, 0)
        uid = Assets.GetShaderUniform(s, "Rotation")
        GraphicsDrawing.SetUniform1f(uid, 0)
        uid = Assets.GetShaderUniform(s, "Position")

        GraphicsDrawing.BindTexture(consoleFontText)

        local bufof = consoleMessageBuffersOffset - 1
        for i = 1, consoleMessagesPerScreen do
            if bufof <= 0 then
                bufof = consoleMessagesPerScreen
            end

            GraphicsDrawing.SetUniform2f(uid, padding / 2, y)

            local bufstr = consoleMessageBuffers[bufof]
            GraphicsDrawing.DrawBuffer(bufstr.buffer, bufstr.size, 0)
            bufof = bufof - 1
            y = y - consoleFontHeight
            if y < 0 then  
                break
            end
        end
        
    end)
    

    GraphicsDrawing.DrawText(consoleFont, consoleInput:sub(1, consoleMessageWidth), {padding / 2, wsize[2] - consoleFontHeight}, 0, 1.0 / scale, 0, false);
    GraphicsDrawing.DrawText(consoleFont, "_", {padding / 2 + consoleFontWidth * (consoleInputIndex - 1) , wsize[2] - consoleFontHeight}, 0, 1.0 / scale, 0, false);
end





GraphicsNode.SetShader(node, Assets.GetShader("staticQuadTexturedCameraSpace"))

--[[ use blending, SFactor, DFactor, Equation, use depth buffer --]]
GraphicsNode.SetShadingSettings(node, false, GL.SrcAlpha, GL.OneMinusSrcAlpha, GL.FuncAdd, false)
--[[ GraphicsNode.SetShadingSettings(node, GL.One, GL.Zero, GL.FuncAdd, false) --]]
GraphicsNode.SetTraverseMethod(node, nodeTraverse)

local insertConsoleLog = function (msg)
    
    local insertRow = function (row)
        newMessages = newMessages + 1;
        table.insert(consoleMessages, row)
        if #consoleMessages > maxConsoleMessages then
            table.remove(consoleMessages, 1);
        end
    end
    local idx = 1
    local msglen = #msg
    local first = true

    while idx <= msglen do
        local maxlen = consoleMessageWidth
        if not first then
            maxlen = maxlen - 4
        end
        local s, e
        
        if idx + maxlen < msglen then
            e = idx
            while true do
                local ns, ne = msg:find("%s+", e + 1)
                if ns == nil or ns > idx + maxlen then
                    break
                end
                s = ns
                e = ne
            end
        end

        if s == nil or e <= idx then

            if first then
                insertRow(msg:sub(idx, idx + maxlen))
            else
                insertRow(" ~  " .. msg:sub(idx, idx + maxlen))
            end
            idx = idx + maxlen + 1
        else
            if first then
                insertRow(msg:sub(idx, s - 1))
            else
                insertRow(" ~  " .. msg:sub(idx, s - 1))
            end
            idx = e + 1
        end
        first = false
    end
end

Control.SetMessageCallback( insertConsoleLog )

-- Input handlers for the text input meta keys
-- enter, arrows, backspace, home, end...


Control.AddInputHandler("sys_enter",0,function ()
    if string.len(consoleInput) == 0 then
        return
    end
    if consoleActive then
        
        print (consoleInput)
        consoleInputIndex = 1
        
        commandHistory[currentCommand] = consoleInput
        
        local firstChar = string.sub(consoleInput, 1, 1)
        
        
        -- If the string starts with a bang, treat is as an AngelScript
        -- command
        -- If the first character is a $, use the special GameVar syntax
        if firstChar== "!" then
            -- "!!""
            if #consoleInput > 1 and consoleInput:sub(2, 2) == "!" then
                consoleInput = string.sub(consoleInput, 3)
                GameControlCommand(consoleInput)
            else
                consoleInput = string.sub(consoleInput, 2)
                ScriptEngine.ExecuteString(consoleInput);
            end
        elseif firstChar == "$" then
            consoleInput = string.sub(consoleInput, 2)
            
            local i = 0
            local varname = {}
            local fb, fe = consoleInput:find("[^ =]+")
            
            if fb == nil then
                print("???")
                
            else
                local varname = consoleInput:sub(fb, fe)
                local nfb, nfe = consoleInput:find("=", fe + 1)
                if nfb ~= nil then
                    fe = nfe
                end
                nfb, nfe = consoleInput:find("[^ =]+", fe + 1)
                local send = ""
                
                if nfb ~= nil then
                    send = consoleInput:sub(nfb, nfe)
                end
                
                if string.len(send) == 0 then
                    local v = Vars[varname]
                    print(tostring(v))
                else
                    Vars[varname] = send
                    local newval = Vars[varname] 
                    print(varname .. " <- " .. newval)
                end
            end
            
            
            
        else
            local noreturn = true
            -- dont try to return the value, just run
            if firstChar == "=" then
                noreturn = false
                consoleInput = string.sub(consoleInput, 2)
            end
            
            if noreturn == false then
                consoleInput = "return " .. consoleInput
            end
            local fun, e = load(consoleInput)
            if fun == nil then
                print(e)
            else
                local status, res = pcall(fun)
                if status then
                    if not noreturn then
                        print(tostring(res))
                    end
                else
                    -- error occurred
                    print(res)
                end
            end
        end
        consoleInput = ""
        currentCommand = currentCommand + 1
        commandHistoryIndex = currentCommand
        commandHistory[currentCommand] = ""
    end
end)

Control.AddInputHandler("sys_tab",0,function ()
    if consoleActive then
        if consoleInputIndex > 1 and consoleInput:sub(1,1) == "$" then
            
            local fb, fe = consoleInput:find("[^ =]+", 2)            
            if fb == nil then
                fb = 2
                fe = 1
            end
            local res = GameVar.GetPrefixedMatch(consoleInput:sub(fb, fe))
            if #res > 0 then
                local longest = res[1] .. " "
                insertConsoleLog("")
                insertConsoleLog(consoleInput .. " ...")
                for k, v in ipairs(res) do
                    insertConsoleLog(v)
                    if k ~= 1 then
                        for j = 1, #longest do
                            if longest:sub(j,j) ~= v:sub(j,j) then
                                longest = longest:sub(1,j - 1)
                                break
                            end
                        end
                    end
                end
                consoleInput = "$" .. longest
                consoleInputIndex = consoleInput:len() + 1
                
            else
                insertConsoleLog("")
                insertConsoleLog(consoleInput .. " ... No matches found")
            end    
        end
    end
end)



Control.AddInputHandler("sys_backspace",0,function ()
    if consoleActive then
       consoleInputIndex = consoleInputIndex - 1
       if consoleInputIndex < 1 then
           consoleInputIndex = 1
       else
            consoleInput = string.sub(consoleInput, 1, consoleInputIndex - 1) .. string.sub(consoleInput, consoleInputIndex + 1)
       end
    end
end)


Control.AddInputHandler("sys_delete",0,function ()
    if consoleActive then
        if consoleInputIndex ~= string.len(consoleInput) + 1 then
            consoleInput = string.sub(consoleInput, 1, consoleInputIndex - 1) .. string.sub(consoleInput, consoleInputIndex + 1)
        end
    end
end)


Control.AddInputHandler("sys_up",0,function ()
    if consoleActive then
        commandHistoryIndex = commandHistoryIndex - 1
        if commandHistoryIndex < 1 then
            commandHistoryIndex = 1
        else
            consoleInput = commandHistory[commandHistoryIndex]
            consoleInputIndex = string.len(consoleInput) + 1
        end
    end
end)
Control.AddInputHandler("sys_down",0,function ()
    if consoleActive then
        commandHistoryIndex = commandHistoryIndex + 1
        if commandHistoryIndex > currentCommand then
            commandHistoryIndex = currentCommand
        else
            local temp = commandHistory[commandHistoryIndex]
            if temp ~= nil then
                consoleInput = temp
                consoleInputIndex = string.len(consoleInput) + 1
            end
        end
    end
end)

Control.AddInputHandler("sys_left",0,function ()
    if consoleActive then
       consoleInputIndex = consoleInputIndex - 1
       if consoleInputIndex < 1 then
           consoleInputIndex = 1
       end
    end
end)
Control.AddInputHandler("sys_right",0,function ()
    if consoleActive then
       consoleInputIndex = consoleInputIndex + 1
       if consoleInputIndex > string.len(consoleInput) + 1 then
           consoleInputIndex = string.len(consoleInput) + 1
       end
    end
end)
Control.AddInputHandler("sys_home",0,function ()
    if consoleActive then
       consoleInputIndex = 1
    end
end)

Control.AddInputHandler("sys_end",0,function ()
    if consoleActive then
        consoleInputIndex = string.len(consoleInput) + 1
    end
end)
Control.AddStepCallback( function ()
    if consoleActive then
        local characters = Application.GetCharacterInput()
        local input = ""
        for i, v in ipairs(characters) do
            if v < 128 then
                input = input .. string.char(v)
            end
        end
        if string.len(input) > 0 then
            if string.len(consoleInput) == 0 then
                consoleInput = input
            elseif consoleInputIndex == 1 then
                consoleInput = input .. consoleInput
            elseif consoleInputIndex == string.len(consoleInput) + 1 then
                consoleInput = string.sub(consoleInput, 1, consoleInputIndex - 1) .. input
            else
                consoleInput = string.sub(consoleInput, 1, consoleInputIndex - 1) .. input .. string.sub(consoleInput, consoleInputIndex)
            end
            consoleInputIndex = consoleInputIndex + string.len(input)
            commandHistory[currentCommand] = consoleInput
        end
    end
end)


end

return {load = f_load}
