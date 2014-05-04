import kivy
kivy.require('1.8.0')

from kivy.uix.spinner import Spinner

class MappedSpinner(Spinner):
    def __init__(self, **kwargs):
        self.valueMappings = {}
        self.keyMappings = {}
        self.values = []
        super(MappedSpinner, self).__init__(**kwargs)
        
    def setValueMap(self, valueMap, defaultValue):
        keyMappings = {}
        values = []
        for k,v in valueMap.items():
            values.append(v)
            keyMappings[v] = k
        try:
            values.sort(key=int)
        except ValueError:
            values.sort()
            
        self.defaultValue = defaultValue
        self.valueMappings = valueMap
        self.keyMappings = keyMappings
        self.values = values
                    
    def setFromValue(self, value):
        self.text = self.valueMappings.get(value, self.defaultValue)

    def getValueFromKey(self, key):
        return self.keyMappings.get(key, None)
