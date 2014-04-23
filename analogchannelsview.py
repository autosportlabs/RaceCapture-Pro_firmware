import kivy
kivy.require('1.8.0')

from kivy.garden.graph import Graph, MeshLinePlot
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from valuefield import *
from utils import *
from rcpconfig import *
from channelnameselectorview import ChannelNameSelectorView

Builder.load_file('analogchannelsview.kv')

class AnalogChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')

        self.channelCount = kwargs['channelCount']
        self.channels = kwargs['channels']

        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=80 * self.channelCount)

        editors = []    
        for i in range(self.channelCount):
            channel = AccordionItem(title='Analog ' + str(i + 1))
            editor = AnalogChannel(id='analog' + str(i))
            channel.add_widget(editor)
            accordion.add_widget(channel)
            editors.append(editor)
            
        self.editors = editors
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def on_config_updated(self, rcpCfg):
        analogCfg = rcpCfg.analogConfig
        channelCount = analogCfg.channelCount

        for i in range(channelCount):
            editor = self.editors[i]
            
            c = editor.children[2].children

            analogChannel = analogCfg.channels[i]
            channelSpinner = kvFind(editor, 'rcid', 'chan')
            channelSpinner.setValue(self.channels.getNameForId(analogChannel.channelId))

            sampleRateSpinner = kvFind(editor, 'rcid', 'sr')
            sampleRateSpinner.setValue(analogChannel.sampleRate)

            scalingMode = analogChannel.scalingMode

            checkRaw = kvFind(editor, 'rcid','smRaw')
            checkLinear = kvFind(editor, 'rcid', 'smLinear')
            checkMapped = kvFind(editor, 'rcid', 'smMapped')
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
            
            mapEditor = kvFind(editor, 'rcid', 'mapEditor')
            mapEditor.update(analogChannel.scalingMap)


        

class AnalogChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogChannel, self).__init__(**kwargs)


class AnalogScaler(Graph):
    def __init__(self, **kwargs):
        super(AnalogScaler, self).__init__(**kwargs)


class AnalogScalingMapEditor(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogScalingMapEditor, self).__init__(**kwargs)
        self.mapSize = 5
        lastField = None    

    def setTabStops(self, mapSize):
        voltsCellFirst = kvFind(self, 'rcid', 'v_0')
        voltsCellNext = None
        for i in range(mapSize):
            voltsCell = kvFind(self, 'rcid', 'v_' + str(i))
            scaledCell = kvFind(self, 'rcid', 's_' + str(i))
            voltsCell.set_next(scaledCell)
            if (i < mapSize - 1):
                voltsCellNext = kvFind(self, 'rcid', 'v_' + str(i + 1))
            else:
                voltsCellNext = voltsCellFirst
            scaledCell.set_next(voltsCellNext)

    def update(self, scalingMap):
        graph = kvFind(self, 'rcid', 'scalingGraph')
        editor = kvFind(self, 'rcid', 'mapEditor')

        mapSize = self.mapSize
        self.setTabStops(mapSize)

        plot = MeshLinePlot(color=[0, 1, 0, 1])
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

            voltsCell = kvFind(editor, 'rcid', 'v_' + str(i))
            scaledCell = kvFind(editor, 'rcid', 's_' + str(i))

            voltsCell.text = '{:.3g}'.format(volts)
            scaledCell.text = '{:.3g}'.format(scaled)
            
        plot.points = points
        graph.ymin = minScaled
        graph.ymax = maxScaled
        graph.add_plot(plot)

        

