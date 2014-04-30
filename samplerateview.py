import kivy
kivy.require('1.8.0')
from kivy.uix.spinner import Spinner
from kivy.uix.boxlayout import BoxLayout

class SampleRateSpinner(Spinner):
    def __init__(self, **kwargs):
        super(SampleRateSpinner, self).__init__(**kwargs)
        self.values = ['Disabled', '1 Hz', '5 Hz', '10 Hz', '25 Hz', '50 Hz', '100 Hz']

class SampleRateSelectorView(BoxLayout):
    def __init__(self, **kwargs):
        super(SampleRateSelectorView, self).__init__(**kwargs)
        self.valueMap = {0:'Disabled', 1:'1 Hz', 5:'5 Hz', 10:'10 Hz', 25:'25 Hz', 50:'50 Hz', 100:'100 Hz'}

    def setValue(self, value):
        self.ids.sampleRate.text = self.valueMap[value]
