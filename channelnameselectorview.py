import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.properties import StringProperty
from utils import *

Builder.load_file('channelnameselectorview.kv')

class ChannelNameSelectorView(BoxLayout):

    def __init__(self, **kwargs):
        super(ChannelNameSelectorView, self).__init__(**kwargs)
        self.register_event_type('on_channel')
    
    def setValue(self, value):
        spinner = kvFind(self, 'rcid', 'id')
        spinner.text = value

    def onSelect(self, instance, value):
        self.dispatch('on_channel', value)
        
    def on_channel(self, value):
        pass
        
        