import kivy
kivy.require('1.8.0')

from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder

Builder.load_file('trackconfigview.kv')

class TargetConfigView(GridLayout):
    def __init__(self, **kwargs):
        super(TargetConfigView, self).__init__(**kwargs)

class TrackConfigView(BoxLayout):
    def __init__(self, **kwargs):
        super(TrackConfigView, self).__init__(**kwargs)
#        Builder.load_file('analogchannelsview.kv')
        self.register_event_type('on_config_updated')

    def on_config_updated(self, rcpCfg):
        pass
