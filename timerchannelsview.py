import kivy
kivy.require('1.0.8')

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from rcpconfig import *
from utils import *

Builder.load_file('timerchannelsview.kv')

class PulseChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(PulseChannel, self).__init__(**kwargs)

class PulseChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(PulseChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=110 * 3)
    
        # add button into that grid
        for i in range(3):
            channel = AccordionItem(title='Pulse Input ' + str(i + 1))
            editor = PulseChannel()
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def on_config_updated(self, rcpCfg):
        pass
