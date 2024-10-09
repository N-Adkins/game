local script = {}

local Engine = require("Engine")

local sprite = Engine.CreateSprite()
local other = Engine.CreateSprite()
local next = Engine.CreateSprite()
print(sprite)
print(other)
print(next)

local velocity = Vec2.new(0, 0)
local just_moved = false

local connection = Engine.Events.OnKeyPressed:Connect(function (key)
    velocity = Vec2.new(0, 0)
    if key == Engine.KeyCode.Up then
        velocity = velocity + Vec2.new(0, 0.01)
    elseif key == Engine.KeyCode.Down then
        velocity = velocity + Vec2.new(0, -0.01)
    elseif key == Engine.KeyCode.Left then
        velocity = velocity + Vec2.new(-0.01, 0)
    elseif key == Engine.KeyCode.Right then
        velocity = velocity + Vec2.new(0.01, 0)
    end
end)

function script:OnFrame()
    sprite.position = sprite.position + velocity
    velocity = Vec2.new(0, 0)
end

return script
