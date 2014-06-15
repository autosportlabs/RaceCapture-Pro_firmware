import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.clock import Clock
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.app import Builder
from kivy.metrics import dp
import json
from autosportlabs.racecapture.views.util.alertview import alertPopup
from autosportlabs.uix.track.trackmap import TrackMap
from autosportlabs.uix.track.racetrackview import RaceTrackView
from utils import *
from autosportlabs.racecapture.geo.geopoint import GeoPoint
Builder.load_file('autosportlabs/racecapture/views/tracks/tracksview.kv')

class TracksUpdateStatusView(BoxLayout):
    progressView = None
    messageView = None
    def __init__(self, **kwargs):
        super(TracksUpdateStatusView, self).__init__(**kwargs)
        self.progressView = kvFind(self, 'rcid', 'progress')
        self.messageView = kvFind(self, 'rcid', 'updatemsg')
        
    def on_progress(self, count, total, message = None):
        self.progressView.value = (float(count) / float(total) * 100)
        if message:
            self.messageView.text = message
    
    def on_message(self, message):
        self.messageView.text = message

class TrackItemView(BoxLayout):
    track = None
    def __init__(self, **kwargs):
        super(TrackItemView, self).__init__(**kwargs)
        track = kwargs.get('track', None)
        if track:
            raceTrackView = kvFind(self, 'rcid', 'track')
            raceTrackView.loadTrack(track)
            trackLabel = kvFind(self, 'rcid', 'name')
            trackLabel.text = track.name
            lengthLabel = kvFind(self, 'rcid', 'length')
            lengthLabel.text = str(track.length) + ' mi.'
    
class TracksView(BoxLayout):
    trackmap = None
    trackMinHeight = dp(300)
    trackManager = None
    tracksUpdatePopup = None
    tracksGrid = None
    regionsSpinner = None
    lastNameSearch = None
    searchDelay = 1.5
    
    def __init__(self, **kwargs):
        super(TracksView, self).__init__(**kwargs)
        self.trackManager = kwargs.get('trackManager')
        self.register_event_type('on_channels_updated')
        self.register_event_type('on_tracks_updated')
        self.tracksGrid = kvFind(self, 'rcid', 'tracksgrid')
        self.regionsSpinner = kvFind(self, 'rcid', 'regions')
        self.lastNameSearch = ''
        self.setViewDisabled(True)
            
    def on_channels_updated(self, channels):
        pass
        
    def on_tracks_updated(self, trackManager):
        self.trackManager = trackManager
        self.initTracksList()
        self.initRegionsList()
        
    def setViewDisabled(self, disabled):
        kvFind(self, 'rcid', 'updatecheck').disabled = disabled
        kvFind(self, 'rcid', 'namefilter').disabled = disabled
    
    def dismissPopups(self):
        if self.tracksUpdatePopup:
            self.tracksUpdatePopup.dismiss()
         
    def searchAndUpdate(self, dt):
        foundTrackIds = self.trackManager.searchTracksByName(self.lastNameSearch)
        self.initTracksList(foundTrackIds)

    def on_search_track_name(self, instance, search):
        
        if search == '' and len(self.lastNameSearch) > 0:
            self.initTracksList()
        elif not self.lastNameSearch == search:
            self.lastNameSearch = search
            Clock.unschedule(self.searchAndUpdate)
            Clock.schedule_once(self.searchAndUpdate, self.searchDelay)
        
    def on_update_check_success(self):
        self.initTracksList()
        self.tracksUpdatePopup.content.on_message('Processing...')
        
    def on_update_check_error(self, details):
        self.initTracksList()
        self.dismissPopups() 
        print('Error updating: ' + str(details))       
        alertPopup('Error Updating', 'There was an error updating the track list.\n\nPlease check your network connection and try again')
            
    def on_update_check(self):
        self.setViewDisabled(True)
        tracksUpdateView = TracksUpdateStatusView()
        popup = Popup(title='Checking for updates', content=tracksUpdateView, auto_dismiss=False, size_hint=(None, None), size=(dp(400), dp(200)))
        popup.open()
        self.tracksUpdatePopup = popup
        self.trackManager.updateAllTracks(tracksUpdateView.on_progress, self.on_update_check_success, self.on_update_check_error)
        
    def addNextTrack(self, index, keys):
            if index < len(keys):
                track = self.trackManager.tracks[keys[index]]
                trackView = TrackItemView(track=track)
                trackView.size_hint_y = None
                trackView.height = self.trackMinHeight
                self.tracksGrid.add_widget(trackView)
                Clock.schedule_once(lambda dt: self.addNextTrack(index + 1, keys))
            else:
                self.dismissPopups()
                self.setViewDisabled(False)
                
        
    def initTracksList(self, trackIds = None):
        self.setViewDisabled(True)
        if trackIds == None:
            trackIds = self.trackManager.tracks.keys()
        trackCount = len(trackIds)
        grid = kvFind(self, 'rcid', 'tracksgrid')
        self.tracksGrid.height = self.trackMinHeight * trackCount
        self.tracksGrid.clear_widgets()
        self.addNextTrack(0, trackIds)
            
    def initRegionsList(self):
        regions = self.trackManager.regions

        values = ['All']
        for regionName in regions.keys():
            values.append(regionName)

        self.regionsSpinner.values = values
        
        
        
