import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.app import Builder
from kivy.metrics import dp
from autosportlabs.uix.track.trackmap import TrackMap
from autosportlabs.uix.track.racetrackview import RaceTrackView
from utils import *
from autosportlabs.racecapture.geo.geopoint import GeoPoint

Builder.load_file('autosportlabs/racecapture/views/tracks/tracksview.kv')

class TrackItemView(BoxLayout):
    def __init__(self, **kwargs):
        super(TrackItemView, self).__init__(**kwargs)
    
class TracksView(BoxLayout):
    trackmap = None
    trackMinHeight = dp(100)
    def __init__(self, **kwargs):
        super(TracksView, self).__init__(**kwargs)
        self.register_event_type('on_channels_updated')
        self.initTracksList()
        
    def on_channels_updated(self, channels):
        pass
    
    def initTracksList(self):
        
        trackCount = 50
        grid = kvFind(self, 'rcid', 'tracksgrid')
        grid.height = self.trackMinHeight * trackCount
        for i in range(trackCount):
            track = TrackItemView()
            grid.add_widget(track)
