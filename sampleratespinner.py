import kivy
kivy.require('1.8.0')
from mappedspinner import MappedSpinner

class SampleRateSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(SampleRateSpinner, self).__init__(**kwargs)
        self.setValueMap({0:'Disabled', 1:'1 Hz', 5:'5 Hz', 10:'10 Hz', 25:'25 Hz', 50:'50 Hz', 100:'100 Hz'}, 'Disabled')
