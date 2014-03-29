setTickRate(10)

function onTick()
  if getAccel(0) > .5 then
    
    startLogging()
  else
    stopLogging()
  end
end