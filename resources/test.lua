local script = {}

local Engine = require("Engine")

local sprite = Engine.CreateSprite()

-- Variables for velocity and acceleration
local velocity = Vec2.new(0, 0)
local acceleration = Vec2.new(0, 0)
local maxSpeed = 10
local accelRate = 0.5 -- Rate at which acceleration increases
local friction = 0.95   -- Friction for slowing down the sprite

function script:OnFrame()
    -- Reset acceleration each frame
    acceleration = Vec2.new(0, 0)

    -- Apply acceleration based on input
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

    -- Normalize acceleration vector to prevent diagonal speed boost
    if acceleration:Length() > 0 then
        acceleration = acceleration / acceleration:Length() * accelRate
    end

    -- Update velocity based on acceleration
    velocity = velocity + acceleration

    -- Clamp velocity to maxSpeed
    if velocity:Length() > maxSpeed then
        velocity = velocity:Normalize() * maxSpeed
    end

    -- Apply friction to slow down the sprite
    velocity = velocity * friction

    -- Update sprite position based on velocity
    sprite.position = sprite.position + velocity
end

return script
