local script = {}

local Engine = require("Engine")
local Test = require("other")

Engine.SetVSync(false)

local sprite = Engine.CreateSprite()

local velocity = Vec2.new(0, 0)
local acceleration = Vec2.new(0, 0)
local maxSpeed = 100
local accelRate = 30
local friction = 10

function script:OnFrame(delta_time)
    acceleration = Vec2.new(0, 0)

    if Engine.IsKeyPressed(Engine.KeyCode.Up) then
        acceleration = acceleration + Vec2.new(0, 1)
    end
    if Engine.IsKeyPressed(Engine.KeyCode.Down) then
        acceleration = acceleration + Vec2.new(0, -1)
    end
    if Engine.IsKeyPressed(Engine.KeyCode.Left) then
        acceleration = acceleration + Vec2.new(-1, 0)
    end
    if Engine.IsKeyPressed(Engine.KeyCode.Right) then
        acceleration = acceleration + Vec2.new(1, 0)
    end

    if acceleration:Length() > 0 then
        acceleration = acceleration / acceleration:Length() * accelRate * 1000
    end

    local friction_force = velocity * -1 * friction
    acceleration = acceleration + friction_force

    velocity = velocity + acceleration * delta_time

    if velocity:Length() > maxSpeed then
        velocity = velocity:Normalize() * maxSpeed
    end

    sprite.position = sprite.position + velocity * delta_time
end

return script
