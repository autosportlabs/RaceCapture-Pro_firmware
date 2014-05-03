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
        self.valueMap = {0:'Disabled', 1:'1 Hz', 5:'5 Hz', 10:'10 Hz', 25:'25 Hz', 50:'50 Hz', 100:'100 Hz'}
        self.keyMap = {'Disabled':0, '1 Hz':1, '5 Hz':5, '10 Hz':10, '25 Hz':25, '50 Hz':50, '100 Hz':100}
        self.register_event_type('on_sample_rate')

    def on_sample_rate(self, value):
        pass
    
    def setValue(self, value):
        kvFind(self, 'rcid', 'sampleRate').text = self.valueMap[value]

    def onSelect(self, instance, value):
        selectedValue = self.keyMap.get(value)
        self.dispatch('on_sample_rate', selectedValue)
        
