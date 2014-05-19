import kivy
kivy.require('1.8.0')
from kivy.uix.spinner import Spinner

class ChannelNameSpinner(Spinner):
    channelType = None
    def __init__(self, **kwargs):
        self.register_event_type('on_channels_updated')
        super(ChannelNameSpinner, self).__init__(**kwargs)
        self.channelType = kwargs.get('channelType', None)
        self.values = []
     
    def on_channels_updated(self, channels):
        self.values = channels.getNamesList(self.channelType)
