local lcmdcore = require "scripts.core.lcmd"
print("begin load models")

local model_map = {
    {name = "rdscache_model", path = "scripts/models/rdscache_model.lua"},
    {name = "test_model", path = "scripts/models/test_model.lua"},
}

for i,v in pairs(model_map) do
    lcmdcore.start(v.name, v.path)
end

