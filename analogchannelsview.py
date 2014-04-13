import kivy
kivy.require('1.0.8')

from kivy.garden.graph import Graph, MeshLinePlot
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from utils import *
from configview import *
    
class AnalogChannelsView(ConfigView):
    def __init__(self, **kwargs):
        Builder.load_file('analogchannelsview.kv')
        self.register_event_type('on_config_updated')

        self.channelCount = kwargs['channelCount']
        super(AnalogChannelsView, self).__init__(**kwargs)
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=80 * self.channelCount)
    
        # add button into that grid
        for i in range(self.channelCount):
            channel = AccordionItem(title='Analog ' + str(i + 1))
            editor = AnalogChannel(id='analog' + str(i)) 
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def update(self, rcpCfg):
        print('updating config')

        analogCfg = rcpCfg.analogConfig
        channelCount = analogCfg.channelCount
        for i in range(channelCount):
            editor = kvquery(self, id='analog' + str(i)).next()
            channel = kvquery(editor, rcpid='chan').next()
            channelId = analogCfg.channels[i].channelId
#            channel.setValue(self.channels[channelId])
        
    def on_config_updated(self, rcpCfg):
        self.update(rcpCfg)

        

class AnalogChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogChannel, self).__init__(**kwargs)

    def update(self, data):
        self.ids.channelName.text = 'fooo'
        #self.ids.config.ids.sampleRate.text = '25'

class AnalogScaler(Graph):
    def __init__(self, **kwargs):
        super(AnalogScaler, self).__init__(**kwargs)

        plot = MeshLinePlot(color=[0, 1, 0, 1])
        plot.points = [[0,-100],[5,100]]
        self.add_plot(plot)

