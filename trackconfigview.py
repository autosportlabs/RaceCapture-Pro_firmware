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

Builder.load_file('trackconfigview.kv')

class SectorPointView(BoxLayout):
    def __init__(self, **kwargs):
        super(SectorPointView, self).__init__(**kwargs)
        label = kvFind(self, 'rcid', 'title')
        label.text = kwargs.get('title', 'Sector')

    
class TrackConfigView(BoxLayout):
    def __init__(self, **kwargs):
        super(TrackConfigView, self).__init__(**kwargs)
        self.separateStartFinish = False        
        self.register_event_type('on_config_updated')
        
        sectors = kvFind(self, 'rcid', 'sectorsGrid')
        self.addSectorViews(sectors)
            
        sectors.height = 35 * 20
        sectors.size_hint = (1.0, None)
    
    def addSectorViews(self, sectors):
        startPoint = None
        endPoint = None
        if not self.separateStartFinish:
            startPoint = SectorPointView(title='Start / Finish')
        else:
            startPoint = SectorPointView(title='Start Line')
            endPoint = SectorPointView(title='Finish Line')
            
        if startPoint: 
            sectors.add_widget(startPoint)
        for i in range(1,20):
            sectorView = SectorPointView(title = 'Sector ' + str(i))
            sectors.add_widget(sectorView)
        if endPoint:
            sectors.add_widget(endPoint)

    def on_config_updated(self, rcpCfg):
        pass
