import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout


class BaseConfigView(BoxLayout):
    def __init__(self, **kwargs):    
        super(BaseConfigView, self).__init__(**kwargs)
        self.register_event_type('on_tracks_updated')
        
        
    def on_tracks_updated(self, trackManager):
        pass
        
