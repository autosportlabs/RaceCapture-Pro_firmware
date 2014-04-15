import kivy
kivy.require('1.0.8')

from kivy.garden.graph import Graph, MeshLinePlot
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from utils import *
from configview import *
from rcpconfig import *

class AnalogChannelsView(ConfigView):
    def __init__(self, **kwargs):
        Builder.load_file('analogchannelsview.kv')
        self.register_event_type('on_config_updated')

        self.channelCount = kwargs['channelCount']
        self.channels = kwargs['channels']

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
        analogCfg = rcpCfg.analogConfig
        channelCount = analogCfg.channelCount

        for i in range(channelCount):
            editor = kvquery(self, id='analog' + str(i)).next()

            analogChannel = analogCfg.channels[i]
            channelSpinner = kvquery(editor, rcpid='chan').next()
            channelSpinner.setValue(self.channels.getNameForId(analogChannel.channelId))

            sampleRateSpinner = kvquery(editor, rcpid='sr').next()
            sampleRateSpinner.setValue(analogChannel.sampleRate)

            scalingMode = analogChannel.scalingMode

            checkRaw = kvquery(editor, rcpid='smRaw').next()
            checkLinear = kvquery(editor, rcpid='smLinear').next()
            checkMapped = kvquery(editor, rcpid='smMapped').next()
            if scalingMode == 0:
                checkRaw.active = True
                checkLinear.active = False
                checkMapped.active = False
            elif scalingMode == 1:
                checkRaw.active = False
                checkLinear.active = True
                checkMapped.active = False
            elif scalingMode == 2:
                checkRaw.active = False
                checkLinear.active = False
                checkMapped.active = True
            
            mapEditor = kvquery(editor, rcpid='mapEditor').next()
            mapEditor.update(analogChannel.scalingMap)
        
    def on_config_updated(self, rcpCfg):
        self.update(rcpCfg)

        

class AnalogChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogChannel, self).__init__(**kwargs)


class AnalogScaler(Graph):
    def __init__(self, **kwargs):
        super(AnalogScaler, self).__init__(**kwargs)


class AnalogScalingMapEditor(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogScalingMapEditor, self).__init__(**kwargs)

    def update(self, scalingMap):
        graph = kvquery(self, rcpid='scalingGraph').next()
        editor = kvquery(self, rcpid='mapEditor').next()
        
        plot = MeshLinePlot(color=[0, 1, 0, 1])
        mapSize = scalingMap.points
        points = []
        maxScaled = None
        minScaled = None
        for i in range(mapSize):
            volts = (5.0 * scalingMap.raw[i]) / 1024.0
            scaled = scalingMap.scaled[i]
            points.append([volts, scaled])
            if maxScaled == None or scaled > maxScaled:
                maxScaled = scaled
            if minScaled == None or scaled < minScaled:
                minScaled = scaled

            voltsCell = kvquery(editor, rcpid='v_' + str(i)).next()
            scaledCell = kvquery(editor, rcpid='s_' + str(i)).next()

            voltsCell.text = str(volts)
            scaledCell.text = str(scaled)
            
        plot.points = points
        graph.ymin = minScaled
        graph.ymax = maxScaled
        graph.add_plot(plot)

        

