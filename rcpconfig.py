import json

MAX_ANALOG_RAW_VALUE = 1023
MIN_ANALOG_RAW_VALUE = 0

class ScalingMap:
    def __init__(self, **kwargs):
        points = 5
        raw = []
        scaled = []
        for i in range(points):
            raw.append(0)
            scaled.append(0)
        self.points = points
        self.raw = raw
        self.scaled = scaled
 
    def fromJson(self, mapJson):
        rawJson = mapJson.get('raw', None)
        if rawJson:
            i = 0
            for rawValue in rawJson:
                self.raw[i] = rawValue
                i+=1

        scaledJson = mapJson.get('scal', None)
        if scaledJson:
            i = 0
            for scaledValue in scaledJson:
                self.scaled[i] = scaledValue
                i+=1
                
    def toJson(self):
        mapJson = {}
        rawBins = []
        scaledBins = []
        for rawValue in self.raw:
            rawBins.append(rawValue)
        
        for scaledValue in self.scaled:
            scaledBins.append(scaledValue)
            
        mapJson['raw'] = rawBins
        mapJson['scal'] = scaledBins
        
        return mapJson 
        
    def getVolts(self, mapBin):
        try:
            return (5.0 * self.raw[mapBin]) / 1024.0
        except IndexError:
            print('Index error getting volts')
            return 0
     
    def setVolts(self, mapBin, value):
        try:
            value = float(value)
            raw = value * 204.6
            raw = int(raw)
            raw = MAX_ANALOG_RAW_VALUE if raw >= MAX_ANALOG_RAW_VALUE else raw
            raw = MIN_ANALOG_RAW_VALUE if raw <= MIN_ANALOG_RAW_VALUE else raw
            self.raw[mapBin] = raw
        except IndexError:
            print('Index error setting bin')
            
    def getScaled(self, mapBin):
        try:
            return self.scaled[mapBin]
        except IndexError:
            print('Index error getting scaled value')
            return 0
    
    def setScaled(self, mapBin, value):
        try:
            self.scaled[mapBin] = float(value)
        except IndexError:
            print('Index error setting bin')
            
ANALOG_SCALING_MODE_RAW     = 0
ANALOG_SCALING_MODE_LINEAR  = 1
ANALOG_SCALING_MODE_MAP     = 2

class AnalogChannel:
    def __init__(self, **kwargs):
        self.channelId = 0
        self.sampleRate = 0
        self.scalingMode = 0
        self.linearScaling = 0
        self.alpha = 0
        self.scalingMap = ScalingMap()
    
    def fromJson(self, json):
        self.channelId = json.get('id', self.channelId)
        self.sampleRate = json.get('sr', self.sampleRate)
        self.scalingMode = json.get('scalMod', self.scalingMode)
        self.linearScaling = json.get('linScal', self.linearScaling)
        self.alpha = json.get('alpha', self.alpha)
        mapJson = json.get('map', None)
        if mapJson:
            self.scalingMap.fromJson(mapJson)
            
    def toJson(self):
        channelJson = {}
        channelJson['id'] = self.channelId
        channelJson['sr'] = self.sampleRate
        channelJson['scalMod'] = self.scalingMode
        channelJson['linScal'] = self.linearScaling
        channelJson['alpha'] = self.alpha
        channelJson['map'] = self.scalingMap.toJson()
        return channelJson
    
ANALOG_CHANNEL_COUNT = 8

class AnalogConfig:
    def __init__(self, **kwargs):
        self.channelCount = ANALOG_CHANNEL_COUNT
        self.channels = []

        for i in range (self.channelCount):
            self.channels.append(AnalogChannel())   

    def fromJson(self, analogCfgJson):
        for i in range (self.channelCount):
            analogChannelJson = analogCfgJson.get(str(i), None)
            if analogChannelJson:
                self.channels[i].fromJson(analogChannelJson)

    def toJson(self):
        analogCfgJson = {}
        for i in range(ANALOG_CHANNEL_COUNT):
            analogChannel = self.channels[i]
            analogCfgJson[str(i)] = analogChannel.toJson()
        return {'analogCfg':analogCfgJson}
            
        
