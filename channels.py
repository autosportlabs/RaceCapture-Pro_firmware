class Channel:
    def __init__(self, **kwargs):
        self.name = kwargs.get('name', 'Unknown')
        self.units = kwargs.get('units', 'Unknown')

class Channels:
    def __init__(self, **kwargs):
        items = []
        items.append(Channel(name='OilTemp', units='F'))
        items.append(Channel(name='Battery', units='V'))
        items.append(Channel(name='AFR', units='AFR'))
        self.items = items

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

                
