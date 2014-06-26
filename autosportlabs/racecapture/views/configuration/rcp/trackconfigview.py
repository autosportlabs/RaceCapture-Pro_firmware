import kivy
kivy.require('1.8.0')

from kivy.metrics import dp
from kivy.uix.screenmanager import Screen, ScreenManager
from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.scrollview import ScrollView
from kivy.uix.popup import Popup
from kivy.app import Builder
from helplabel import HelpLabel
from fieldlabel import FieldLabel
from settingsview import *
from utils import *
from rcpconfig import *
from valuefield import FloatValueField
from autosportlabs.racecapture.views.util.alertview import alertPopup
from autosportlabs.racecapture.views.tracks.tracksview import TrackInfoView, TracksView
from autosportlabs.racecapture.views.configuration.rcp.baseconfigview import BaseConfigView

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/trackconfigview.kv')

class SectorPointView(BoxLayout):
    geoPoint = None
    latView = None
    lonView = None
    def __init__(self, **kwargs):
        super(SectorPointView, self).__init__(**kwargs)
        self.register_event_type('on_config_changed')
        title = kwargs.get('title', None)
        if title:
            self.setTitle(title)

    def on_config_changed(self):
        pass
    
    def setNext(self, widget):
        self.lonView.set_next(widget)
        
    def getPrevious(self):
        return self.latView
    
    def setTitle(self, title):
        kvFind(self, 'rcid', 'title').text = title
        
    def setPoint(self, geoPoint):
        self.latView = kvFind(self, 'rcid', 'lat')
        self.lonView = kvFind(self, 'rcid', 'lon')
        self.latView.text = str(geoPoint.latitude)
        self.lonView.text = str(geoPoint.longitude)
        self.latView.set_next(self.lonView)        
        self.geoPoint = geoPoint
        
    def on_lat(self, instance, value):
        if self.geoPoint:
            self.geoPoint.latitude = float(value)
            self.dispatch('on_config_changed')
    
    def on_lon(self, instance, value):
        if self.geoPoint:
            self.geoPoint.longitude = float(value)
            self.dispatch('on_config_changed')
            
class EmptyTrackDbView(BoxLayout):
    def __init__(self, **kwargs):
        super(EmptyTrackDbView, self).__init__(**kwargs)
        
class TrackDbItemView(BoxLayout):
    track = None
    trackInfoView = None
    index = 0
    def __init__(self, **kwargs):
        super(TrackDbItemView, self).__init__(**kwargs)
        track = kwargs.get('track', None)
        self.index = kwargs.get('index', 0)
        trackInfoView = kvFind(self, 'rcid', 'trackinfo')
        trackInfoView.setTrack(track)
        self.track = track
        self.trackInfoView = trackInfoView
        self.register_event_type('on_remove_track')

    def on_remove_track(self, index):
        pass
    
    def removeTrack(self):
        self.dispatch('on_remove_track', self.index)
        
class TrackSelectionPopup(BoxLayout):
    trackBrowser = None
    def __init__(self, **kwargs):
        super(TrackSelectionPopup, self).__init__(**kwargs)
        trackManager = kwargs.get('trackManager', None)
        trackBrowser = kvFind(self, 'rcid', 'browser')
        trackBrowser.on_tracks_updated(trackManager)
        self.register_event_type('on_tracks_selected')
        self.trackBrowser = trackBrowser
        
    def on_tracks_selected(self, selectedTrackIds):
        pass
    
    def confirmAddTracks(self):
        self.dispatch('on_tracks_selected', self.trackBrowser.selectedTrackIds)        
        
            