class ImuChannel:
    def __init__(self, **kwargs):
        self.sampleRate = 0
        self.mode = 0
        self.chan = 0
        self.zeroValue = 0
        self.alpha = 0

    def fromJson(self, imuChannelJson):
        self.sampleRate =  imuChannelJson.get('sr', self.sampleRate)
        self.mode = imuChannelJson.get('mode', self.mode)
        self.chan = imuChannelJson.get('chan', self.chan)
        self.zeroValue = imuChannelJson.get('zeroVal', self.zeroValue)
        self.alpha = imuChannelJson.get('alpha', self.alpha)
        
    def toJson(self):
        jsonCfg = {}
        jsonCfg['sr'] = self.sampleRate
        jsonCfg['mode'] = self.mode
        jsonCfg['chan'] = self.chan
        jsonCfg['zeroVal'] = self.zeroValue
        jsonCfg['alpha'] = self.alpha
        return jsonCfg
    
IMU_CHANNEL_COUNT = 4
IMU_ACCEL_CHANNEL_IDS   = [0,1,2]
IMU_GYRO_CHANNEL_IDS    = [3]
IMU_MODE_DISABLED       = 0
IMU_MODE_NORMAL         = 1
IMU_MODE_INVERTED       = 2

class ImuConfig:
    def __init__(self, **kwargs):
        self.channelCount = IMU_CHANNEL_COUNT
        self.channels = []
        
        for i in range(self.channelCount):
            self.channels.append(ImuChannel())

    def fromJson(self, imuConfigJson):
        for i in range (self.channelCount):
            imuChannelJson = imuConfigJson.get(str(i), None)
            if imuChannelJson:
                self.channels[i].fromJson(imuChannelJson)
                
    def toJson(self):
        imuCfgJson = {}
        for i in range (self.channelCount):
            imuChannel = self.channels[i]
            imuCfgJson[str(i)]=imuChannel.toJson()
            
        return {'imuCfg':imuCfgJson}
        
            
                  
class GpsConfig:
    def __init__(self, **kwargs):
        self.sampleRate = 0
        self.positionEnabled = False
        self.speedEnabled = False
        self.distanceEnabled = False
        self.timeEnabled = False
        self.satellitesEnabled = False

    def fromJson(self, json):
        if json:
            self.sampleRate = int(json.get('sr', self.sampleRate))
            self.positionEnabled = int(json.get('pos', self.positionEnabled))
            self.speedEnabled = int(json.get('speed', self.speedEnabled))
            self.timeEnabled = int(json.get('time', self.timeEnabled))
            self.distanceEnabled = int(json.get('dist', self.timeEnabled))
            self.satellitesEnabled = int(json.get('sats', self.satellitesEnabled))
            
    def toJson(self):
        gpsJson = {'gpsCfg':{
                              'sr' : self.sampleRate,
                              'pos' : self.positionEnabled,
                              'speed' : self.speedEnabled,
                              'time' : self.timeEnabled,
                              'dist' : self.distanceEnabled,
                              'sats' : self.satellitesEnabled
                              }
                    }
                   
        return gpsJson
        
    
TIMER_CHANNEL_COUNT = 3

class TimerChannel:
    def __init__(self, **kwargs):
        self.channelId = 0
        self.sampleRate = 0
        self.mode = 0
        self.divider = 0
        self.pulsePerRev = 0
        self.slowTimer = 0
        
    def fromJson(self, timerJson):
        if (timerJson):
            self.channelId = timerJson.get('id', self.channelId)
            self.sampleRate = timerJson.get('sr', self.sampleRate)
            self.mode = timerJson.get('mode', self.mode)
            self.divider = timerJson.get('div', self.divider)
            self.pulsePerRev = timerJson.get('ppr', self.pulsePerRev)
            self.slowTimer = timerJson.get('st', self.slowTimer)
            
    def toJson(self):
        timerJson = {}
        timerJson['id'] = self.channelId
        timerJson['sr'] = self.sampleRate
        timerJson['mode'] = self.mode
        timerJson['ppr'] = self.pulsePerRev
        timerJson['div'] = self.divider
        timerJson['st'] = self.slowTimer
        return timerJson

