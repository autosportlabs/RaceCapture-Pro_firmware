--[[
Cheesy RCP script

Inputs as wired on 2014-03-20 -- Stieg

Analog Channel - Sender
0 - Oil Pressure
1 - Oil Temp
2 - Fuel PrewriteScriptPageressure has an issue
2 - Water Temp has an issue
--]]


-- Globals --
LIGHTS = 3
LIGHT_VALS = nil
INIT_LIGHTS = 0

function updateLights()
   local l = 0
   repeat
      -- Convert our booleans to 1 (true) or 0 (false) --
      local val = LIGHT_VALS[l] and 1 or 0
      setGpio(l, val)
      l = l + 1
   until l >= LIGHTS
end

function toggleLight(index)
   LIGHT_VALS[index] = not LIGHT_VALS[index]
end

function checkWaterTemp()
   local waterTemp = getAnalog(3)
   if waterTemp >= 205 then
      toggleLight(2)
   end
end

function checkOilPressure()
   local oilPress = getAnalog(0)
   if oilPress <= 10 then
      toggleLight(1)
   end
end

function checkFuelLevel()
   local fuelLevel = getAnalog(4)
   if fuelLevel < 0.05 then
      toggleLight(0)
   end
end

function initLights()
   if INIT_LIGHTS >= LIGHTS then
      return true
   end

   -- If here then we are initializing the lights --
   local prevVal = LIGHT_VALS[INIT_LIGHTS] -- nil or true
   toggleLight(INIT_LIGHTS)
   if prevVal then INIT_LIGHTS = INIT_LIGHTS + 1 end
   return false
end

function controlLights()
   local initialized = initLights()
   if initialized then
      checkWaterTemp()
      checkOilPress()
      checkFuelLevel()
   end
   updateLights()
end

function onTick()
    startLogging()
   controlLights()
end
