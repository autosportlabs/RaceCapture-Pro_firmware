import serial
import io
import json

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
        print('send cmd: ' + cmd)
        ser.write(cmd + '\r')

        echo = self.readLine(ser)
        if echo:
            rsp = self.readLine(ser)

        if rsp:
            print('rsp: ' + rsp)
            rsp = json.loads(rsp)
        return rsp      

    def sendGet(self, name, index):
        if index == None:
            index = 'null'
        else:
            index = str(index)
        cmd = '{"' + name + '":' + index + '}'
        return self.sendCommand(cmd)

    def getSerial(self):
        if not self.ser:
            self.ser = self.open()
        return self.ser
            
    def open(self):
        print('Opening serial')
        ser = serial.Serial(self.port, timeout = 3)
        ser.flushInput()
        ser.flushOutput()
        return ser

    def close(self, ser):
        if ser != None:
            ser.close()
        ser = None
    
    def readLine(self, ser):
        eol = b'\n'
        eol2 = b'\r'

        leneol = len(eol)
        line = bytearray()
        while True:
            c = ser.read(1)
            if c == eol or c == eol2:
                break
            else:
                line += c
        line = bytes(line)
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

    def getAnalogCfg(self, channelId):
        return self.sendGet('getAnalogCfg', channelId)

    def getImuCfg(self, channelId):
        return self.sendGet('getImuCfg', channelId)
    
    def getGpsCfg(self):
        return self.sendGet('getGpsCfg', None)

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
    
    def decodeScript(self, s):
        return s.replace('\\n','\n').replace('\_',' ').replace('\\r','\r').replace('\\"','"')

    def encodeScript(self, s):
        return s.replace('\n','\\n').replace(' ', '\_').replace('\r', '\\r').replace('"', '\\"')

    def writeScriptPage(self, ser, script, page):
        cmd = 'writeScriptPage ' + str(page) + ' ' + self.encodeScript(script) + '\r'
        print(cmd)
        ser.write(cmd)
        line = self.readLine(ser)
        line = self.readLine(ser)
        print(line)
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


