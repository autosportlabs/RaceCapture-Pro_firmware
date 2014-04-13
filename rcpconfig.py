class AnalogChannel:
    def __init__(self, **kwargs):
        self.channelId = 0
    
class AnalogConfig:
    def __init__(self, **kwargs):
        self.channelCount = 8
        self.channels = []
        for i in range (self.channelCount):
            self.channels.append(AnalogChannel())   

class RcpConfig:
    def __init__(self, **kwargs):
        self.analogConfig = AnalogConfig()

    def fromJson(self, json):
        print('from json: ' + str(json))

    def toJson():
        print('toJson')
    
