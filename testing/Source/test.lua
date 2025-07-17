import "CoreLibs/object"

local pd = playdate

class('Results').extends()

function Results:init(name)
    self.name = name
    self.results = {}
    self.resultsDir = 'results/' .. self.name
    pd.file.mkdir(self.resultsDir)
end

function Results:run()
    error('Test:run not overridden')
end

function Results:add(name, ...)
    args = table.pack(...)
    count = args.n
    for i,v in ipairs(args) do
        local t = type(v)
        local n = name .. '_' .. tostring(i)
        if count == 1 then
            n = name
        end
        if t == 'number' then
            self:addNum(n, v)
        elseif t == 'nil' then
            self:addNil(n, v)
        elseif t == 'string' then
            self:addText(n, v)
        elseif t == 'boolean' then
            self:addBool(n, v)
        elseif t == 'table' or t == 'userdata' then
            local mt = getmetatable(v)
            if mt == pd.geometry.rect then
                self:addRect(n, v)
            elseif mt == pd.geometry.point then
                self:addPoint(n, v)
            elseif mt == pd.geometry.size then
                self:addSize(n, v)
            elseif mt == pd.geometry.vector2D then
                self:addVector(n, v)
            elseif mt == pd.geometry.lineSegment then
                self:addLineSegment(n, v)
            elseif mt == pd.geometry.affineTransform then
                self:addTransform(n, v)
            elseif mt == pd.geometry.polygon then
                self:addPolygon(n, v)
            elseif mt == pd.geometry.arc then
                self:addArc(n, v)
            elseif t == 'table' then
                self:addTable(n, v)
            else
                error('Unsupported test value type: ' .. t)
            end
        else
            error('Unsupported test value type: ' .. t)
        end
    end
end

function Results:addNil(name, options)
    self.results[name] = { type = 'nil', value = nil, options = options }
end

function Results:addNum(name, value, options)
    self.results[name] = { type = 'number', value = value, options = options }
end

function Results:addInt(name, value, options)
    self.results[name] = { type = 'int', value = value, options = options }
end

function Results:addBool(name, value, options)
    self.results[name] = { type = 'bool', value = value, options = options }
end

function Results:addText(name, value, options)
    self.results[name] = { type = 'text', value = value, options = options }
end

function Results:addTable(name, value, options)
    self.results[name] = { type = 'table', value = value, options = options }
end

function Results:addImage(name, filename, options)
    self.results[name] = { type = 'image', value = filename .. '.pdi', options = options }
end

function Results:saveImage(name, image, options)
    pd.datastore.writeImage(image, self.resultsDir .. name)
    self.addImage(name, name, options)
end

function Results:saveDisplay(name, options)
    pd.datastore.writeImage(pd.graphics.getDisplayImage(), self.resultsDir .. name)
    self.addImage(name, name, options)
end

function Results:addRect(name, value, options)
    self:addNum(name .. '_x', value.x, options)
    self:addNum(name .. '_y', value.y, options)
    self:addNum(name .. '_width', value.width, options)
    self:addNum(name .. '_height', value.height, options)
end

function Results:addPoint(name, value, options)
    self:addNum(name .. '_x', value.x, options)
    self:addNum(name .. '_y', value.y, options)
end

function Results:addSize(name, value, options)
    self:addNum(name .. '_width', value.width, options)
    self:addNum(name .. '_height', value.height, options)
end

function Results:addVector(name, value, options)
    self:addNum(name .. '_dx', value.dx, options)
    self:addNum(name .. '_dy', value.dy, options)
end

function Results:addLineSegment(name, value, options)
    self:addNum(name .. '_x1', value.x1, options)
    self:addNum(name .. '_y1', value.y1, options)
    self:addNum(name .. '_x2', value.x2, options)
    self:addNum(name .. '_y2', value.y2, options)
end

function Results:addTransform(name, value, options)
    self:addNum(name .. '_m11', value.m11, options)
    self:addNum(name .. '_m12', value.m12, options)
    self:addNum(name .. '_m21', value.m21, options)
    self:addNum(name .. '_m22', value.m22, options)
    self:addNum(name .. '_tx', value.tx, options)
    self:addNum(name .. '_ty', value.ty, options)
end

function Results:addPolygon(name, value, options)
    local count = value:count()
    for i = 1, count do
        self:addPoint(name .. tostring(i), value[i], options)
    end
    self:addBool(name .. '_closed', value:isClosed())
end

function Results:addArc(name, value, options)
    self:addNum(name .. '_x', value.x, options)
    self:addNum(name .. '_y', value.y, options)
    self:addNum(name .. '_radius', value.radius, options)
    self:addNum(name .. '_startAngle', value.startAngle, options)
    self:addNum(name .. '_endAngle', value.endAngle, options)
    self:addBool(name .. '_clockwise', value.isClockwise(), options)
end
