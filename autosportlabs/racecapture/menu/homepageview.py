import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.screenmanager import Screen
from kivy.app import Builder
from utils import kvFind

Builder.load_file('homepageview.kv')

class HomePageView(Screen):
    def __init__(self, **kwargs):
        super(HomePageView, self).__init__(**kwargs)
    