from kivy.uix.boxlayout import BoxLayout

class CANChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(CANChannelsView, self).__init__(**kwargs)
#        Builder.load_file('analogchannelsview.kv')
        self.register_event_type('on_config_updated')

    def on_config_updated(self, rcpCfg):
        pass
