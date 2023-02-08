local cmdcore = require "cmdcore"
local lcmdcore = {}

function lcmdcore.start(name,script_path)
    return cmdcore.cmdcore_start(name, script_path)
end

function lcmdcore.restart(name,script_path)
    return cmdcore.cmdcore_restart(name, script_path)
end

function lcmdcore.stop(name)
    return cmdcore.cmdcore_stop(name)
end

function lcmdcore.list()
    return cmdcore.cmdcore_list()
end

return lcmdcore