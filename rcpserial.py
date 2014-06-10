import serial
import io
import json
import traceback
import Queue
from threading import Thread, Lock
from rcpconfig import *
from serial.tools import list_ports
from functools import partial

CHANNEL_ADD_MODE_IN_PROGRESS = 1
CHANNEL_ADD_MODE_COMPLETE = 2
DEFAULT_READ_RETRIES = 2
DEFAULT_LEVEL2_RETRIES = 4
DEFAULT_SERIAL_READ_TIMEOUT = None
DEFAULT_SERIAL_WRITE_TIMEOUT = 0
DEFAULT_MSG_RX_TIMEOUT = 1.0


class RcpCmd:
    name = None
    cmd = None
    payload = None
    index = None
    option = None
    def __init__(self, name, cmd, payload = None, index = None, option = None):
        self.name = name
        self.cmd = cmd
        self.payload = payload
        self.index = index
        self.option = option

class SingleRcpCmd(RcpCmd):
    winCallback = None
    failCallback = None
    def __init__(self, name, cmd, winCallback, failCallback, payload = None, index = None, option = None):
        super(SingleRcpCmd, self).__init__(name, cmd, payload, index, option)
        self.winCallback = winCallback
        self.failCallback = failCallback
            
class RcpSerial:    
    msgListeners = {}
    cmdQueue = Queue.Queue()
    cmdSequenceQueue = Queue.Queue()
    cmdSequenceLock = Lock()
    sendCommandLock = Lock()
    on_progress = lambda value: value
    on_tx = lambda self, value: None
    on_rx = lambda self, value: None
    
    retryCount = DEFAULT_READ_RETRIES
    timeout = DEFAULT_SERIAL_READ_TIMEOUT
    writeTimeout = DEFAULT_SERIAL_WRITE_TIMEOUT
    
    def __init__(self, **kwargs):
        self.ser = None
        self.port = kwargs.get('port', None)
     
    def setPort(self, port):
        self.port = port
        
    def initSerial(self):
        rxThread = Thread(target=self.msgRxWorker)
        rxThread.daemon = True
        rxThread.start()        

    def addListener(self, messageName, callback):
        listeners = self.msgListeners.get(messageName, None)
        if listeners:
            listeners.add(callback)
        else:
            listeners = set()
            listeners.add(callback)
            self.msgListeners[messageName] = listeners

    def removeListener(self, messageName, callback):
        listeners = self.msgListeners.get(messageName, None)
        if listeners:
            listeners.discard(callback)
            
    def msgRxWorker(self):
        print('msgRxWorker started')
        while True:
            serial = self.getSerial()
            try:
                msg = self.readLine(serial)
                print('msgRxWorker Rx: ' + str(msg))
                msgJson = json.loads(msg, strict = False)
                self.on_rx(True)
                for messageName in msgJson.keys():
                    print('processing message ' + messageName)
                    listeners = self.msgListeners.get(messageName, None)
                    if listeners:
                        for listener in listeners:
                            listener(msgJson)
                    break
            except Exception:
                print('Message Rx Exception: ' + str(Exception))
                traceback.print_exc()
        
    def rcpCmdComplete(self, msgReply):
        self.cmdSequenceQueue.put(msgReply)
                
    def recoverTimeout(self):
        print('POKE')
        self.getSerial().write('\r')
        
    def executeSingle(self, rcpCmd, winCallback, failCallback):
        t = Thread(target=self.executeSequence, args=([rcpCmd], None, winCallback, failCallback))
        t.daemon = True
        t.start()
        
    def notifyProgress(self, count, total):
        if self.on_progress:
            self.on_progress((float(count) / float(total)) * 100)
        
        
    def executeSequence(self, cmdSequence, rootName, winCallback, failCallback):
                    
        self.cmdSequenceLock.acquire()
        print('Execute Sequence begin')
                
        q = self.cmdSequenceQueue
        
        responseResults = {}
        cmdCount = 0
        cmdLength = len(cmdSequence)
        self.notifyProgress(cmdCount, cmdLength)
        try:
            for rcpCmd in cmdSequence:
                payload = rcpCmd.payload
                index = rcpCmd.index
                option = rcpCmd.option

                level2Retry = 0
                name = rcpCmd.name
                result = None
                
                self.addListener(name, self.rcpCmdComplete)
                while not result and level2Retry < DEFAULT_LEVEL2_RETRIES:
                    if not payload == None and not index == None and not option == None:
                        rcpCmd.cmd(payload, index, option)
                    elif not payload == None and not index == None:
                        rcpCmd.cmd(payload, index)
                    elif not payload == None:
                        rcpCmd.cmd(payload)
                    else:
                        rcpCmd.cmd()
                    

                    retry = 0
                    while not result and retry < self.retryCount:
                        try:
                            result = q.get(True, DEFAULT_MSG_RX_TIMEOUT)
                            msgName = result.keys()[0]
                            if not msgName == name:
                                print('rx message did not match expected name ' + str(name) + '; ' + str(msgName))
                                result = None
                        except Exception:
                            print('Read message timeout')
                            self.recoverTimeout()
                            retry += 1
                    if not result:
                        print('Level 2 retry for ' + name)
                        level2Retry += 1


                if not result:
                    raise Exception('Timeout waiting for ' + name)
                            
                                    
                responseResults[name] = result[name]
                self.removeListener(name, self.rcpCmdComplete)
                cmdCount += 1
                self.notifyProgress(cmdCount, cmdLength)
                
            if rootName:
                winCallback({rootName: responseResults})
            else:
                winCallback(responseResults)
        except Exception as detail:
            print('Command sequence exception: ' + str(detail))
            traceback.print_exc()
            failCallback(detail)
        finally:
            self.cmdSequenceLock.release()
        print('Execute Sequence complete')
                
    def sendCommand(self, cmd, sync = False):
        try:
            self.sendCommandLock.acquire()
            rsp = None
            ser = self.getSerial()
            ser.flushInput()
            ser.flushOutput()
            cmdStr = json.dumps(cmd, separators=(',', ':')) + '\r'
            print('send cmd: ' + cmdStr)
            ser.write(cmdStr)
            if sync:
                rsp = self.readLine(ser)
                if cmdStr.startswith(rsp):
                    rsp = self.readLine(ser)
                return json.loads(rsp)
        finally:
            self.sendCommandLock.release()
            self.on_tx(True)
        
    def sendGet(self, name, index = None):
        if index == None:
            index = None
        else:
            index = str(index)
        cmd = {name:index}
        self.sendCommand(cmd)

    def sendSet(self, name, payload, index = None):
        if not index == None:
            self.sendCommand({name: {str(index): payload}})            
        else:
            self.sendCommand({name: payload})
        
    def getSerial(self):
        if not self.ser:
            ser = self.open()
            ser.flushInput()
            ser.flushOutput()
            self.ser = ser
        return self.ser
            
    def open(self):
        print('Opening serial')
        if self.port == None:
            return None
        else:
            ser = serial.Serial(self.port, timeout=self.timeout, writeTimeout = self.writeTimeout) 
            ser.flushInput()
            ser.flushOutput()
            return ser

    def close(self):
        if self.ser != None:
            self.ser.close()
        self.ser = None
    
    def readLine(self, ser):
        eol2 = b'\r'
        retryCount = 0
        line = bytearray()

        while True:
            c = ser.read(1)
            if  c == eol2:
                break
            elif c == '':
                if retryCount >= self.retryCount:
                    self.close()
                    raise Exception('Could not read message')
                retryCount +=1
                print('Timeout - retry: ' + str(retryCount))
                print("POKE")
                ser.write(' ')
            else:
                line += c
                
        line = bytes(line).strip()
        line = line.replace('\r', '')
        line = line.replace('\n', '')
        return line
        
    def getRcpCfg(self, winCallback, failCallback):
        cmdSequence = [       RcpCmd('ver',         self.getVersion),
                              RcpCmd('analogCfg',   self.getAnalogCfg),
                              RcpCmd('imuCfg',      self.getImuCfg),
                              RcpCmd('gpsCfg',      self.getGpsCfg),
                              RcpCmd('timerCfg',    self.getTimerCfg),
                              RcpCmd('gpioCfg',     self.getGpioCfg),
                              RcpCmd('pwmCfg',      self.getPwmCfg),
                              RcpCmd('trackCfg',    self.getTrackCfg),
                              RcpCmd('canCfg',      self.getCanCfg),
                              RcpCmd('obd2Cfg',     self.getObd2Cfg),
                              RcpCmd('scriptCfg',   self.getScript),
                              RcpCmd('connCfg',     self.getConnectivityCfg)
                           ]
                
        t = Thread(target=self.executeSequence, args=(cmdSequence, 'rcpCfg', winCallback, failCallback))
        t.daemon = True
        t.start()
            
    def writeRcpCfg(self, cfg, winCallback = None, failCallback = None):
        cmdSequence = []
        rcpCfg = cfg.get('rcpCfg', None)
        if rcpCfg:
            gpsCfg = rcpCfg.get('gpsCfg', None)
            if gpsCfg:
                cmdSequence.append(RcpCmd('setGpsCfg', self.setGpsCfg, gpsCfg))
                imuCfg = rcpCfg.get('imuCfg', None)
            if imuCfg:
                for i in range(IMU_CHANNEL_COUNT):
                    imuChannel = imuCfg.get(str(i))
                    if imuChannel:
                        cmdSequence.append(RcpCmd('setImuCfg', self.setImuCfg, imuChannel, i))
                
            analogCfg = rcpCfg.get('analogCfg', None)
            if analogCfg:
                for i in range(ANALOG_CHANNEL_COUNT):
                    analogChannel = analogCfg.get(str(i))
                    if analogChannel:
                        cmdSequence.append(RcpCmd('setAnalogCfg', self.setAnalogCfg, analogChannel, i))
            
            timerCfg = rcpCfg.get('timerCfg', None)
            if timerCfg:
                for i in range(TIMER_CHANNEL_COUNT):
                    timerChannel = timerCfg.get(str(i))
                    if timerChannel:
                        cmdSequence.append(RcpCmd('setTimerCfg', self.setTimerCfg, timerChannel, i))
                            
            gpioCfg = rcpCfg.get('gpioCfg', None)
            if gpioCfg:
                for i in range(GPIO_CHANNEL_COUNT):
                    gpioChannel = gpioCfg.get(str(i))
                    if gpioChannel:
                        cmdSequence.append(RcpCmd('setGpioCfg', self.setGpioCfg, gpioChannel, i))
    
            pwmCfg = rcpCfg.get('pwmCfg', None)
            if pwmCfg:
                for i in range(PWM_CHANNEL_COUNT):
                    pwmChannel = pwmCfg.get(str(i))
                    if pwmChannel:
                        cmdSequence.append(RcpCmd('setPwmCfg', self.setPwmCfg, pwmChannel, i))
    
            connCfg = rcpCfg.get('connCfg', None)
            if connCfg:
                cmdSequence.append(RcpCmd('setConnCfg', self.setConnectivityCfg, connCfg))

            canCfg = rcpCfg.get('canCfg', None)
            if canCfg:
                cmdSequence.append(RcpCmd('setCanCfg', self.setCanCfg, canCfg))
            
            obd2Cfg = rcpCfg.get('obd2Cfg', None)
            if obd2Cfg:
                cmdSequence.append(RcpCmd('setObd2Cfg', self.setObd2Cfg, obd2Cfg))

            trackCfg = rcpCfg.get('trackCfg', None)
            if trackCfg:
                cmdSequence.append(RcpCmd('setTrackCfg', self.setTrackCfg, trackCfg))
                
            scriptCfg = rcpCfg.get('scriptCfg', None)
            if scriptCfg:
                self.sequenceWriteScript(scriptCfg, cmdSequence)

            cmdSequence.append(RcpCmd('flashCfg', self.sendFlashConfig))
                
        t = Thread(target=self.executeSequence, args=(cmdSequence, 'setRcpCfg', winCallback, failCallback,))
        t.daemon = True
        t.start()
                        
                
    def getAnalogCfg(self, channelId = None):
        self.sendGet('getAnalogCfg', channelId)    

    def setAnalogCfg(self, analogCfg, channelId):
        self.sendSet('setAnalogCfg', analogCfg, channelId)

    def getImuCfg(self, channelId = None):
        self.sendGet('getImuCfg', channelId)
    
    def setImuCfg(self, imuCfg, channelId):
        self.sendSet('setImuCfg', imuCfg, channelId)
    
    def getGpsCfg(self):
        self.sendGet('getGpsCfg', None)
    
    def setGpsCfg(self, gpsCfg):
        self.sendSet('setGpsCfg', gpsCfg)
        
    def getTimerCfg(self, channelId = None):
        self.sendGet('getTimerCfg', channelId)
    
    def setTimerCfg(self, timerCfg, channelId):
        self.sendSet('setTimerCfg', timerCfg, channelId)
    
    def setGpioCfg(self, gpioCfg, channelId):
        self.sendSet('setGpioCfg', gpioCfg, channelId)
        
    def getGpioCfg(self, channelId = None):
        self.sendGet('getGpioCfg', channelId)
    
    def getPwmCfg(self, channelId = None):
        self.sendGet('getPwmCfg', channelId)
    
    def setPwmCfg(self, pwmCfg, channelId):
        self.sendSet('setPwmCfg', pwmCfg, channelId)
        
    def getTrackCfg(self):
        self.sendGet('getTrackCfg', None)
    
    def setTrackCfg(self, trackCfg):
        self.sendSet('setTrackCfg', trackCfg)
    
    def getCanCfg(self):
        self.sendGet('getCanCfg', None)
    
    def setCanCfg(self, canCfg):
        self.sendSet('setCanCfg', canCfg)
    
    def getObd2Cfg(self):
        self.sendGet('getObd2Cfg', None)
    
    def setObd2Cfg(self, obd2Cfg):
        self.sendSet('setObd2Cfg', obd2Cfg)
    
    def getConnectivityCfg(self):
        self.sendGet('getConnCfg', None)
    
    def setConnectivityCfg(self, connCfg):
        self.sendSet('setConnCfg', connCfg)
    
    def getScript(self):
        self.sendGet('getScriptCfg', None)

    def setScriptPage(self, scriptPage, page):
        self.sendCommand({'setScriptCfg': {'data':scriptPage,'page':page}})
        
    def sequenceWriteScript(self, scriptCfg, cmdSequence):
        i = 0
        script = scriptCfg['data']
        while True:
            if len(script) >= 256:
                scr = script[:256]
                script = script[256:]
                cmdSequence.append(RcpCmd('setScriptCfg', self.setScriptPage, scr, i))
                i = i + 1
            else:
                cmdSequence.append(RcpCmd('setScriptCfg', self.setScriptPage, script, i))
                cmdSequence.append(RcpCmd('setScriptCfg', self.setScriptPage, '', i + 1))
                break
        
    def sendRunScript(self):
        self.sendCommand({'runScript': None})
        
    def runScript(self, winCallback, failCallback):
        self.executeSingle(RcpCmd('runScript', self.sendRunScript), winCallback, failCallback)
        
    def getLogfile(self, winCallback = None, failCallback = None):
        def getLogfileCmd():
            self.sendCommand({'getLogfile': None})
            
        if winCallback and failCallback:
            self.executeSingle(RcpCmd('logfile', getLogfileCmd), winCallback, failCallback)
        else:
            getLogfileCmd()
            
    def sendFlashConfig(self):
        self.sendCommand({'flashCfg': None})

    def getChannels(self):
        self.sendGet('getChannels')
        
    def getChannelList(self, winCallback, failCallback):
        cmdSequence = [ RcpCmd('channels', self.getChannels) ]
                
        t = Thread(target=self.executeSequence, args=(cmdSequence, None, winCallback, failCallback))
        t.daemon = True
        t.start()
                
    def setChannelList(self, channels, winCallback, failCallback):
        cmdSequence = []
        
        channels = channels.get('channels', None)
        if channels:
            index = 0
            channelCount = len(channels)
            for channel in channels:
                mode = CHANNEL_ADD_MODE_IN_PROGRESS if index < channelCount - 1 else CHANNEL_ADD_MODE_COMPLETE
                cmdSequence.append(RcpCmd('addChannel', self.addChannel, channel, index, mode))
                index += 1
            
            getRcpCfgThread = Thread(target=self.executeSequence, args=(cmdSequence, None, winCallback, failCallback))
            getRcpCfgThread.daemon = True
            getRcpCfgThread.start()
        
                    
    def addChannel(self, channelJson, index, mode):
        return self.sendCommand({'addChannel': 
                                 {'index': index, 
                                 'mode': mode,
                                 'channel': channelJson
                                 }
                                 })
                                  
    def getVersion(self, sync = False):
        rsp = self.sendCommand({"getVer":None}, sync)
        return rsp

    def autoDetect(self):
        ports = [x[0] for x in list_ports.comports()]

        self.retryCount = 0
        self.timeout = 0.5
        self.writeTimeout = 0
        print "Searching for RaceCapture on all serial ports"
        testVer = VersionConfig()
        verJson = None
        for p in ports:
            try:
                print "Trying", p
                self.port = p
                verJson = self.getVersion(True)
                testVer.fromJson(verJson.get('ver', None))
                if testVer.major > 0 or testVer.minor > 0 or testVer.bugfix > 0:
                    break
            except Exception as detail:
                print('Not found on ' + self.port)
                #traceback.print_exc()                
                self.port = None
                self.close()

        self.retryCount = DEFAULT_READ_RETRIES
        self.timeout = DEFAULT_SERIAL_READ_TIMEOUT
        self.writeTimeout = DEFAULT_SERIAL_WRITE_TIMEOUT
        if not verJson == None:
            print "Found racecapture version " + testVer.toString() + " on port:", self.port
            self.close()
        


    

