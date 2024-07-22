-- Patch official library functions that expect userdata rather than table objects

local drawCircleInRect = playdate.graphics.drawCircleInRect
local fillCircleInRect = playdate.graphics.fillCircleInRect
local drawCircleAtPoint = playdate.graphics.drawCircleAtPoint
local fillCircleAtPoint = playdate.graphics.fillCircleAtPoint
local drawArc = playdate.graphics.drawArc
local drawTextInRect = playdate.graphics.drawTextInRect
local drawLocalizedTextInRect = playdate.graphics.drawLocalizedTextInRect
local addEmptyCollisionSprite = spritelib.addEmptyCollisionSprite
local drawInRect = playdate.graphics.nineSlice.drawInRect

function playdate.graphics.drawCircleInRect(x, ...)
    if type(x) == 'table' then
        drawCircleInRect(x.x, x.y, x.width, x.height)
    else
        drawCircleInRect(x, ...)
    end
end

function playdate.graphics.fillCircleInRect(x, ...)
    if type(x) == 'table' then
        fillCircleInRect(x.x, x.y, x.width, x.height)
    else
        fillCircleInRect(x, ...)
    end
end

function playdate.graphics.drawCircleAtPoint(x, ...)
    if type(x) == 'table' then
        drawCircleAtPoint(x.x, x.y, ...)
    else
        drawCircleAtPoint(x, ...)
    end
end

function playdate.graphics.fillCircleAtPoint(x, ...)
    if type(x) == 'table' then
        fillCircleAtPoint(x.x, x.y, ...)
    else
        fillCircleAtPoint(x, ...)
    end
end

function playdate.graphics.drawArc(x, ...)
    if type(x) == 'table' then
        drawArc(x.x, x.y, x.radius, x.startAngle, x.endAngle, ...)
    else
        drawArc(x, ...)
    end
end

function playdate.graphics.drawTextInRect(str, x, ...)
    if type(x) == 'table' then
        drawTextInRect(str, x.x, x.y, x.width, x.height, ...)
    else
        drawTextInRect(str, x, ...)
    end
end

function playdate.graphics.drawLocalizedTextInRect(text, x, ...)
    if type(x) == 'table' then
        drawLocalizedTextInRect(text, x.x, x.y, x.width, x.height, ...)
    else
        drawLocalizedTextInRect(text, x, ...)
    end
end

function spritelib.addEmptyCollisionSprite(x, ...)
    if type(x) == 'table' then
        addEmptyCollisionSprite(x.x, x.y, x.width, x.height, ...)
    else
        addEmptyCollisionSprite(x, ...)
    end
end

function playdate.graphics.nineSlice:drawInRect(x, ...)
    if type(x) == 'table' then
        drawInRect(self, x.x, x.y, x.width, x.height)
    else
        drawInRect(self, x, ...)
    end
end
