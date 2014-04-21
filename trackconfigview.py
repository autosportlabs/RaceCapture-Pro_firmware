import kivy
kivy.require('1.8.0')

from kivy.uix.gridlayout import GridLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from utils import *

Builder.load_file('trackconfigview.kv')

class SectorPointView(BoxLayout):
    def __init__(self, **kwargs):
        super(SectorPointView, self).__init__(**kwargs)
    
class TrackConfigView(BoxLayout):
    def __init__(self, **kwargs):
        super(TrackConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        
        sectors = kvFind(self, 'rcid', 'sectorsGrid')
        
        for i in range(20):
            sectorView = SectorPointView()
            sectors.add_widget(sectorView)  
            
        sectors.height = 35 * 20
        sectors.size_hint = (1.0, None)
        

    def on_config_updated(self, rcpCfg):
        pass
