import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.screenmanager import Screen
from kivy.app import Builder
from utils import kvFind
from iconbutton import TileIconButton
from kivy.metrics import dp

Builder.load_file('autosportlabs/racecapture/menu/homepageview.kv')

class FeatureButton(TileIconButton):
    def __init__(self, **kwargs):
        super(FeatureButton, self).__init__(**kwargs)
    
class HomePageView(Screen):
    def __init__(self, **kwargs):
        super(HomePageView, self).__init__(**kwargs)
    