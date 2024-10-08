local script = {}

local Engine = require("Engine")

local sprite = Engine.CreateSprite()
local direction = 1
local value = 0

function script:OnStart()
    print(sprite.position)
    sprite = nil
end

function script:OnFrame()
end

return script
