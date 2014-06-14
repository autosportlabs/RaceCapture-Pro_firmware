import kivy
kivy.require('1.8.0')
from kivy.uix.label import Label
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.widget import Widget
from kivy.uix.scatter import Scatter
from kivy.app import Builder
from kivy.metrics import dp
from kivy.graphics import Color, Line
from autosportlabs.racecapture.geo.geopoint import GeoPoint
from utils import *

Builder.load_file('autosportlabs/uix/track/racetrackview.kv')
        
class RaceTrackView(BoxLayout):
    trackmap = None
    def __init__(self, **kwargs):
        super(RaceTrackView, self).__init__(**kwargs)

    def loadTrack(self, track):
        self.initMap(track)
                
    def initMap(self, track):
        trackmap = kvFind(self, 'rcid', 'trackmap1')  
        trackmap.setTrackPoints(track.mapPoints)