class TimerConfig:
    def __init__(self, **kwargs):
        self.channelCount = TIMER_CHANNEL_COUNT
        self.channels = []

        for i in range (self.channelCount):
            self.channels.append(TimerChannel())   

    def fromJson(self, json):
        for i in range (self.channelCount):
            timerChannelJson = json.get(str(i), None)
            if timerChannelJson:
                self.channels[i].fromJson(timerChannelJson)
    
    def toJson(self):
        timerCfgJson = {}
        for i in range(TIMER_CHANNEL_COUNT):
            timerChannel = self.channels[i]
            timerCfgJson[str(i)] = timerChannel.toJson()
        return {'timerCfg':timerCfgJson}
        
class GpioChannel:
    def __init__(self, **kwargs):
        self.channelId = 0
        self.sampleRate = 0
        self.mode = 0
        
    def fromJson(self, json):
        if (json):
            self.channelId = json.get('id', self.channelId)
            self.sampleRate = json.get('sr', self.sampleRate)
            self.mode = json.get('mode', self.mode)
            
    def toJson(self):
        gpioJson = {}
        gpioJson['sr'] = self.sampleRate
        gpioJson['id'] = self.channelId
        gpioJson['mode'] = self.mode
        return gpioJson
         
GPIO_CHANNEL_COUNT = 3

class GpioConfig:
    def __init__(self, **kwargs):
        self.channelCount = GPIO_CHANNEL_COUNT
        self.channels = []

        for i in range (self.channelCount):
            self.channels.append(GpioChannel())   

    def fromJson(self, json):
        for i in range (self.channelCount):
            channelJson = json.get(str(i), None)
            if channelJson:
                self.channels[i].fromJson(channelJson)
                
    def toJson(self):
        gpioCfgJson = {}
        for i in range(GPIO_CHANNEL_COUNT):
            gpioChannel = self.channels[i]
            gpioCfgJson[str(i)] = gpioChannel.toJson()
        return {'gpioCfg':gpioCfgJson}
        
    
class PwmChannel:
    def __init__(self, **kwargs):
        self.channelId = 0
        self.sampleRate = 0
        self.outputMode = 0
        self.loggingMode = 0
        self.startupPeriod = 0
        self.startupDutyCycle = 0
        
    def fromJson(self, json):
        if (json):
            self.channelId = json.get('id', self.channelId)
            self.sampleRate = json.get('sr', self.sampleRate)
            self.outputMode = json.get('outMode', self.outputMode)
            self.loggingMode = json.get('logMode', self.loggingMode)
            self.startupDutyCycle = json.get('stDutyCyc', self.startupDutyCycle)
            self.startupPeriod = json.get('stPeriod', self.startupPeriod)
            
    def toJson(self):
        pwmJson = {}
        pwmJson['id'] = self.channelId
        pwmJson['sr'] = self.sampleRate
        pwmJson['outMode'] = self.outputMode
        pwmJson['logMode'] = self.loggingMode
        pwmJson['stDutyCyc'] = self.startupDutyCycle
        pwmJson['stPeriod'] = self.startupPeriod
        return pwmJson

PWM_CHANNEL_COUNT = 4   

class PwmConfig:
    def __init__(self, **kwargs):
        self.channelCount = PWM_CHANNEL_COUNT
        self.channels = []

        for i in range (self.channelCount):
            self.channels.append(PwmChannel())   

    def fromJson(self, json):
        for i in range (self.channelCount):
            channelJson = json.get(str(i), None)
            if channelJson:
                self.channels[i].fromJson(channelJson)

    def toJson(self):
        pwmCfgJson = {}
        for i in range(PWM_CHANNEL_COUNT):
            pwmChannel = self.channels[i]
            pwmCfgJson[str(i)] = pwmChannel.toJson()
        return {'pwmCfg':pwmCfgJson}
        
        
CONFIG_SECTOR_COUNT = 20

