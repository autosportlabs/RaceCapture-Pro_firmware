function onTick()
   if getAnalog(3) >= 205 then setGpio(0,1) else setGpio(0,0) end
   if getAnalog(0) <= 10 then setGpio(1,1) else setGpio(1,0) end
   if getAnalog(4) < 0.05 then setGpio(2,1) else setGpio(2,0) end
end

