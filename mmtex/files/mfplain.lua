local mplib = require('mplib')
local mpkpse = kpse.new("luatex", "mpost")

local function finder(name, mode, ftype)
	if mode == "w" then
		return name
	else
		return mpkpse:find_file(name,ftype)
	end
end

local function handle_result(result)
	if result.log then
		print(result.log)
	end
	if result.status ~= 0 then
		os.exit(1)
	end
end

for _, font in ipairs(arg) do
	local mpx = mplib.new({find_file=finder, interaction="batch", job_name=font})
	handle_result(mpx:execute(string.format('\\input mfplain \\mode=localfont; input %s', font)))
	handle_result(mpx:finish())
end
