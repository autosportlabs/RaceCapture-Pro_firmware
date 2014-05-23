import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder

from utils import *
from rcpconfig import *

Builder.load_file('autosportlabs/racecapture/views/channels/channelsview.kv')


class ChannelsView(BoxLayout):
    channels = None
    def __init__(self, **kwargs):
        super(ChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_channels_updated')
        self.values = []
     
    def on_channels_updated(self, channels):
        self.channels = channels

