import kivy
kivy.require('1.8.0')

from kivy.metrics import dp
from kivy.uix.screenmanager import Screen, ScreenManager
from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from helplabel import HelpLabel
from fieldlabel import FieldLabel
from settingsview import *
from utils import *
from rcpconfig import *
from valuefield import FloatValueField
from autosportlabs.racecapture.views.tracks.tracksview import TrackInfoView
from autosportlabs.racecapture.views.configuration.rcp.baseconfigview import BaseConfigView

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/trackconfigview.kv')

class SectorPointView(BoxLayout):
    geoPoint = None
    latView = None
    lonView = None
    def __init__(self, **kwargs):
        super(SectorPointView, self).__init__(**kwargs)
        title = kwargs.get('title', None)
        if title:
            self.setTitle(title)

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
    
    def on_lon(self, instance, value):
        if self.geoPoint:
            self.geoPoint.longitude = float(value)
        
        
class AutomaticTrackConfigScreen(Screen):
    trackDb = None
    tracksGrid = None
    trackManager = None
    def __init__(self, **kwargs):
        super(AutomaticTrackConfigScreen, self).__init__(**kwargs)
        self.tracksGrid = kvFind(self, 'rcid', 'tracksgrid')
        
    def on_config_updated(self, rcpCfg):
        self.trackDb = rcpCfg.trackDb
        
    def on_tracks_updated(self, trackManager):
        self.trackManager = trackManager
        
    def init_tracks_list(self):
        if self.trackManager and self.trackDb:
            print('init tracks list nao!')
            
        
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
                        
    def on_separate_start_finish(self, instance, value):        
        if self.trackCfg:
            self.trackCfg.track.trackType = 1 if value else 0
        self.separateStartFinish = value
        self.updateTrackViewState()
              
    def initSectorViews(self):
        
        sectorsContainer = self.sectorsContainer
        sectorsContainer.clear_widgets()
        
        self.startLineView = kvFind(self, 'rcid', 'startLine')
        self.finishLineView = kvFind(self, 'rcid', 'finishLine')
                                
        self.updateTrackViewState()
            
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

    screenManager = None
    manualTrackConfigView = None
    autoConfigView = None
    
    def __init__(self, **kwargs):
        super(TrackConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        
        self.manualTrackConfigView = ManualTrackConfigScreen(name='manual')
        self.autoConfigView = AutomaticTrackConfigScreen(name='auto')
        
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
        
        autoDetectSwitch = kvFind(self, 'rcid', 'autoDetect')
        autoDetectSwitch.setValue(trackCfg.autoDetect)
        
        self.manualTrackConfigView.on_config_updated(rcpCfg)
        self.trackCfg = trackCfg
        
    def on_auto_detect(self, instance, value):
        if value:
            self.screenManager.switch_to(self.autoConfigView)
        else:
            self.screenManager.switch_to(self.manualTrackConfigView)

        if self.trackCfg:
            self.trackCfg.autoDetect = value
        