class GeoPoint:
    def __init__(self, **kwargs):
        self.latitude = 0
        self.longitude = 0
    
    def fromJson(self, json):
        self.latitude = json[0]
        self.longitude = json[1]
    
    def toJson(self):
        return [self.latitude, self.longitude]
        
TRACK_TYPE_CIRCUIT  = 0
TRACK_TYPE_STAGE    = 1

CONFIG_SECTOR_COUNT_CIRCUIT = 19
CONFIG_SECTOR_COUNT_STAGE = 18

class TrackConfig:
    def __init__(self, **kwargs):
        self.sectorCount = CONFIG_SECTOR_COUNT
        self.startLine = GeoPoint()
        self.finishLine = GeoPoint()
        self.sectors = []
        self.radius = 0
        self.autoDetect = 0
        self.trackType = TRACK_TYPE_CIRCUIT
        
    def fromJson(self, json):
        self.radius = json.get('rad', self.radius)
        self.autoDetect = json.get('autoDetect', self.autoDetect)
        
        trackJson = json.get('track', None)
        if trackJson:
            self.trackType = trackJson.get('type', self.trackType)
            sectorsJson = trackJson.get('sec', None)
            del self.sectors[:]
            sectorCount = CONFIG_SECTOR_COUNT_CIRCUIT if self.trackType == TRACK_TYPE_CIRCUIT else CONFIG_SECTOR_COUNT_STAGE
            returnedSectorCount = len(sectorsJson)
            if sectorsJson:
                for i in range(sectorCount):
                    sector = GeoPoint()
                    if i < returnedSectorCount:
                        sectorJson = sectorsJson[i]
                        sector.fromJson(sectorJson)
                    self.sectors.append(sector)
            self.sectorCount = sectorCount
                    
    def toJson(self):
        trackCfgJson = {}
        trackCfgJson['rad'] = self.radius
        trackCfgJson['autoDetect'] = 1 if self.autoDetect else 0 
        
        sectors = []
        for sector in self.sectors:
            sectors.append(sector.toJson())
        trackJson = {}
        trackJson['sec']  = sectors
        trackJson['type'] = self.trackType
        
        if self.trackType == TRACK_TYPE_STAGE:
            trackJson['st'] = self.startLine.toJson()
            trackJson['fin'] = self.finishLine.toJson()
        else:
            trackJson['sf'] = self.startLine.toJson()
            
        trackCfgJson['track'] = trackJson
            
        return {'trackCfg':trackCfgJson}

class PidConfig:
    def __init__(self, **kwargs):
        self.channelId = 0
        self.sampleRate = 0
        self.pidId = 0
        
    def fromJson(self, json):
        self.channelId = json.get("id", self.channelId)
        self.sampleRate = json.get("sr", self.sampleRate)
        self.pid = json.get("pid", self.pidId)
        
    def toJson(self):
        pidJson = {}
        pidJson['id'] = self.channelId
        pidJson['sr'] = self.sampleRate
        pidJson['pid'] = self.pidId
        return pidJson

class CanConfig:
    enabled = False
    baudRate = 0

    def __init__(self, **kwargs):
        pass
    
    def fromJson(self, canCfgJson):
        self.enabled = True if canCfgJson.get('en', self.enabled) == 1 else False 
        self.baudRate = canCfgJson.get('baud', self.baudRate)
        
    def toJson(self):
        canCfgJson = {}
        canCfgJson['en'] = 1 if self.enabled else 0
        canCfgJson['baud'] = self.baudRate
        return {'canCfg':canCfgJson}        
        
        
            
OBD2_CONFIG_MAX_PIDS = 20