class AutomaticTrackConfigScreen(Screen):
    trackDb = None
    tracksGrid = None
    trackManager = None
    trackItemMinHeight = 200
    searchRadiusMeters = 2000
    searchBearing = 360
    trackSelectionPopup = None
    def __init__(self, **kwargs):
        super(AutomaticTrackConfigScreen, self).__init__(**kwargs)
        self.tracksGrid = kvFind(self, 'rcid', 'tracksgrid')
        self.register_event_type('on_tracks_selected')
        self.register_event_type('on_modified')
                        
    def on_modified(self, *args):
        pass
                
    def on_config_updated(self, rcpCfg):
        self.trackDb = rcpCfg.trackDb
        self.init_tracks_list()
        
    def on_tracks_updated(self, trackManager):
        self.trackManager = trackManager
        self.init_tracks_list()
    
    def on_tracks_selected(self, instance, selectedTrackIds):
        if self.trackDb:
            failures = False
            for trackId in selectedTrackIds:
                trackMap = self.trackManager.getTrackById(trackId)
                if trackMap:
                    startFinish = trackMap.startFinishPoint
                    if startFinish and startFinish.latitude and startFinish.longitude:
                        track = Track.fromTrackMap(trackMap)
                        self.trackDb.tracks.append(track)
                    else:
                        failures = True
            if failures:
                alertPopup('Cannot Add Tracks', 'One or more tracks could not be added due to missing start/finish points.\n\nPlease check for track map updates and try again.')            
            self.init_tracks_list()
            self.trackSelectionPopup.dismiss()
            self.trackDb.stale = True
            self.dispatch('on_modified')
                    
    def on_add_track_db(self):
        trackSelectionPopup = TrackSelectionPopup(trackManager=self.trackManager)
        popup = Popup(title = 'Add Race Tracks', content = trackSelectionPopup, size_hint=(0.9, 0.9))
        trackSelectionPopup.bind(on_tracks_selected=self.on_tracks_selected)
        popup.open()
        self.trackSelectionPopup = popup
        
    
    def init_tracks_list(self):
        if self.trackManager and self.trackDb:
            matchedTracks = []
            for track in self.trackDb.tracks:
                startPoint = track.startLine
                radius = startPoint.metersToDegrees(self.searchRadiusMeters, self.searchBearing)
                matchedTrack = self.trackManager.findNearbyTrack(track.startLine, radius)
                if matchedTrack:
                    matchedTracks.append(matchedTrack)
                    
            grid = kvFind(self, 'rcid', 'tracksgrid')
            grid.clear_widgets()
            if len(matchedTracks) == 0:
                grid.add_widget(EmptyTrackDbView())
                self.tracksGrid.height = self.trackItemMinHeight
            else:
                self.tracksGrid.height = self.trackItemMinHeight * (len(matchedTracks) + 1)
                index = 0
                for track in matchedTracks:
                    trackDbView = TrackDbItemView(track=track, index=index)
                    trackDbView.bind(on_remove_track=self.on_remove_track)
                    trackDbView.size_hint_y = None
                    trackDbView.height = self.trackItemMinHeight
                    grid.add_widget(trackDbView)
                    index += 1
                
            self.disableView(False)
            
    def on_remove_track(self, instance, index):
            try:
                del self.trackDb.tracks[index]
                self.init_tracks_list()
                self.trackDb.stale = True
                self.dispatch('on_modified')
                            
            except Exception as detail:
                print('Error removing track from list ' + str(detail))
                    
    def disableView(self, disabled):
        kvFind(self, 'rcid', 'addtrack').disabled = disabled
        
