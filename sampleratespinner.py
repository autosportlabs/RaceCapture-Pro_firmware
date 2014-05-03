import kivy
kivy.require('1.8.0')
from kivy.uix.spinner import Spinner

class SampleRateSpinner(Spinner):
    def __init__(self, **kwargs):
        super(SampleRateSpinner, self).__init__(**kwargs)
        self.values = ['Disabled', '1 Hz', '5 Hz', '10 Hz', '25 Hz', '50 Hz', '100 Hz']