class Obd2Config:
    pids = []
    enabled = False
    def __init__(self, **kwargs):
        pass
    
    def fromJson(self, obd2CfgJson):
        self.enabled = obd2CfgJson.get('en', self.enabled) 
        pidsJson = obd2CfgJson.get("pids", None)
        if pidsJson:
            del self.pids[:]
            for pidJson in pidsJson:
                pid = PidConfig()
                pid.fromJson(pidJson)
                self.pids.append(pid)
                
    def toJson(self):
        pidsJson = []
        pidCount = len(self.pids)
        pidCount = pidCount if pidCount <= OBD2_CONFIG_MAX_PIDS else OBD2_CONFIG_MAX_PIDS
        
        for i in range(pidCount):
            pidsJson.append(self.pids[i].toJson())
            
        obd2Json =  {'obd2Cfg':{'en': 1 if self.enabled else 0, 'pids':pidsJson }}
        return obd2Json
        
class LuaScript:
    script = ""
    def __init__(self, **kwargs):
        pass
            
    def fromJson(self, jsonScript):
        self.script = jsonScript['data']
        
    def toJson(self):
        scriptJson = {"scriptCfg":{'data':self.script,'page':None}}
        return scriptJson
        
class BluetoothConfig:
    name = ""
    passKey = ""
    btEnabled = False
    def __init__(self, **kwargs):
        pass

    def fromJson(self, btCfgJson):
        self.btEnabled = btCfgJson['btEn'] == 1
        self.name = btCfgJson['name']
        self.passKey = btCfgJson['pass']
        
    def toJson(self):
        btCfgJson = {}
        btCfgJson['btEn'] = 1 if self.btEnabled else 0
        btCfgJson['name'] = self.name
        btCfgJson['passKey'] = self.passKey
        return btCfgJson 

class CellConfig:
    cellEnabled = False
    apnHost = ""
    apnUser = ""
    apnPass = ""
    def __init__(self, **kwargs):
        pass
    
    def fromJson(self, cellCfgJson):
        self.cellEnabled = cellCfgJson['cellEn'] == 1
        self.apnHost = cellCfgJson['apnHost']
        self.apnUser = cellCfgJson['apnUser']
        self.apnPass = cellCfgJson['apnPass']

    def toJson(self):
        cellConfigJson = {}
        cellConfigJson['cellEn'] = 1 if self.cellEnabled else 0
        cellConfigJson['apnHost'] = self.apnHost
        cellConfigJson['apnUser'] = self.apnUser
        cellConfigJson['apnPass'] = self.apnPass
        return cellConfigJson        
    
class TelemetryConfig:
    deviceId = ""
    backgroundStreaming = 0
    
    def fromJson(self, telCfgJson):
        self.deviceId = telCfgJson['deviceId']
        self.backgroundStreaming = True if telCfgJson['bgStream'] == 1 else False 
        
    def toJson(self):
        telCfgJson = {}
        telCfgJson['deviceId'] = self.deviceId
        telCfgJson['bgStream'] = 1 if self.backgroundStreaming else 0
        return telCfgJson
    
class ConnectivityConfig:
    bluetoothConfig = BluetoothConfig()
    cellConfig = CellConfig()
    telemetryConfig = TelemetryConfig()
    
    def fromJson(self, connCfgJson):
        btCfgJson = connCfgJson.get('btCfg')
        if btCfgJson:
            self.bluetoothConfig.fromJson(btCfgJson)
            
        cellCfgJson = connCfgJson.get('cellCfg')
        if cellCfgJson:
            self.cellConfig.fromJson(cellCfgJson)
            
        telCfgJson = connCfgJson.get('telCfg')
        if telCfgJson:
            self.telemetryConfig.fromJson(telCfgJson)
            
    def toJson(self):
        connCfgJson = {'btCfg' : self.bluetoothConfig.toJson(),
                       'cellCfg' : self.cellConfig.toJson(),
                       'telCfg' : self.telemetryConfig.toJson()}
        
        return {'connCfg':connCfgJson}


class VersionConfig:
    major = 0
    minor = 0
    bugfix = 0
    def __init__(self, **kwargs):
        pass
    
    def fromJson(self, versionJson):
        self.major = versionJson.get('major', self.major)
        self.minor = versionJson.get('minor', self.minor)
        self.bugfix = versionJson.get('bugfix', self.bugfix)
        
    def toJson(self):
        versionJson = {'major': self.major, 'minor': self.minor, 'bugfix': self.bugfix}
        return {'ver': versionJson}
        
