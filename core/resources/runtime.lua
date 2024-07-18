-- Runtime features that are easier to implement in Lua

-- Hints for IDE, will always actually be populated before this is loaded
playdate = playdate or {}

-- Local copies for efficiency
local geometry = playdate.geometry
local graphics = playdate.graphics

-- Initialize defaults for optional libraries
spritelib = spritelib or {}
graphics.nineSlice = graphics.nineSlice or {}

-- Create global table for emulator data
cranked = {
    printNewlines = true,
    inputHandlers = { { handler = playdate, masks = false } },
    menuCallbacks = {},
    preserved = {}, -- Mapping of value to reference count for preserved Lua values
    updateThread = nil,
    resources = {}, -- Weak mapping between light userdata and lua values
}
setmetatable(cranked.resources, { __mode = "v" })

playdate.argv = {}

function cranked.dispatchInputEvent(name, change, acceleratedChange)
    local handlers
    handlers = cranked.inputHandlers
    local count = #handlers
    local handled = false
    for i = 1, count do
        local handlerTable = handlers[count - i + 1]
        local handler = handlerTable.handler
        local masks = handlerTable.masks
        if handler[name] then
            handler[name](change, acceleratedChange) -- Extra args are nil anyway when unused
            if masks then
                return true
            end
        end
    end
    return handled
end

-- Overwrite default print functionality with ability to control newlines
function print(...)
    local write = io.write
    local n = select("#", ...)
    for i = 1, n do
        local v = tostring(select(i, ...))
        write(v)
        if i ~= n then
            write('\t')
        end
    end
    if cranked.printNewlines then
        write('\n')
    end
end

function table.indexOfElement(table, element)
    for k, v in ipairs(table) do
        if v == element then
            return k
        end
    end
    return nil
end

-- Not sure of an easy way to get size of the hash portion, so iterating it is...
function table.getSize(table)
    local count = 0
    for _ in pairs(table) do
        count = count + 1
    end
    local arraySize = #table
    return arraySize, count - arraySize
end

function table.shallowcopy(source, destination)
    if destination == nil then
        destination = {}
    end
    for k, v in pairs(source) do
        destination[k] = v
    end
    return destination
end

function table.deepcopy(source)
    local destination, tables, tab -- Separate lines to fool dumb, broken IDE plugin
    tab = table
    destination = {}
    tables = {} -- Cache copies to avoid infinite recursion
    for k, v in pairs(source) do
        if type(v) == 'table' then
            if tables[v] then
                destination[k] = tables[v]
            else
                local t = tab.deepcopy(v)
                tables[v] = t
                destination[k] = t
            end
        else
            destination[k] = v
        end
    end
    return destination
end

function playdate.setNewlinePrinted(flag)
    cranked.printNewlines = flag
end

-- Accelerometer control not needed
function playdate.startAccelerometer() end
function playdate.stopAccelerometer() end

function playdate.accelerometerIsRunning()
    return true -- Hopefully nothing relies on this actually changing
end

-- Ignore GC control
function playdate.setCollectsGarbage() end
function playdate.setMinimumGCTime() end
function playdate.setGCScaling() end

-- Simulator-only
function playdate.clearConsole() end
function playdate.setDebugDrawColor() end

