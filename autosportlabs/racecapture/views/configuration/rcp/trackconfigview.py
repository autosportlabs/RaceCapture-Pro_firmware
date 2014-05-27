import kivy
kivy.require('1.8.0')

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
        
class TrackConfigView(BoxLayout):
    trackCfg = None
    sectorViews = []
    startLineView = None
    finishLineView = None
    separateStartFinish = False
    
    def __init__(self, **kwargs):
        super(TrackConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        
        
        sepStartFinish = kvFind(self, 'rcid', 'sepStartFinish') 
        sepStartFinish.bind(on_setting=self.on_separate_start_finish)
        sepStartFinish.setControl(SettingsSwitch())

        autoDetect = kvFind(self, 'rcid', 'autoDetect') 
        autoDetect.bind(on_setting=self.on_auto_detect)
        autoDetect.setControl(SettingsSwitch())
        
        self.separateStartFinish = False        
        sectorsContainer = kvFind(self, 'rcid', 'sectorsGrid')
        self.sectorsContainer = sectorsContainer
        self.initSectorViews()
            
        sectorsContainer.height = 35 * CONFIG_SECTOR_COUNT
        sectorsContainer.size_hint = (1.0, None)
                
    def on_auto_detect(self, instance, value):
        if self.trackCfg:
            self.trackCfg.autoDetect = value
        
    def on_separate_start_finish(self, instance, value):        
        if self.trackCfg:
            self.trackCfg.trackType = 1 if value else 0
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
        
        autoDetectSwitch = kvFind(self, 'rcid', 'autoDetect')
        autoDetectSwitch.setValue(trackCfg.autoDetect)
        
        separateStartFinishSwitch = kvFind(self, 'rcid', 'sepStartFinish')
        self.separateStartFinish = trackCfg.trackType == TRACK_TYPE_STAGE
        separateStartFinishSwitch.setValue(self.separateStartFinish) 
        
        sectorsContainer = self.sectorsContainer

        sectorsContainer.clear_widgets()
        for i in range(1, trackCfg.sectorCount):
            sectorView = SectorPointView(title = 'Sector ' + str(i))
            sectorsContainer.add_widget(sectorView)
            sectorView.setPoint(trackCfg.sectors[i])
            self.sectorViews.append(sectorView)

        self.startLineView.setPoint(trackCfg.startLine)
        self.finishLineView.setPoint(trackCfg.finishLine)
        
        self.trackCfg = trackCfg
        self.updateTrackViewState()
        self.update_tabs()
        
