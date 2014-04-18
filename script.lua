setTickRate(15)

function onTick() 
    yaw=getAccel(3)
    
    if yaw < 0 then yaw = -yaw end

    if yaw > 10 then setGpio(0,1) else setGpio(0,0) end
    if yaw > 20 then setGpio(1,1) else setGpio(1,0) end
    if yaw > 30 then setGpio(2,1) else setGpio(2,0) end
end