function playdate.inputHandlers.push(handler, masks)
    if masks == nil then
        masks = false
    end
    local handlers
    handlers = cranked.inputHandlers
    handlers[#handlers + 1] = { handler = handler, masks = masks }
end

function playdate.inputHandlers.pop()
    local handlers
    handlers = cranked.inputHandlers
    local count = #handlers
    if count > 1 then
        handlers[count] = nil
    end
end

function playdate.geometry.rect.new(x, y, width, height)
    local rect = {x = x + .0, y = y + .0, width = width + .0, height = height + .0}
    setmetatable(rect, geometry.rect)
    return rect
end

function playdate.geometry.rect:copy()
    return geometry.rect.new(self.x, self.y, self.width, self.height)
end

function playdate.geometry.rect:toPolygon()
    local rect = geometry.polygon.new(
            self.x, self.y,
            self.x + self.width, self.y,
            self.x + self.width, self.y + self.height,
            self.x, self.y + self.height
    )
    rect:close()
    return rect
end

function playdate.geometry.rect:unpack()
    return self.x, self.y, self.width, self.height
end

function playdate.geometry.rect:isEmpty()
    return self.width == 0 or self.height == 0
end

function playdate.geometry.rect:isEqual(r2)
    return self.x == r2.x and self.y == r2.y and self.width == r2.width and self.height == r2.height
end

function playdate.geometry.rect:intersects(r2)
    return not ((r2.x + r2.width < self.x and r2.y + r2.height < self.y) or (r2.x > self.x + self.width and r2.y > self.y + self.height))
end

function playdate.geometry.rect:intersection(r2)
    local rect = geometry.rect.new(0, 0, 0, 0)
    if (r2.x > self.x) then
        rect.x = r2.x
        rect.width = math.max(0, math.min(r2.width, self.width - (r2.x - self.x)))
    else
        rect.x = self.x
        rect.width = math.max(0, math.min(self.width, r2.width - (self.x - r2.x)))
    end
    if (r2.y > self.y) then
        rect.y = r2.y
        rect.height = math.max(0, math.min(r2.height, self.height - (r2.y - self.y)))
    else
        rect.y = self.y
        rect.height = math.max(0, math.min(self.height, r2.height - (self.y - r2.y)))
    end
    return rect
end

function playdate.geometry.rect.fast_intersection(x1, y1, w1, h1, x2, y2, w2, h2)
    local x, y, width, height
    if (x2 > x1) then
        x = x2
        height = math.max(0, math.min(w2, w1 - (x2 - x1)))
    else
        x = x1
        width = math.max(0, math.min(w1, w2 - (x1 - x2)))
    end
    if (y2 > y1) then
        y = y2
        height = math.max(0, math.min(h2, h1 - (y2 - y1)))
    else
        y = y1
        height = math.max(0, math.min(h1, h2 - (y1 - y2)))
    end
    return x, y, width, height
end

function playdate.geometry.rect:union(r2)
    return geometry.rect.new(math.min(self.x, r2.x), math.min(self.y, r2.y),
            math.max(self.x + self.width, r2.x + r2.width), math.max(self.y + self.height, r2.y + r2.height))
end

function playdate.geometry.rect.fast_union(x1, y1, w1, h1, x2, y2, w2, h2)
    return math.min(x1, x2), math.min(y1, y2), math.max(x1 + w1, x2 + w2), math.max(y1 + h1, y2 + h2)
end

function playdate.geometry.rect:inset(dx, dy)
    self.x = self.x + dx
    self.y = self.y + dy
    self.width = self.width - dx * 2
    self.height = self.height - dy * 2
end

function playdate.geometry.rect:insetBy(dx, dy)
    return geometry.rect.new(self.x + dx, self.y + dy, self.width - dx * 2, self.height - dy * 2)
end

function playdate.geometry.rect:offset(dx, dy)
    self.x = self.x + dx
    self.y = self.y + dy
end

function playdate.geometry.rect:offsetBy(dx, dy)
    return geometry.rect.new(self.x + dx, self.y + dy)
end

function playdate.geometry.rect:containsRect(r2)
    return r2.x >= self.x and r2.y >= self.y and r2.x + r2.width <= self.x + self.width and r2.y + r2.height <= self.y + self.height
end

function playdate.geometry.rect:containsPoint(x, y)
    if type(x) == 'number' then
        return x >= self.x and y >= self.y and x <= self.x + self.width and y <= self.y + self.height
    else
        local p = x
        return p.x >= self.x and p.y >= self.y and p.x <= self.x + self.width and p.y <= self.y + self.height
    end
end

function playdate.geometry.rect:centerPoint()
    return geometry.point.new(self.x + self.width / 2, self.y + self.height / 2)
end

function playdate.geometry.rect:flipRelativeToRect(r2, flip)
    if flip == geometry.kFlippedX or flip == geometry.kFlippedXY then
        self.x = r2.x * 2 + r2.width - self.x - self.width
    end
    if flip == geometry.kFlippedY or flip == geometry.kFlippedXY then
        self.y = r2.y * 2 + r2.height - self.y - self.height
    end
end

function playdate.geometry.rect:__eq(other)
    return self.x == other.x and self.y == other.y and self.width == other.width and self.height == other.height
end

function playdate.geometry.rect:__tostring()
    return string.format('(%s, %s, %s, %s)', self.x, self.y, self.width, self.height)
end

function playdate.geometry.rect:__index(key)
    if key == 'top' then
        return self.y
    elseif key == 'bottom' then
        return self.y + self.height
    elseif key == 'right' then
        return self.x + self.width
    elseif key == 'left' then
        return self.x
    elseif key == 'origin' then
        return geometry.point.new(self.x, self.y)
    elseif key == 'size' then
        return geometry.size.new(self.width, self.height)
    else
        return geometry.rect[key]
    end
end

function playdate.geometry.point.new(x, y)
    local rect = {x = x + .0, y = y + .0}
    setmetatable(rect, geometry.point)
    return rect
end

function playdate.geometry.point:copy()
    return geometry.point.new(self.x, self.y)
end

function playdate.geometry.point:unpack()
    return self.x, self.y
end

function playdate.geometry.point:offset(dx, dy)
    self.x = self.x + dx
    self.y = self.y + dy
end

function playdate.geometry.point:offsetBy(dx, dy)
    return geometry.point.new(self.x + dx, self.y + dy)
end

function playdate.geometry.point:squaredDistanceToPoint(p)
    return (self.x - p.x) ^ 2 + (self.y - p.y) ^ 2
end

function playdate.geometry.point:distanceToPoint(p)
    return math.sqrt((self.x - p.x) ^ 2 + (self.y - p.y) ^ 2)
end

function playdate.geometry.point:__tostring()
    return string.format('(%s, %s)', self.x, self.y)
end

function playdate.geometry.point:__eq(p)
    return self.x == p.x and self.y == p.y
end

function playdate.geometry.point:__add(v)
    return geometry.point.new(self.x + v.dx, self.y + v.dy)
end

function playdate.geometry.point:__sub(p)
    return geometry.vector.new(self.x - p.x, self.y - p.y)
end

function playdate.geometry.point:__mul(t)
    return t:transformedPoint(self)
end

function playdate.geometry.point:__concat(p)
    return geometry.lineSegment.new(self.x, self.y, p.x, p.y)
end

function playdate.geometry.size.new(width, height)
    local rect = {width = width + .0, height = height + .0}
    setmetatable(rect, geometry.size)
    return rect
end

function playdate.geometry.size:copy()
    return geometry.size.new(self.width, self.height)
end

function playdate.geometry.size:__tostring()
    return string.format('(%s, %s)', self.width, self.height)
end

function playdate.geometry.size:__eq(other)
    return self.width == other.width and self.height == other.height
end

function playdate.geometry.size:unpack()
    return self.width, self.height
end

function playdate.geometry.vector2D.new(x, y)
    local vector = {dx = x + .0, dy = y + .0}
    setmetatable(vector, geometry.vector2D)
    return vector
end

function playdate.geometry.vector2D.newPolar(length, angle)
    local vector = {dx = math.cos(angle - 90) * length, dy = math.sin(angle - 90) * length}
    setmetatable(vector, geometry.vector2D)
    return vector
end

function playdate.geometry.vector2D:copy()
    return geometry.vector2D.new(self.dx, self.dy)
end

function playdate.geometry.vector2D:unpack()
    return self.dx, self.dy
end

function playdate.geometry.vector2D:addVector(v)
    self.dx = self.dx + v.dx
    self.dy = self.dy + v.dy
end

function playdate.geometry.vector2D:scale(s)
    self.dx = self.dx * s
    self.dy = self.dy * s
end

function playdate.geometry.vector2D:scaledBy(s)
    return geometry.vector2D.new(self.dx * s, self.dy * s)
end

function playdate.geometry.vector2D:normalize()
    local length = math.sqrt(self.dx ^ 2 + self.dy ^ 2)
    self.dx = self.dx / length
    self.dy = self.dy / length
end

function playdate.geometry.vector2D:dotProduct(v)
    return self.dx * v.dx + self.dy * v.dy
end

function playdate.geometry.vector2D:magnitude()
    return math.sqrt(self.dx ^ 2 + self.dy ^ 2)
end

function playdate.geometry.vector2D:magnitudeSquared()
    return self.dx ^ 2 + self.dy ^ 2
end

function playdate.geometry.vector2D:projectAlong(v)
    local vLen = v:magnitude()
    local length = (self.dx * v.dx + self.dy * v.dy) / vLen
    self.dx = v.dx / vLen * length
    self.dy = v.dy / vLen * length
end

function playdate.geometry.vector2D:projectedAlong(v)
    local copy = self:copy()
    copy:projectAlong(v)
    return copy
end

function playdate.geometry.vector2D:angleBetween(v)
    return math.atan(v.dy, v.dx) - math.atan(self.dy, self.dx)
end

function playdate.geometry.vector2D:__unm()
    return geometry.vector2D.new(-self.dx, -self.dy)
end

function playdate.geometry.vector2D:__add(v2)
    return geometry.vector2D.new(self.dx + v2.dx, self.dy + v2.dy)
end

function playdate.geometry.vector2D:__sub(v2)
    return geometry.vector2D.new(self.dx - v2.dx, self.dy - v2.dy)
end

function playdate.geometry.vector2D:__mul(other)
    if type(other) == 'number' then
        return geometry.vector2D.new(self.dx * other, self.dy * other)
    elseif other.__name == 'playdate.geometry.vector2D' then
        return self.dx * other.dx + self.dy * other.dy
    else -- Assume transform
        local x, y = other:transformXY(self.dx, self.dy)
        return geometry.vector2D.new(x, y)
    end
end

function playdate.geometry.vector2D:__div(s)
    return geometry.vector2D.new(self.dx / s, self.dy / s)
end

function playdate.geometry.vector2D:leftNormal()
    return geometry.vector2D.new(self.dy, -self.dx)
end

function playdate.geometry.vector2D:rightNormal()
    return geometry.vector2D.new(-self.dy, self.dx)
end

function playdate.geometry.vector2D:__tostring()
    return string.format('(%s, %s)', self.x, self.y)
end

function playdate.geometry.vector2D:__eq(other)
    return self.dx == other.dx and self.dy == other.dy
end

-- It's not documented, but x and y resolve to dx and dy, respectively
function playdate.geometry.vector2D:__index(key)
    if key == 'x' then
        return self.dx
    elseif key == 'y' then
        return self.dy
    else
        return geometry.vector2D[key]
    end
end

function playdate.geometry.vector2D:__newindex(key, value)
    if key == 'x' then
        self.dx = value + .0
    elseif key == 'y' then
        self.dy = value + .0
    else
        self[key] = value
    end
end

function playdate.geometry.lineSegment.new(x1, y1, x2, y2)
    local vector = {x1 = x1 + .0, y1 = y1 + .0, x2 = x2 + .0, y2 = y2 + .0}
    setmetatable(vector, geometry.lineSegment)
    return vector
end

function playdate.geometry.lineSegment:copy()
    return geometry.lineSegment.new(self.x1, self.y1, self.x2, self.y2)
end

function playdate.geometry.lineSegment:unpack()
    return self.x1, self.y1, self.x2, self.y2
end

function playdate.geometry.lineSegment:length()
    return math.sqrt((self.x1 - self.x2) ^ 2 + (self.y1 - self.y2) ^ 2)
end

function playdate.geometry.lineSegment:offset(dx, dy)
    self.x1 = self.x1 + dx
    self.y1 = self.y1 + dy
    self.x2 = self.x2 + dx
    self.y2 = self.y2 + dy
end

function playdate.geometry.lineSegment:offsetBy(dx, dy)
    return geometry.lineSegment.new(self.x1 + dx, self.y1 + dy, self.x2 + dx, self.y2 + dy)
end

function playdate.geometry.lineSegment:midPoint()
    return geometry.point.new((self.x1 + self.x2) / 2, (self.y1 + self.y2) / 2)
end

function playdate.geometry.lineSegment:pointOnLine(distance, extend)
    local length = self:length()
    if not extend then
        if distance < 0 then
            distance = 0
        else
            distance = math.max(-length, math.min(length, distance))
        end
    end
    local vecX = (self.x2 - self.x1) / length
    local vecY = (self.y2 - self.y1) / length
    return geometry.point.new(self.x1 + vecX * distance, self.y1 + vecY * distance)
end

function playdate.geometry.lineSegment:segmentVector()
    return geometry.vector2D.new(self.x2 - self.x1, self.y2 - self.y1)
end

-- Based on: https://math.stackexchange.com/a/406895
function playdate.geometry.lineSegment:closestPointOnLineToPoint(p)
    local length = self:length()
    local vecX = (self.x2 - self.x1) / length
    local vecY = (self.y2 - self.y1) / length
    local normX = -vecY
    local normY = vecX
    local offset = ((p.x - self.x1) * -normY + normX * (p.y - self.y1)) / (vecX * -normY + normX * vecY)
    return geometry.point.new(self.x1 + offset * vecX, self.y1 + offset * vecY)
end

-- Based on: https://stackoverflow.com/a/1968345
function playdate.geometry.lineSegment:intersectsLineSegment(ls)
    local sx1 = self.x2 - self.x1
    local sy1 = self.y2 - self.y1
    local sx2 = ls.x2 - ls.x1
    local sy2 = ls.y2 - ls.y1
    local d = -sx2 * sy1 + sx1 * sy2
    local s = (-sy1 * (self.x1 - ls.x1) + sx1 * (self.y1 - ls.y1)) / d
    local t = (sx2 * (self.y1 - ls.y1) + sy2 * (self.x1 - ls.x1)) / d
    if s >= 0 and s <= 1 and t >= 0 and t <= 1 then
        return true, geometry.point.new(self.x1 + t * self.x2, self.y1 + t * self.y2)
    else
        return false
    end
end

function playdate.geometry.lineSegment.fast_intersection(x1, y1, x2, y2, x3, y3, x4, y4)
    local sx1 = x2 - x1
    local sy1 = y2 - y1
    local sx2 = x4 - x3
    local sy2 = y4 - y3
    local d = -sx2 * sy1 + sx1 * sy2
    local s = (-sy1 * (x1 - x3) + sx1 * (y1 - y3)) / d
    local t = (sx2 * (y1 - y3) + sy2 * (x1 - x3)) / d
    if s >= 0 and s <= 1 and t >= 0 and t <= 1 then
        return true, x1 + t * x2, y1 + t * y2
    else
        return false
    end
end

function playdate.geometry.lineSegment:intersectsPolygon(poly)
    local points = {}
    local count = 1
    for i = 1, poly.numberOfVertices - 1 do
        local intersects, x, y = geometry.lineSegment.fast_intersection(self.x1, self.y1, self.x2, self.y2, poly[i].x, poly[i].y, poly[i + 1].x, poly[i + 1].y)
        if intersects then
            points[count] = geometry.point.new(x, y)
            count = count + 1
        end
    end
    if count > 1 then
        return true, points
    else
        return false
    end
end

function playdate.geometry.lineSegment:intersectsRect(rect)
    local intersects, x, y
    local points = {}
    local i = 1
    intersects, x, y = geometry.fast_intersection(self.x1, self.y1, self.x2, self.y2, rect.x, rect.y, rect.x + rect.width, rect.y)
    if intersects then
        points[i] = geometry.points.new(x, y)
        i = i + 1
    end
    intersects, x, y = geometry.fast_intersection(self.x1, self.y1, self.x2, self.y2, rect.x, rect.y, rect.x, rect.y + rect.height)
    if intersects then
        points[i] = geometry.points.new(x, y)
        i = i + 1
    end
    intersects, x, y = geometry.fast_intersection(self.x1, self.y1, self.x2, self.y2, rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height)
    if intersects then
        points[i] = geometry.points.new(x, y)
        i = i + 1
    end
    intersects, x, y = geometry.fast_intersection(self.x1, self.y1, self.x2, self.y2, rect.x, rect.y + rect.height, rect.x + rect.width, rect.y + rect.height)
    if intersects then
        points[i] = geometry.points.new(x, y)
        i = i + 1
    end
    if i > 1 then
        return true, points
    else
        return false
    end
end

function playdate.geometry.lineSegment:__tostring()
    return string.format('((%s, %s), (%s, %s))', self.x1, self.y1, self.x2, self.y2)
end

function playdate.geometry.lineSegment:__eq(other)
    return self.x1 == other.x1 and self.y1 == other.y1 and self.x2 == other.x2 and self.y2 == other.y2
end

function playdate.geometry.affineTransform.new(m11, m12, m21, m22, tx, ty)
    local vector
    if m11 == nil then
        vector = {m11 = 1.0, m12 = .0, m21 = .0, m22 = 1.0, tx = .0, ty = .0}
    else
        vector = {m11 = m11 + .0, m12 = m12 + .0, m21 = m21 + .0, m22 = m22 + .0, tx = tx + .0, ty = ty + .0}
    end
    setmetatable(vector, geometry.affineTransform)
    return vector
end

function playdate.geometry.affineTransform:copy()
    return playdate.geometry.affineTransform.new(self.m11, self.m12, self.m21, self.m22, self.tx, self.ty)
end

function playdate.geometry.affineTransform:invert()
    local det = self.m11 * self.m22 - self.m12 * self.m21
    local t_m11 = self.m22
    local t_m12 = self.m12
    local t_tx = self.m12 * self.ty - self.m22 * self.tx
    local t_m21 = self.m21
    local t_m22 = self.m11
    local t_ty = self.m11 * self.ty - self.m21 * self.tx
    self.m11 = t_m11 / det
    self.m12 = -t_m12 / det
    self.m21 = -t_m21 / det
    self.m22 = t_m22 / det
    self.tx = t_tx / det
    self.ty = -t_ty / det
end

function playdate.geometry.affineTransform:reset()
    self.m11 = 1.0
    self.m12 = 0.0
    self.m21 = 0.0
    self.m22 = 1.0
    self.tx = 0.0
    self.ty = 0.0
end

function playdate.geometry.affineTransform:concat(af)
    local t_m11 = af.m11 * self.m11 + af.m12 * self.m21
    local t_m12 = af.m11 * self.m12 + af.m12 * self.m22
    local t_tx = af.m11 * self.tx + af.m12 * self.ty + af.tx
    local t_m21 = af.m21 * self.m11 + af.m22 * self.m21
    local t_m22 = af.m21 * self.m12 + af.m22 * self.m22
    local t_ty = af.m21 * self.tx + af.m22 * self.ty + af.ty
    self.m11 = t_m11
    self.m12 = t_m12
    self.m21 = t_m21
    self.m22 = t_m22
    self.tx = t_tx
    self.ty = t_ty
end

function playdate.geometry.affineTransform:translate(dx, dy)
    self.tx = self.tx + dx
    self.ty = self.ty + dy
end

function playdate.geometry.affineTransform:translatedBy(dx, dy)
    return geometry.affineTransform.new(self.m11, self.m12, self.m21, self.m22, self.tx + dx, self.ty + dy)
end

function playdate.geometry.affineTransform:scale(sx, sy)
    if sy == nil then
        sy = sx
    end
    self.m11 = sx * self.m11
    self.m12 = sx * self.m12
    self.m21 = sy * self.m21
    self.m22 = sy * self.m22
    self.tx = sx * self.tx
    self.ty = sy * self.ty
end

function playdate.geometry.affineTransform:scaledBy(sx, sy)
    if sy == nil then
        sy = sx
    end
    return geometry.affineTransform.new(sx * self.m11, sx * self.m12, sy * self.m21, sy * self.m22, sx * self.tx, sy * self.ty)
end

function playdate.geometry.affineTransform:rotate(angle, x, y)
    if x == nil then
        x = 0
        y = 0
    elseif y == nil then
        y = x.y
        x = x.x
    end
    self.tx = self.tx - x
    self.ty = self.ty - y
    angle = angle / 180 * math.pi
    local cos = math.cos(angle)
    local sin = math.sin(angle)
    local t_m11 = cos * self.m11 + -sin * self.m21
    local t_m12 = cos * self.m12 + -sin * self.m22
    local t_tx = cos * self.tx + -sin * self.ty
    local t_m21 = sin * self.m11 + cos * self.m21
    local t_m22 = sin * self.m12 + cos * self.m22
    local t_ty = sin * self.tx + cos * self.ty
    self.m11 = t_m11
    self.m12 = t_m12
    self.m21 = t_m21
    self.m22 = t_m22
    self.tx = t_tx + x
    self.ty = t_ty + y
end

-- Official implementation doesn't respect geometry.point option here, for some reason
function playdate.geometry.affineTransform:rotatedBy(angle, x, y)
    local clone = self:copy()
    clone:rotate(angle, x, y)
    return clone
end

function playdate.geometry.affineTransform:skew(sx, sy)
    local tan_x = math.tan(sx / 180 * math.pi)
    local tan_y = math.tan(sy / 180 * math.pi)
    local t_m11 = self.m11 + tan_x * self.m21
    local t_m12 = self.m12 + tan_x * self.m22
    local t_tx = self.tx + tan_x * self.ty
    local t_m21 = tan_y * self.m11 + self.m21
    local t_m22 = tan_y * self.m12 + self.m22
    local t_ty = tan_y * self.tx + self.ty
    self.m11 = t_m11
    self.m12 = t_m12
    self.m21 = t_m21
    self.m22 = t_m22
    self.tx = t_tx
    self.ty = t_ty
end

function playdate.geometry.affineTransform:skewedBy(sx, sy)
    local clone = self:copy()
    clone:skew(sx, sy)
    return clone
end

function playdate.geometry.affineTransform:transformPoint(p)
    local x = p.x * self.m11 + p.y * self.m12 + self.tx
    local y = p.x * self.m21 + p.y * self.m22 + self.ty
    p.x = x
    p.y = y
end

function playdate.geometry.affineTransform:transformedPoint(p)
    return geometry.point.new(p.x * self.m11 + p.y * self.m12 + self.tx, p.x * self.m21 + p.y * self.m22 + self.ty)
end

function playdate.geometry.affineTransform:transformXY(x, y)
    return x * self.m11 + y * self.m12 + self.tx, x * self.m21 + y * self.m22 + self.ty
end

function playdate.geometry.affineTransform:transformLineSegment(ls)
    ls.x1, ls.y1 = self:transformXY(ls.x1, ls.y1)
    ls.x2, ls.y2 = self:transformXY(ls.x2, ls.y2)
end

function playdate.geometry.affineTransform:transformedLineSegment(ls)
    local x1, y1 = self:transformXY(ls.x1, ls.y1)
    local x2, y2 = self:transformXY(ls.x2, ls.y2)
    return geometry.lineSegment.new(x1, y1, x2, y2)
end

-- Doesn't affect width and height in official implementation (Should it?)
function playdate.geometry.affineTransform:transformAABB(r)
    r.x, r.y = self:transformXY(r.x, r.y)
end

function playdate.geometry.affineTransform:transformedAABB(r)
    local x, y = self:transformXY(r.x, r.y)
    return geometry.rect.new(x, y, r.width, r.height)
end

function playdate.geometry.affineTransform:transformedPolygon(p)
    local transformed = geometry.polygon.new(0)
    transformed.numberOfVertices = p.numberOfVertices
    for i = 1, p.numberOfVertices do
        transformed[i] = self:transformedPoint(p[i])
    end
    return transformed
end

function playdate.geometry.affineTransform:__tostring()
    return string.format('[ %s    %s    %s ]\n', self.m11, self.m12, self.tx)
            .. string.format('[ %s    %s    %s ]\n', self.m21, self.m22, self.ty)
            .. string.format('[ %s    %s    %s ]\n', 0.0, 0.0, 1.0)
end

function playdate.geometry.affineTransform:__eq(other)
    return self.m11 == other.m11 and self.m12 == other.m12 and self.m21 == other.m21 and self.m22 == other.m22 and self.tx == other.tx and self.ty == other.ty
end

function playdate.geometry.affineTransform:__mul(other)
    if other.__name == 'playdate.geometry.affineTransform' then
        local clone = self:copy()
        clone:concat(other)
        return clone
    elseif other.__name == 'playdate.geometry.vector2D' then
        return geometry.vector2D.new(other.dx * self.m11 + other.dy * self.m12 + self.tx, other.dx * self.m21 + other.dy * self.m22 + self.ty)
    elseif other.__name == 'playdate.geometry.point' then
        return geometry.point.new(other.x * self.m11 + other.y * self.m12 + self.tx, other.x * self.m21 + other.y * self.m22 + self.ty)
    else
       error('Invalid affineTransform __mul arg')
    end
end

function playdate.geometry.polygon.new(...)
    local count = select('#', ...)
    local polygon = {}
    setmetatable(polygon, geometry.polygon)
    if count == 1 then
        polygon.numberOfVertices = select(1, ...)
        for i = 1, polygon.numberOfVertices do
            polygon[i] = geometry.point.new(0, 0)
        end
    elseif type(select(1, ...)) == 'number' then
        polygon.numberOfVertices = count / 2
        for i = 1, count / 2 do
            polygon[i] = geometry.point.new(select(i * 2 - 1, ...), select(i * 2, ...))
        end
    elseif select(1, ...).__name == 'playdate.geometry.point' then
        polygon.numberOfVertices = count
        for i = 1, count do
            polygon[i] = select(i, ...)
        end
    else
        error('polygon.new args')
    end
    return polygon
end

function playdate.geometry.polygon:copy()
    local polygon = {}
    setmetatable(polygon, geometry.polygon)
    for i = 1, self.numberOfVertices do
        polygon[i] = self[i]
    end
    return polygon
end

function playdate.geometry.polygon:close()
    if self[self.numberOfVertices] ~= self[1] then
        self[self.numberOfVertices + 1] = self[1]
    end
end

function playdate.geometry.polygon:isClosed()
    return self[self.numberOfVertices] == self[1]
end

function playdate.geometry.polygon:containsPoint(x, y, fillRule)
    if type(x) ~= 'number' then -- A bit ugly, but nicer than the varargs syntax?
        fillRule = y
        y = x.y
        x = x.x
    end
    if fillRule == nil or fillRule == graphics.kPolygonFillEvenOdd then -- Even-odd
        -- Based on: https://en.wikipedia.org/wiki/Even%E2%80%93odd_rule
        local prev = self[self.numberOfVertices]
        local c = false
        for i = 1, self.numberOfVertices do
            current = self[i]
            if x == current.x and y == current.y then -- Is corner
                return true
            end
            if (current.y > y) ~= (prev.y > y) then
                local slope = (x - current.x) * (prev.y - current.y) - (prev.x - current.x) * (y - current.y)
                if slope == 0 then -- On boundary
                    return true
                end
                if (slope < 0) ~= (prev.y < current.y) then
                    c = not c
                end
            end
            prev = current
        end
        return c
    else -- Non-zero (Winding number algorithm)
        -- Based on: https://github.com/anirudhtopiwala/OpenSource_Problems/blob/master/Point_In_Polygon/src/point_in_polygon.cpp
        local count = 0
        for i = 1, self.numberOfVertices - 1 do -- Todo: Is the last vertex an issue when closed/unclosed?
            local p1 = self[i]
            local p2 = self[i + 1]
            local pointInLine = ((y - p1.y) * (p2.x - p1.x)) - ((x - p1.x) * (p2.y - p1.y))
            if pointInLine == 0 then -- Lies on polygon
                return true
            end
            if p1.y <= y then
                if p2.y > y then -- Upward crossing
                    if pointInLine > 0 then -- Point is left of edge
                        count = count + 1
                    end
                end
            else
                if p2.y < y then -- Downward crossing
                    if pointInLine < 0 then -- Point is right of edge
                        count = count - 1
                    end
                end
            end
        end
        return count ~= 0
    end
end

function playdate.geometry.polygon:getBounds()
    local minX = 0
    local minY = 0
    local maxX = 0
    local maxY = 0
    for i = 0, self.numberOfVertices do
        local p = self[i]
        minX = math.min(minX, p.x)
        minY = math.min(minY, p.y)
        maxX = math.max(maxX, p.x)
        maxY = math.max(maxY, p.y)
    end
    return minX, minY, maxX, maxY
end

function playdate.geometry.polygon:getBoundsRect()
    local minX, minY, maxX, maxY = self:getBounds()
    return geometry.rect.new(minX, minY, maxX - minX, maxY - minY)
end

function playdate.geometry.polygon:count()
    if self[self.numberOfVertices] ~= self[1] then
        return self.numberOfVertices
    else
        return self.numberOfVertices - 1
    end
end

function playdate.geometry.polygon:length()
    local length = 0
    for i = 1, self.numberOfVertices - 1 do
        length = length + self[i]:distanceToPoint(self[i + 1])
    end
    return length
end

function playdate.geometry.polygon:setPointAt(n, x, y)
    self[n].x = x
    self[n].y = y
end

function playdate.geometry.polygon:getPointAt(n)
    return self[n]
end

function playdate.geometry.polygon:intersects(p)
    for i = 1, self.numberOfVertices - 1 do
        for j = 1, p.numberOfVertices - 1 do
            local s1 = self[i]
            local e1 = self[i + 1]
            local s2 = p[j]
            local e2 = p[j + 1]
            if geometry.lineSegment.fast_intersection(s1.x, s1.y, e1.x, e1.y, s2.x, s2.y, e2.x, e2.y) then
                return true
            end
        end
    end
    return false
end

function playdate.geometry.polygon:pointOnPolygon(distance, extend)
    if self.numberOfVertices < 2 then
        return nil -- Todo: What should be returned for empty polygon?
    end
    local traveled = 0
    for i = 1, self.numberOfVertices - 1 do
        local len = self[i]:distanceToPoint(self[i + 1])
        if traveled + len >= distance then
            return geometry.lineSegment.new(self[i].x, self[i].y, self[i + 1].x, self[i + 1].y):pointOnLine(distance - traveled)
        else
            traveled = traveled + len
        end
    end
    if extend then
        local segment = geometry.lineSegment.new(self[self.numberOfVertices - 1].x, self[self.numberOfVertices - 1].y, self[self.numberOfVertices].x, self[self.numberOfVertices].y)
        return segment:pointOnLine(distance - traveled)
    else
        return self[self.numberOfVertices]:copy()
    end
end

function playdate.geometry.polygon:translate(dx, dy)
    for i = 1, self.numberOfVertices do
        local point = self[i]
        point.x = point.x + dx
        point.y = point.y + dy
    end
end

function playdate.geometry.polygon:__mul(t)
    for i = 1, self.numberOfVertices do
        t:transformPoint(self[i])
    end
end

function playdate.geometry.polygon:__eq(other)
    if self.numberOfVertices ~= other.numberOfVertices then
        return false
    end
    for i = 1, self.numberOfVertices do
        if self[i] ~= other[i] then
            return false
        end
    end
    return true
end

function playdate.geometry.polygon:__tostring()
    local string = '['
    for i = 1, self.numberOfVertices do
        if i > 1 then
            string = string .. ', '
        end
        string = string .. tostring(self[i])
    end
    return string .. ']'
end

function playdate.geometry.arc.new(x, y, radius, startAngle, endAngle, direction)
    if direction == nil then
        direction = endAngle > startAngle
    end
    local arc = {x = x + .0, y = y + .0, radius = radius + .0, startAngle = startAngle + .0, endAngle = endAngle + .0, direction = direction}
    setmetatable(arc, geometry.arc)
    return arc
end

function playdate.geometry.arc:copy()
    return playdate.geometry.arc.new(self.x, self.y, self.radius, self.startAngle, self.endAngle, self.direction)
end

function playdate.geometry.arc:length()
    return 2 * math.pi * self.radius * math.abs(self.startAngle - self.endAngle) / 360
end

function playdate.geometry.arc:isClockwise()
    return self.direction;
end

function playdate.geometry.arc:setIsClockwise(flag)
    self.direction = flag
end

function playdate.geometry.arc:pointOnArc(distance, extend)
    local angle = distance / (2 * math.pi * self.radius) * 360
    if not self.direction then
        angle = -angle
    end

    if not extend then
        local length = math.abs(self.endAngle - self.startAngle)
        if distance < 0 then
            angle = 0
        else
            angle = math.max(math.min(angle, length), -length)
        end
    end

    angle = (self.startAngle + angle -90) / 180 * math.pi
    return geometry.point.new(math.cos(angle) * self.radius + self.x, math.sin(angle) * self.radius + self.y)
end

function playdate.geometry.arc:__tostring()
    return string.format('location: (%s, %s), radius: %s, startAngle: %s, endAngle: %s, clockwise? %s', self.x, self.y, self.radius, self.startAngle, self.endAngle, self.direction)
end

function playdate.geometry.arc:eq(other)
    return self.x == other.x and self.y == other.y and self.radius == other.radius
            and self.startAngle == other.startAngle and self.endAngle == other.endAngle and self.direction == other.direction
end

function playdate.geometry.squaredDistanceToPoint(x1, y1, x2, y2)
    return (x1 - x2) ^ 2 + (y1 - y2) ^ 2
end

function playdate.geometry.distanceToPoint(x1, y1, x2, y2)
    return math.sqrt((x1 - x2) ^ 2 + (y1 - y2) ^ 2)
end

-- Unpack object args for official library functions that expect userdata rather than table objects
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
        drawCircleInRect(x, select(1, ...))
    end
end

function playdate.graphics.fillCircleInRect(x, ...)
    if type(x) == 'table' then
        fillCircleInRect(x.x, x.y, x.width, x.height)
    else
        fillCircleInRect(x, select(1, ...))
    end
end

function playdate.graphics.drawCircleAtPoint(x, ...)
    if type(x) == 'table' then
        drawCircleAtPoint(x.x, x.y, select(1, ...))
    else
        drawCircleAtPoint(x, select(1, ...))
    end
end

function playdate.graphics.fillCircleAtPoint(x, ...)
    if type(x) == 'table' then
        fillCircleAtPoint(x.x, x.y, select(1, ...))
    else
        fillCircleAtPoint(x, select(1, ...))
    end
end

function playdate.graphics.drawArc(x, ...)
    if type(x) == 'table' then
        drawArc(x.x, x.y, x.radius, x.startAngle, x.endAngle, select(1, ...))
    else
        drawArc(x, select(1, ...))
    end
end

function playdate.graphics.drawTextInRect(str, x, ...)
    if type(x) == 'table' then
        drawTextInRect(str, x.x, x.y, x.width, x.height, select(1, ...))
    else
        drawTextInRect(str, x, select(1, ...))
    end
end

function playdate.graphics.drawLocalizedTextInRect(text, x, ...)
    if type(x) == 'table' then
        drawLocalizedTextInRect(text, x.x, x.y, x.width, x.height, select(1, ...))
    else
        drawLocalizedTextInRect(text, x, select(1, ...))
    end
end

function spritelib.addEmptyCollisionSprite(x, ...)
    if type(x) == 'table' then
        addEmptyCollisionSprite(x.x, x.y, x.width, x.height, select(1, ...))
    else
        addEmptyCollisionSprite(x, select(1, ...))
    end
end

function playdate.graphics.nineSlice:drawInRect(x, ...)
    if type(x) == 'table' then
        drawInRect(self, x.x, x.y, x.width, x.height)
    else
        drawInRect(self, x, select(1, ...))
    end
end
