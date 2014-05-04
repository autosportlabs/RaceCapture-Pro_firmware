import kivy
kivy.require('1.8.0')
from kivy.uix.spinner import Spinner
from kivy.uix.boxlayout import BoxLayout
from utils import *
from sampleratespinner import SampleRateSpinner

from kivy.app import Builder
Builder.load_file('samplerateview.kv')            


class SampleRateSelectorView(BoxLayout):
    def __init__(self, **kwargs):
        super(SampleRateSelectorView, self).__init__(**kwargs)
        self.register_event_type('on_sample_rate')

    def on_sample_rate(self, value):
        pass
    
    def setValue(self, value):
        kvFind(self, 'rcid', 'sampleRate').setFromValue(value)

    def onSelect(self, instance, value):
        selectedValue = int(instance.getValueFromKey(value))
        self.dispatch('on_sample_rate', selectedValue)
        
