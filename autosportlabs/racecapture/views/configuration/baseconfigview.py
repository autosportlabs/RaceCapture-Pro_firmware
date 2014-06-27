import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout


class BaseConfigView(BoxLayout):
    def __init__(self, **kwargs):    
        super(BaseConfigView, self).__init__(**kwargs)
        self.register_event_type('on_tracks_updated')
        self.register_event_type('on_modified')
        self.register_event_type('on_config_modified')
        
    def on_modified(self, *args):
        self.dispatch('on_config_modified', *args)
    
    def on_config_modified(self, *args):
        pass
    
    def on_tracks_updated(self, trackManager):
        pass
        
    def createTitleForChannel(self, channels, channel):
        try:
            sampleRate = channel.sampleRate
            sampleRateInfo = 'Disabled' if sampleRate == 0 else (str(sampleRate) + 'Hz')
            return self.channels.getNameForId(channel.channelId) + ' ( ' + sampleRateInfo + ' )'
        except:
            return 'Unknown Channel'