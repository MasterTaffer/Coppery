

local f_parse = function (filename, separator)
	if separator == nil then
		separator = ":"
	end

	local f = File.Open(filename);
	if f == nil then
		return nil
	end

	local rows = {}
	while not f:isEOF() do
		local line = f:readLine()
		if #line > 0 and line:sub(1,1) ~= "#" then
			
			local row = {}
			local s = 1

			while true do
				local ns, e = line:find(separator, s)
				local col
				if ns == nil then
					col = line:sub(s)
					col = Util.trim(col)

					if #col == 0 and s == 1 then
						break
					end

					table.insert(row, col)
					break
				else
					col = line:sub(s, ns - 1)
					col = Util.trim(col)
					table.insert(row, col)
					s = e + 1
				end
			end
			if #row > 0 then
				table.insert(rows, row)
			end
		end
	end

	return rows
end




return {parse = f_parse}