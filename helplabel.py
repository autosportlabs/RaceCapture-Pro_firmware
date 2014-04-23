import kivy
kivy.require('1.8.0')

from kivy.uix.label import Label
from fieldlabel import FieldLabel

class HelpLabel(FieldLabel):
    def __init__(self, **kwargs):
        super(HelpLabel, self).__init__(**kwargs)
        self.color = (0.5, 0.5, 0.5, 1.0)
    
    