class ManualTrackConfigScreen(Screen):
    trackCfg = None
    sectorViews = []
    startLineView = None
    finishLineView = None
    separateStartFinish = False

    def __init__(self, **kwargs):
        super(ManualTrackConfigScreen, self).__init__(**kwargs)
        
        sepStartFinish = kvFind(self, 'rcid', 'sepStartFinish') 
        sepStartFinish.bind(on_setting=self.on_separate_start_finish)
        sepStartFinish.setControl(SettingsSwitch())
        
        self.separateStartFinish = False        
        sectorsContainer = kvFind(self, 'rcid', 'sectorsGrid')
        self.sectorsContainer = sectorsContainer
        self.initSectorViews()
            
        sectorsContainer.height = dp(35) * CONFIG_SECTOR_COUNT
        sectorsContainer.size_hint = (1.0, None)
        
        self.register_event_type('on_modified')
                        
    def on_modified(self, *args):
        pass
    
    def on_separate_start_finish(self, instance, value):        
        if self.trackCfg:
            self.trackCfg.track.trackType = 1 if value else 0
            self.trackCfg.stale = True
            self.dispatch('on_modified')            
            self.separateStartFinish = value
            self.updateTrackViewState()
              
    def initSectorViews(self):
        
        sectorsContainer = self.sectorsContainer
        sectorsContainer.clear_widgets()
        
        self.startLineView = kvFind(self, 'rcid', 'startLine')
        self.startLineView.bind(on_config_changed=self.on_config_changed)
        self.finishLineView = kvFind(self, 'rcid', 'finishLine')
        self.finishLineView.bind(on_config_changed=self.on_config_changed)
                                
        self.updateTrackViewState()
            
    def on_config_changed(self, *args):
        self.trackCfg.stale = True
        self.dispatch('on_modified')
                    
    def updateTrackViewState(self):
        if not self.separateStartFinish:
            self.startLineView.setTitle('Start / Finish')
            self.finishLineView.setTitle('- - -')            
            self.finishLineView.disabled = True
        else:
            self.startLineView.setTitle('Start Line')
            self.finishLineView.setTitle('Finish Line')
            self.finishLineView.disabled = False
    
    def update_tabs(self):
        prevSectorView = None
        for sectorView in self.sectorViews:
            if prevSectorView:
                prevSectorView.setNext(sectorView.getPrevious())
            prevSectorView = sectorView
        
    def on_config_updated(self, rcpCfg):
        trackCfg = rcpCfg.trackConfig
        
        separateStartFinishSwitch = kvFind(self, 'rcid', 'sepStartFinish')
        self.separateStartFinish = trackCfg.track.trackType == TRACK_TYPE_STAGE
        separateStartFinishSwitch.setValue(self.separateStartFinish) 
        
        sectorsContainer = self.sectorsContainer

        sectorsContainer.clear_widgets()
        for i in range(1, trackCfg.track.sectorCount):
            sectorView = SectorPointView(title = 'Sector ' + str(i))
            sectorView.bind(on_config_changed=self.on_config_changed)
            sectorsContainer.add_widget(sectorView)
            sectorView.setPoint(trackCfg.track.sectors[i])
            self.sectorViews.append(sectorView)

        self.startLineView.setPoint(trackCfg.track.startLine)
        self.finishLineView.setPoint(trackCfg.track.finishLine)
        
        self.trackCfg = trackCfg
        self.updateTrackViewState()
        self.update_tabs()
            
class TrackConfigView(BaseConfigView):
    trackCfg = None
    trackDb = None
    
    screenManager = None
    manualTrackConfigView = None
    autoConfigView = None
    
    def __init__(self, **kwargs):
        super(TrackConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        
        self.manualTrackConfigView = ManualTrackConfigScreen(name='manual')
        self.manualTrackConfigView.bind(on_modified=self.on_modified)
        
        self.autoConfigView = AutomaticTrackConfigScreen(name='auto')
        self.autoConfigView.bind(on_modified=self.on_modified)
        
        screenMgr = kvFind(self, 'rcid', 'screenmgr')
        screenMgr.add_widget(self.manualTrackConfigView)
        self.screenManager = screenMgr
        
        autoDetect = kvFind(self, 'rcid', 'autoDetect') 
        autoDetect.bind(on_setting=self.on_auto_detect)
        autoDetect.setControl(SettingsSwitch())

    def on_tracks_updated(self, trackManager):
        self.autoConfigView.on_tracks_updated(trackManager)
        
    def on_config_updated(self, rcpCfg):
        trackCfg = rcpCfg.trackConfig
        trackDb = rcpCfg.trackDb
        
        autoDetectSwitch = kvFind(self, 'rcid', 'autoDetect')
        autoDetectSwitch.setValue(trackCfg.autoDetect)
        
        self.manualTrackConfigView.on_config_updated(rcpCfg)
        self.autoConfigView.on_config_updated(rcpCfg)
        self.trackCfg = trackCfg
        self.trackDb = trackDb
        
    def on_auto_detect(self, instance, value):
        if value:
            self.screenManager.switch_to(self.autoConfigView)
        else:
            self.screenManager.switch_to(self.manualTrackConfigView)

        if self.trackCfg:
            self.trackCfg.autoDetect = value
            self.trackCfg.stale = True
            self.dispatch('on_modified')