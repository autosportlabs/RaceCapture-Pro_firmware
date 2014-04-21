import kivy
kivy.require('1.8.0')

from kivy.uix.spinner import Spinner

class MappedSpinner(Spinner):
    def __init__(self, **kwargs):
        super(MappedSpinner, self).__init__(**kwargs)
        
    def setValueMap(self, valueMap, defaultValue):
        self.valueMappings = valueMap
        self.defaultValue = defaultValue
        values = []
        for k,v in valueMap.items():
            values.append(v)
        try:
            values.sort(key=int)
        except ValueError:
            values.sort()
            
        self.values = values
            
        
    def setFromValue(self, value):
        self.text = self.valueMappings.get(value, self.defaultValue)

