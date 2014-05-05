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

Builder.load_file('trackconfigview.kv')

class SectorPointView(BoxLayout):
    def __init__(self, **kwargs):
        super(SectorPointView, self).__init__(**kwargs)

    def setTitle(self, title):
        kvFind(self, 'rcid', 'title').text = title
        
    def setPoint(self, geoPoint):
        latView = kvFind(self, 'rcid', 'lat')
        lonView = kvFind(self, 'rcid', 'lon')
        latView.text = str(geoPoint.latitude)
        lonView.text = str(geoPoint.longitude)
        
    
class TrackConfigView(BoxLayout):
    def __init__(self, **kwargs):
        super(TrackConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')

        self.separateStartFinish = False        
        sectorsContainer = kvFind(self, 'rcid', 'sectorsGrid')
        self.sectorsContainer = sectorsContainer
        
        startLineView, finishLineView, sectorViews = self.initSectorViews(self.separateStartFinish)
        
        self.startLineView  = startLineView
        self.finishLineView = finishLineView
        self.sectorViews = sectorViews
            
        sectorsContainer.height = 35 * CONFIG_SECTOR_COUNT
        sectorsContainer.size_hint = (1.0, None)
        
    
    def initSectorViews(self, separateStartFinish):
        
        sectorsContainer = self.sectorsContainer
        sectorsContainer.clear_widgets()
        
        sectorViews = []
        startLineView = None
        finishLineView = None
        
        if not separateStartFinish:
            startLineView = SectorPointView(title='Start / Finish')
        else:
            startLineView = SectorPointView(title='Start Line')
            finishLineView = SectorPointView(title='Finish Line')
            
        if startLineView: 
            sectorsContainer.add_widget(startLineView)
        for i in range(1, CONFIG_SECTOR_COUNT):
            sectorView = SectorPointView(title = 'Sector ' + str(i))
            sectorsContainer.add_widget(sectorView)
            sectorViews.append(sectorView)
        if finishLineView:
            sectorsContainer.add_widget(finishLineView)
            
        return startLineView, finishLineView, sectorViews

    def on_config_updated(self, rcpCfg):
        trackCfg = rcpCfg.trackConfig
        
        autoDetectSwitch = kvFind(self, 'rcid', 'autoDetect')
        autoDetectSwitch.setValue(trackCfg.autoDetect)
        
        separateStartFinishSwitch = kvFind(self, 'rcid', 'trackSepStartFinish')
        separateStartFinish = trackCfg.trackType == 1
        separateStartFinishSwitch.setValue(separateStartFinish) 

        if separateStartFinish != self.separateStartFinish:
            self.initSectorViews(separateStartFinish)
            self.separateStartFinish = separateStartFinish
        
        if (separateStartFinish):
            self.finishLineView.setPoint(trackCfg.finishLine)
        
        self.startLineView.setPoint(trackCfg.startLine)
        
        for i in range(0, CONFIG_SECTOR_COUNT - 1):
            sectorView = self.sectorViews[i]
            sectorView.setPoint(trackCfg.sectors[i])
        
        
