import kivy
kivy.require('1.8.0')

from kivy.properties import ObjectProperty
from kivy.uix.floatlayout import FloatLayout
from kivy.app import Builder

Builder.load_file('autosportlabs/racecapture/views/file/savedialogview.kv')

class SaveDialog(FloatLayout):
    save = ObjectProperty(None)
    text_input = ObjectProperty(None)
    cancel = ObjectProperty(None)
