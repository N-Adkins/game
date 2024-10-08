local script = {}

local Engine = require("Engine")

--local sprite = Engine.CreateSprite()
local sprite = nil

local direction = 1
local value = 0

function script:OnFrame()
    value = value + 0.07
    if sprite.position.x > 0.8 then
        direction = -1
    elseif sprite.position.x < -0.8 then
        direction = 1
    end
    sprite.position = sprite.position + Vec2.new(direction * 0.01, direction * 0.01)
    sprite.scale = (math.cos(value) + 1) / 2
end

return script
