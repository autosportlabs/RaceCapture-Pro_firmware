import kivy
kivy.require('1.8.0')
from kivy.uix.label import Label
from kivy.app import Builder

Builder.load_file('statusicon.kv')

class StatusIcon(Label):
    pass
