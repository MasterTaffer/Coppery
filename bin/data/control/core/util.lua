
local f_trim = function (string)
	return string:gsub("^%s+",""):gsub("%s+$","")
end

local f_split = function (input, splitter)
	local out = {}
	local idx = 1
	while true do
		local s, e = input:find(splitter, idx)
		if s == nil then
			table.insert(out, input:sub(idx))
			break
		else
			table.insert(out, input:sub(idx, s - 1))
			idx = e + 1
		end
	end
	return out
end

local f_grep = function (input, string)
	local outstring = false
	if type(input) == "string" then
		input = f_split(input, "\n")
		
		local out = ""
		for i, v in ipairs(input) do

			local s, e = v:find(string)
			if s ~= nil then
				out = out .. "\n" .. v
			end
		end
		return out
	else
		local out = {}
		for i, v in ipairs(input) do

			local s, e = v:find(string)
			if s ~= nil then
				table.insert(out, v)
			end
		end
	end
end


return {trim = f_trim, grep = f_grep, split = f_split}