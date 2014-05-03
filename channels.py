class Channel:
    def __init__(self, **kwargs):
        self.name = kwargs.get('name', 'Unknown')
        self.units = kwargs.get('units', 'Unknown')

class Channels:
    items = []
    def __init__(self, **kwargs):
        items = []
        items.append(Channel(name='Unknown', units=''))
        items.append(Channel(name='OilTemp', units='F'))
        items.append(Channel(name='Battery', units='V'))
        items.append(Channel(name='AFR', units='AFR'))
        self.items = items

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

    def getNamesList(self, category):
        names = []
        for channel in self.items:
            names.append(channel.name)
        return names

                
