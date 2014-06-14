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
    track = None
    def __init__(self, **kwargs):
        super(TrackItemView, self).__init__(**kwargs)
        track = kwargs.get('track', None)
        if track:
            raceTrackView = kvFind(self, 'rcid', 'track')
            raceTrackView.loadTrack(track)
    
class TracksView(BoxLayout):
    trackmap = None
    trackMinHeight = dp(300)
    trackManager = None
    def __init__(self, **kwargs):
        super(TracksView, self).__init__(**kwargs)
        self.trackManager = kwargs.get('trackManager')
        self.register_event_type('on_channels_updated')
        
    def on_channels_updated(self, channels):
        pass
    
    def on_update_check(self):
        self.trackManager.downloadAllTracks()
        self.initTracksList(self.trackManager.tracks)
        
    def initTracksList(self, tracks):
        trackCount = len(tracks)
        grid = kvFind(self, 'rcid', 'tracksgrid')
        grid.height = self.trackMinHeight * trackCount
        grid.clear_widgets()
        
        for venueId in tracks.keys():
            track = tracks[venueId]
            trackView = TrackItemView(track=track)
            grid.add_widget(trackView)
