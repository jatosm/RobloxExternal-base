# Lua API

```lua
print(...)                        -- output line
warn(...)                         -- warning line
identifyexecutor()                -- "thenwefuckin", "1.0"
wait(s?)                          -- yield seconds (0.03)
spawn(fn)                         -- new thread, returns it
iskeypressed(k)                   -- key name or vk code -> bool
getmousepos()                     -- screen x, y
httpget(url)                      -- body string or error
players()                         -- {name,x,y,w,h,hp,dist} array
getgenv()                         -- shared globals table
loadstring(src [, chunk])         -- = load
checkcaller()                     -- always true
newcclosure(fn)                   -- returns fn
clonefunction(fn)                 -- distinct copy
tick()                            -- seconds since boot

task.wait(s) / task.spawn(fn)
task.delay(s, fn) / task.defer(fn)  -- token
task.cancel(token)

bit32.band/bor/bxor/bnot/lshift/rshift

cheat.version()                   -- "1.0"
cheat.fps() / cheat.players() / cheat.time()
cheat.on_render(fn | nil)         -- per-frame callback

draw.box(x,y,w,h,r,g,b [,a])
draw.fill(x,y,w,h,r,g,b [,a])
draw.text(x,y,s [,r,g,b])
draw.line(x0,y0,x1,y1,r,g,b [,a])

game.root() / game.workspace() / game.find(name)
inst:Name() / ClassName() / Parent() / Children()
inst:FindFirstChild(name) / IsA(class)

getgc(true)                       -- live game tables
findgc(key [, val])               -- filtered search
setgc("stop"|"restart"|"count"|"isrunning"|"stepmul" [,v])
getgc_info()                      -- heap stats or nil
getrawkeys(proxy)                 -- key/value table
gcverify()                        -- offset self-test -> bool
rawget(t, k)                      -- proxy-transparent
```