class RcpConfig:
    loaded = False
    def __init__(self, **kwargs):
        self.versionConfig = VersionConfig()
        self.analogConfig = AnalogConfig()
        self.imuConfig = ImuConfig()
        self.gpsConfig = GpsConfig()
        self.timerConfig = TimerConfig()
        self.gpioConfig = GpioConfig()
        self.pwmConfig = PwmConfig()
        self.trackConfig = TrackConfig()
        self.connectivityConfig = ConnectivityConfig()
        self.canConfig = CanConfig()
        self.obd2Config = Obd2Config()
        self.scriptConfig = LuaScript()
    
    def fromJson(self, json):
        versionJson = json.get('ver', None)
        if versionJson:
            self.versionConfig.fromJson(versionJson)

        analogCfgJson = json.get('analogCfg', None)
        if analogCfgJson:
            self.analogConfig.fromJson(analogCfgJson)

        timerCfgJson = json.get('timerCfg', None)
        if timerCfgJson:
            self.timerConfig.fromJson(timerCfgJson)
            
        imuCfgJson = json.get('imuCfg', None)
        if imuCfgJson:
            self.imuConfig.fromJson(imuCfgJson)
            
        gpsCfgJson = json.get('gpsCfg', None)
        if gpsCfgJson:
            self.gpsConfig.fromJson(gpsCfgJson)
            
        gpioCfgJson = json.get('gpioCfg', None)
        if gpioCfgJson:
            self.gpioConfig.fromJson(gpioCfgJson)
            
        pwmCfgJson = json.get('pwmCfg', None)
        if pwmCfgJson:
            self.pwmConfig.fromJson(pwmCfgJson)
            
        trackCfgJson = json.get('trackCfg', None)
        if trackCfgJson:
            self.trackConfig.fromJson(trackCfgJson)
            
        connectivtyCfgJson = json.get('connCfg', None)
        if connectivtyCfgJson:
            self.connectivityConfig.fromJson(connectivtyCfgJson)
            
        canCfgJson = json.get('canCfg', None)
        if canCfgJson:
            self.canConfig.fromJson(canCfgJson)
            
        obd2CfgJson = json.get('obd2Cfg', None)
        if obd2CfgJson:
            self.obd2Config.fromJson(obd2CfgJson)
        
        scriptJson = json.get('scriptCfg', None)
        if scriptJson:
            self.scriptConfig.fromJson(scriptJson)
            
        print('RCP config version ' + str(self.versionConfig.major) + '.' + str(self.versionConfig.minor) + '.' + str(self.versionConfig.minor) + ' Loaded')
        self.loaded = True
    
    def fromJsonString(self, rcpJsonString):
        rcpJson = json.loads(rcpJsonString)
        self.fromJson(rcpJson)
        
    def toJsonString(self, pretty = True):
        return json.dumps(self.toJson(), sort_keys=True, indent=2, separators=(',', ': '))

    def toJson(self):
        rcpJson = {'rcpCfg':{
                             'ver': self.versionConfig.toJson().get('ver'),
                             'gpsCfg':self.gpsConfig.toJson().get('gpsCfg'),
                             'imuCfg':self.imuConfig.toJson().get('imuCfg'),
                             'analogCfg':self.analogConfig.toJson().get('analogCfg'),
                             'timerCfg':self.timerConfig.toJson().get('timerCfg'),
                             'gpioCfg':self.gpioConfig.toJson().get('gpioCfg'),
                             'pwmCfg':self.pwmConfig.toJson().get('pwmCfg'),
                             'canCfg':self.canConfig.toJson().get('canCfg'),
                             'obd2Cfg':self.obd2Config.toJson().get('obd2Cfg'),
                             'connCfg':self.connectivityConfig.toJson().get('connCfg'),
                             'trackCfg':self.trackConfig.toJson().get('trackCfg'),
                             'scriptCfg':self.scriptConfig.toJson().get('scriptCfg')
                             }
                   }
        print('\n\n\n\nRCP JSON ' + json.dumps(rcpJson) + '\n\n\n')
        return rcpJson
