import serial
import io
import json
from rcpconfig import *
from serial.tools import list_ports

CHANNEL_ADD_MODE_IN_PROGRESS = 1
CHANNEL_ADD_MODE_COMPLETE = 2

class RcpSerial:
    def __init__(self, **kwargs):
        self.ser = None
        self.port = kwargs.get('port', None)

    def setPort(self, port):
        self.port = port

    def sendCommand(self, cmd):
        rsp = None
        ser = self.getSerial()
        ser.flushInput()
        ser.flushOutput()
        cmdStr = json.dumps(cmd, separators=(',', ':')) + '\r'
        print('send cmd: ' + cmdStr)
        ser.write(cmdStr)
        
        rsp = self.readLine(ser)
        if cmdStr.startswith(rsp):
            rsp = self.readLine(ser)

        if rsp:
            print('rsp: ' + rsp)
            rsp = json.loads(rsp)
        return rsp      
        
    def sendGet(self, name, index):
        if index == None:
            index = None
        else:
            index = str(index)
        cmd = {name:index}
        return self.sendCommand(cmd)

    def getSerial(self):
        if not self.ser:
            ser = self.open()
            ser.flushInput()
            ser.flushOutput()
            self.ser = ser
        return self.ser
            
    def open(self):
        print('Opening serial')
        ser = serial.Serial(self.port, timeout = 1)
        ser.flushInput()
        ser.flushOutput()
        return ser

    def close(self, ser):
        if ser != None:
            ser.close()
        ser = None
    
    def readLine(self, ser):
        eol2 = b'\r'
        retryCount = 0
        line = bytearray()

        while True:
            c = ser.read(1)
            if  c == eol2:
                break
            elif c == '':
                if retryCount > 10:
                    raise Exception('Could not read message')
                retryCount +=1
                print('Timeout - retry: ' + str(retryCount))
                ser.write(' ')
            else:
                line += c
                
        line = bytes(line).strip()
        line = line.replace('\r', '')
        line = line.replace('\n', '')
        return line

    def getRcpCfg(self):
        analogCfg = self.getAnalogCfg(None)
        imuCfg = self.getImuCfg(None)
        gpsCfg = self.getGpsCfg()
        timerCfg = self.getTimerCfg(None)
        gpioCfg = self.getGpioCfg(None)
        pwmCfg = self.getPwmCfg(None)
        trackCfg = self.getTrackCfg()
        canCfg = self.getCanCfg()
        obd2Cfg = self.getObd2Cfg()
        scriptCfg = self.getScript()
        connCfg = self.getConnectivityCfg()
        
        rcpCfg = {}
        
        if analogCfg:
            rcpCfg['analogCfg'] = analogCfg['analogCfg']
    
        if rcpCfg:
            rcpCfg['imuCfg'] = imuCfg['imuCfg']
        
        if gpsCfg:
            rcpCfg['gpsCfg'] = gpsCfg['gpsCfg']
            
        if timerCfg:
            rcpCfg['timerCfg'] = timerCfg['timerCfg']
           
        if gpioCfg:
            rcpCfg['gpioCfg'] = gpioCfg['gpioCfg']
            
        if pwmCfg:
            rcpCfg['pwmCfg'] = pwmCfg['pwmCfg']

        if trackCfg:
            rcpCfg['trackCfg'] = trackCfg['trackCfg']
            
        if connCfg:
            rcpCfg['connCfg'] = connCfg['connCfg']

        if canCfg:
            rcpCfg['canCfg'] = canCfg['canCfg']
            
        if obd2Cfg:
            rcpCfg['obd2Cfg'] = obd2Cfg['obd2Cfg']
        
        if scriptCfg:
            rcpCfg['scriptCfg'] = scriptCfg['scriptCfg']
            
        return rcpCfg
    
    def writeRcpCfg(self, cfg):
        rcpCfg = cfg.get('rcpCfg', None)
        if rcpCfg:
            gpsCfg = rcpCfg.get('gpsCfg', None)
            if gpsCfg:
                self.sendCommand({'setGpsCfg': gpsCfg})
                
            imuCfg = rcpCfg.get('imuCfg', None)
            if imuCfg:
                for i in range(IMU_CHANNEL_COUNT):
                    imuChannel = imuCfg.get(str(i))
                    if imuChannel:
                        self.sendCommand({'setImuCfg': {str(i): imuChannel}})
            
            analogCfg = rcpCfg.get('analogCfg', None)
            if analogCfg:
                for i in range(ANALOG_CHANNEL_COUNT):
                    analogChannel = analogCfg.get(str(i))
                    if analogChannel:
                        self.sendCommand({'setAnalogCfg': {str(i): analogChannel}})
        
            timerCfg = rcpCfg.get('timerCfg', None)
            if timerCfg:
                for i in range(TIMER_CHANNEL_COUNT):
                    timerChannel = timerCfg.get(str(i))
                    if timerChannel:
                        self.sendCommand({'setTimerCfg': {str(i): timerChannel}})
                        
            gpioCfg = rcpCfg.get('gpioCfg', None)
            if gpioCfg:
                for i in range(GPIO_CHANNEL_COUNT):
                    gpioChannel = gpioCfg.get(str(i))
                    if gpioChannel:
                        self.sendCommand({'setGpioCfg': {str(i): gpioChannel}})

            pwmCfg = rcpCfg.get('pwmCfg', None)
            if pwmCfg:
                for i in range(PWM_CHANNEL_COUNT):
                    pwmChannel = pwmCfg.get(str(i))
                    if pwmChannel:
                        self.sendCommand({'setPwmCfg': {str(i): pwmChannel}})

            connCfg = rcpCfg.get('connCfg', None)
            if connCfg:
                self.sendCommand({'setConnCfg' : connCfg})
                
            canCfg = rcpCfg.get('canCfg', None)
            if canCfg:
                self.sendCommand({'setCanCfg': canCfg})
                
            obd2Cfg = rcpCfg.get('obd2Cfg', None)
            if obd2Cfg:
                self.sendCommand({'setObd2Cfg': obd2Cfg})
                
            trackCfg = rcpCfg.get('trackCfg', None)
            if trackCfg:
                self.sendCommand({'setTrackCfg': trackCfg})
                
            scriptCfg = rcpCfg.get('scriptCfg', None)
            if scriptCfg:
                self.writeScript(scriptCfg)
                
            self.flashConfig()
        
                
    def getAnalogCfg(self, channelId):
        return self.sendGet('getAnalogCfg', channelId)    

    def getImuCfg(self, channelId):
        return self.sendGet('getImuCfg', channelId)
    
    def getGpsCfg(self):
        return self.sendGet('getGpsCfg', None)
    
    def setGpsCfg(self, gpsCfg):
        return self.sendSet('setGpsCfg', gpsCfg)
    
    def getTimerCfg(self, channelId):
        return self.sendGet('getTimerCfg', channelId)
    
    def getGpioCfg(self, channelId):
        return self.sendGet('getGpioCfg', channelId)
    
    def getPwmCfg(self, channelId):
        return self.sendGet('getPwmCfg', channelId)
    
    def getTrackCfg(self):
        return self.sendGet('getTrackCfg', None)
    
    def getCanCfg(self):
        return self.sendGet('getCanCfg', None)
    
    def getObd2Cfg(self):
        return self.sendGet('getObd2Cfg', None)
    
    def getConnectivityCfg(self):
        return self.sendGet('getConnCfg', None)
    
    def getScript(self):
        return self.sendGet('getScriptCfg', None)

    def writeScript(self, scriptCfg):
        i = 0
        res = 0
        script = scriptCfg['data']
        while True:
            if len(script) >= 256:
                scr = script[:256]
                script = script[256:]
                res = self.sendCommand({'setScriptCfg': {'data':scr,'page':i}})
                if res == 0:
                    print 'Error: ' + str(i)
                    break
                i = i + 1
            else:
                self.sendCommand({'setScriptCfg': {'data':script,'page':i}})
                self.sendCommand({'setScriptCfg': {'data':'','page':i + 1}})
                res = 1
                break
        return res
        
    def flashConfig(self):
        return self.sendCommand({'flashCfg':None})

    def getChannels(self):
        return self.sendGet("getChannels", None)
                
    def addChannel(self, channelJson, index, mode):
        return self.sendCommand({'addChannel': 
                                 {'index': index, 
                                 'mode': mode,
                                 'channel': channelJson
                                 }
                                 })
                                  
    def getVersion(self):
        rsp = self.sendCommand({"getVer":None})
        return rsp

    def autoDetect(self):
        ports = [x[0] for x in list_ports.comports()]

        print "Searching for RaceCapture on all serial ports"
        ver = None
        for p in ports:
            try:
                print "Trying", p
                self.port = p
                ver = self.getVersion()
                break
            except:
                self.port = None
                self.close(self.ser)

        if not ver == None:
            print "Found racecapture on port:", self.port


    

