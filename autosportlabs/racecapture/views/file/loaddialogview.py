import kivy
kivy.require('1.8.0')

from kivy.properties import ObjectProperty
from kivy.uix.floatlayout import FloatLayout
from kivy.app import Builder

Builder.load_file('autosportlabs/racecapture/views/file/loaddialogview.kv')

class LoadDialog(FloatLayout):
    load = ObjectProperty(None)
    cancel = ObjectProperty(None)