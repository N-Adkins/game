local script = {}

local Engine = require("Engine")

Engine.SetVSync(false)

local sprite = Engine.CreateSprite()

-- Variables for velocity and acceleration
local velocity = Vec2.new(0, 0)
local acceleration = Vec2.new(0, 0)
local maxSpeed = 10
local accelRate = 1 -- Rate at which acceleration increases
local friction = 0.999   -- Friction for slowing down the sprite

function script:OnFrame(delta_time)
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

    -- Update sprite position based on velocity
    sprite.position = sprite.position + velocity * delta_time * 100

    --sprite.scale = 1 - (math.abs(velocity.x) + math.abs(velocity.y)) / 2;

    -- Apply friction to slow down the sprite
    velocity = velocity * friction
end

return script
