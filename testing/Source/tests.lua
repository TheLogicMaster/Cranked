import "test"

local pd = playdate
local gfx = pd.graphics
local display = pd.display

tests = {}

function tests.Display(results)
    results:add('width1', display.getWidth())
    results:add('height1', display.getHeight())
    results:add('displayRect1', display.getRect())
    results:add('scale1', display.getScale())
    results:add('inverted1', display.getInverted())
    results:add('mosaic1', display.getMosaic())
    results:add('offset1', display.getOffset())

    display.setScale(2)
    display.setInverted(true)
    display.setMosaic(1, 1)
    display.setOffset(20, 20)

    results:add('width2', display.getWidth())
    results:add('height2', display.getHeight())
    results:add('displayRect2', display.getRect())
    results:add('scale2', display.getScale())
    results:add('inverted2', display.getInverted())
    results:add('mosaic2', display.getMosaic())
    results:add('offset2', display.getOffset())

    display.setScale(1)
    display.setInverted(false)
    display.setMosaic(0, 0)
    display.setOffset(0, 0)
end
