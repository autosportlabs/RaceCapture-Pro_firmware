import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from autosportlabs.uix.track.trackmap import TrackMap
from autosportlabs.uix.track.racetrackview import RaceTrackView
from utils import *
from autosportlabs.racecapture.geo.geopoint import GeoPoint

Builder.load_file('autosportlabs/racecapture/views/tracks/tracksview.kv')

class TracksView(BoxLayout):
    trackmap = None
    
    def __init__(self, **kwargs):
        super(TracksView, self).__init__(**kwargs)
        self.register_event_type('on_channels_updated')
        
    def on_channels_updated(self, channels):
        pass
    
