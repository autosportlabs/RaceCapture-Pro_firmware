
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

class AnalogChannel:
    def __init__(self, **kwargs):
        self.channelId = 0
        self.sampleRate = 0
        self.scalingMode = 0
        self.linearScaling = 0
        self.alpha = 0
        self.scalingMap = ScalingMap()
    
    def fromJson(self, analogChannelJson):
        self.channelId = analogChannelJson.get('id', self.channelId)
        self.sampleRate = analogChannelJson.get('sr', self.sampleRate)
        self.scalingMode = analogChannelJson.get('scalMod', self.scalingMode)
        self.linearScaling = analogChannelJson.get('scalMod', self.linearScaling)
        self.alpha = analogChannelJson.get('alpha', self.alpha)
        mapJson = analogChannelJson.get('map', None)
        if mapJson:
            self.scalingMap.fromJson(mapJson)
    
class AnalogConfig:
    def __init__(self, **kwargs):
        self.channelCount = 8
        self.channels = []

        for i in range (self.channelCount):
            self.channels.append(AnalogChannel())   

    def fromJson(self, analogCfgJson):
        for i in range (self.channelCount):
            analogChannelJson = analogCfgJson.get(str(i), None)
            if analogChannelJson:
                self.channels[i].fromJson(analogChannelJson)
                  
class RcpConfig:
    def __init__(self, **kwargs):
        self.analogConfig = AnalogConfig()

    def fromJson(self, json):
        analogCfgJson = json.get('analogCfg', None)
        if analogCfgJson:
            self.analogConfig.fromJson(analogCfgJson)                

    def toJson():
        print('toJson')
    
