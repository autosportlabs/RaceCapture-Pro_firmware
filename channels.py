CHANNEL_TYPE_UNKNOWN    = 0
CHANNEL_TYPE_ANALOG     = 1
CHANNEL_TYPE_FREQ       = 2
CHANNEL_TYPE_GPIO       = 3
CHANNEL_TYPE_PWM        = 4
CHANNEL_TYPE_IMU        = 5
CHANNEL_TYPE_GPS        = 6
CHANNEL_TYPE_STATISTICS = 7

class Channel:
    name = ""
    units = ""
    precision = 0
    min = 0
    max = 0
    systemChannel = 0
    channelType = None
    
    def __init__(self, **kwargs):
        self.name = kwargs.get('name', self.name)
        self.units = kwargs.get('units', self.units)
        self.precision = kwargs.get('precision', self.precision)
        self.min = kwargs.get('min', self.min)
        self.max = kwargs.get('max', self.max)
        
    def fromJson(self, channelJson):
        if channelJson:
            self.name = channelJson.get('nm', self.name)
            self.units = channelJson.get('ut', self.units)
            self.precision = channelJson.get('prec', self.precision)
            self.min = channelJson.get('min', self.min)
            self.max = channelJson.get('max', self.max)
            sysChannel = channelJson.get('sys', None)
            if sysChannel:
                self.systemChannel = True if sysChannel == 1 else False
            self.channelType = channelJson.get('type', self.channelType)

    def toJson(self):
        channelJson = {}
        channelJson['nm'] = self.name
        channelJson['ut'] = self.units
        channelJson['prec'] = self.precision
        channelJson['min'] = self.min
        channelJson['max'] = self.max
        channelJson['sys'] = 1 if self.systemChannel else 0
        channelJson['type'] = self.channelType
        
class Channels:
    items = []
    def __init__(self, **kwargs):
        pass

    def fromJson(self, channelsJson):
        channelsList = channelsJson.get('channels')
        if channelsList:
            del self.items[:]
            for channelJson in channelsList:
                channel = Channel()
                channel.fromJson(channelJson)
                self.items.append(channel)
                
        
    def toJson(self):
        channelsListJson = []
        for channel in self.items:
            channelsListJson.append(channel.toJson())
            
        return {'channels': channelsListJson}
        
    def isLoaded(self):
        return len(self.items) > 0
    
    def getIdForName(self, name):
        for i in range(len(self.items)):
            item = self.items[i]
            if item.name == name:
                return i
        return 0
        
    def getNameForId(self, id):
        try:
            return self.items[id].name
        except IndexError:
            return 'Unknown'

    def getUnitsForId(self, id):
        try:
            return self.items[id].units
        except IndexError:
            return ''

    def getNamesList(self, channelType):

        names = []
        for channel in self.items:
            if channelType == None or channelType == channel.channelType:
                names.append(channel.name)
        return names

                
