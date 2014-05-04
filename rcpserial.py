import serial
import io
import json
from rcpconfig import *

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
        obd2Cfg = self.getObd2Cfg()
        scriptCfg = self.getScript()
        
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
            
        if obd2Cfg:
            rcpCfg['obd2Cfg'] = obd2Cfg['obd2Cfg']
        
        if scriptCfg:
            rcpCfg['script'] = scriptCfg['script']
            
        return rcpCfg
    
    def writeRcpCfg(self, cfg):
        rcpCfg = cfg.get('rcpCfg', None)
        if rcpCfg:
            gpsCfg = rcpCfg.get('gpsCfg', None)
            if gpsCfg:
                self.sendCommand({'setGpsCfg': gpsCfg})
                
            imuCfg = rcpCfg.get('imuCfg', None)
            if imuCfg:
                self.sendCommand({'setImuCfg': imuCfg})
                
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
    
    def getObd2Cfg(self):
        return self.sendGet('getObd2Cfg', None)
    
    def getScript(self):
        return self.sendGet('getScript', None)

    def getVersion(self):
        rsp = self.sendCommand("getVer", None)
            
    def decodeScript(self, s):
        return s.replace('\\n','\n').replace('\_',' ').replace('\\r','\r').replace('\\"','"')

    def encodeScript(self, s):
        return s.replace('\n','\\n').replace(' ', '\_').replace('\r', '\\r').replace('"', '\\"')

    def writeScriptPage(self, ser, script, page):
        cmd = 'writeScriptPage ' + str(page) + ' ' + self.encodeScript(script) + '\r'
        ser.write(cmd)
        line = self.readLine(ser)
        line = self.readLine(ser)
        if 'result="ok"' in line:
            return 1
        else:
            return 0

    def readScriptPage(self, ser, page):
        cmd = 'readScriptPage ' + str(page) + '\r'
        print("page: " + cmd)
        ser.write(cmd)
        line = self.readLine(ser)
        line = self.readLine(ser)
        print(line)
        line = line[9:]
        line = line[:-4]
        self.decodeScript(line)
        return line

    def readScript(self):
        i = 0
        lua = ''
        ser = self.getSerial()
        while True:
            script = self.readScriptPage(ser, i)
            if script:
                lua = lua + script
                i+=1
            else:
                break
        return lua

    def writeScript(self, script):
        i = 0
        res = 0
        ser = self.getSerial()
        while True:
            if len(script) >= 256:
                scr = script[:256]
                script = script[256:]
                res = self.writeScriptPage(ser, scr, i)
                if res == 0:
                    print 'Error: ' + str(i)
                    break
                i = i + 1
            else:
                self.writeScriptPage(ser, script, i)
                self.writeScriptPage(ser, '', i + 1)
                res = 1
                break
        return res


