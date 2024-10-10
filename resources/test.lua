local script = {}
local Engine = require("Engine")
Engine.SetVSync(false)
local sprite = Engine.CreateSprite()

-- Variables for velocity and acceleration
local velocity = Vec2.new(0, 0)
local acceleration = Vec2.new(0, 0)
local maxSpeed = 100
local accelRate = 30 -- Increased to compensate for delta time
local friction = 10 -- Adjusted for more appropriate friction

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
        acceleration = acceleration / acceleration:Length() * accelRate * 1000
    end

    -- Apply friction
    local friction_force = velocity * -1 * friction
    acceleration = acceleration + friction_force

    -- Update velocity based on acceleration
    velocity = velocity + acceleration * delta_time

    -- Clamp velocity to maxSpeed
    if velocity:Length() > maxSpeed then
        velocity = velocity:Normalize() * maxSpeed
    end

    -- Update sprite position based on velocity
    sprite.position = sprite.position + velocity * delta_time

    -- Debug output
    --[[print("Delta time: " .. delta_time)
    print("Acceleration: " .. tostring(acceleration))
    print("Velocity: " .. velocity:Length() .. ", Position: " .. tostring(sprite.position))]]--
end

return script
