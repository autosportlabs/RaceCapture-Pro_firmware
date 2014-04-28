import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder

Builder.load_file('obd2channelsview.kv')

class OBD2ChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(OBD2ChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')

    def on_config_updated(self, rcpCfg):
        pass
