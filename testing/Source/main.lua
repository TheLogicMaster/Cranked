import 'tests'

local pd = playdate
local gfx = pd.graphics

pd.file.delete('results', true)

function pd.update()
    for name,testFunc in pairs(tests) do
        gfx.clear()

        local results = Results(name)
        print('Running test: ' .. name)
        local startTime = pd.getElapsedTime()

        testFunc(results)
        local duration = pd.getElapsedTime() - startTime
        print('Finished in ' .. tostring(duration) .. ' seconds')

        json.encodeToFile('results/' .. name .. '/' .. 'results.json', true, {
            name = name,
            duration = duration,
            results = results.results
        })

        coroutine.yield()
    end
    pd.stop()
end
