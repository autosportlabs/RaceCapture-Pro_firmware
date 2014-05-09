import kivy
kivy.require('1.8.0')
from kivy.uix.label import Label
from kivy.uix.widget import Widget
from kivy.app import Builder

Builder.load_file('separator.kv')

class HLineSeparator(Label):
    pass

class HSeparator(Label):
    pass
    
class VSeparator(Widget):
    pass
    
class HSeparatorMinor(Label):
    pass