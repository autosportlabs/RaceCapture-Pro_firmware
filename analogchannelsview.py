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
from channels import *
from channelnameselectorview import ChannelNameSelectorView
from channelnamespinner import ChannelNameSpinner

Builder.load_file('analogchannelsview.kv')

class AnalogChannelsView(BoxLayout):
    editors = []    
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
            analogChannel = analogCfg.channels[i]
            editor.on_config_updated(analogChannel, self.channels)
            

class AnalogChannel(BoxLayout):
    channelConfig = None
    channels = None
    def __init__(self, **kwargs):
        super(AnalogChannel, self).__init__(**kwargs)
        kvFind(self, 'rcid', 'sr').bind(on_sample_rate = self.on_sample_rate)
        kvFind(self, 'rcid', 'chanId').bind(on_channel = self.on_channel)

    def on_channel(self, instance, value):
        if self.channelConfig:
            self.channelConfig.channelId = self.channels.getIdForName(value)

    def on_sample_rate(self, instance, value):
        if self.channelConfig:
            self.channelConfig.sampleRate = value
        
    def on_scaling_type_raw(self, instance, value):
        if self.channelConfig and value:
            self.channelConfig.scalingMode = ANALOG_SCALING_MODE_RAW
            
    def on_scaling_type_linear(self, instance, value):
        if self.channelConfig and value:
            self.channelConfig.scalingMode = ANALOG_SCALING_MODE_LINEAR
            
    def on_scaling_type_map(self, instance, value):
        if self.channelConfig and value:
            self.channelConfig.scalingMode = ANALOG_SCALING_MODE_MAP
        
    def on_config_updated(self, channelConfig, channels):
        channelSpinner = kvFind(self, 'rcid', 'chanId')
        channelSpinner.setValue(channels.getNameForId(channelConfig.channelId))

        sampleRateSpinner = kvFind(self, 'rcid', 'sr')
        sampleRateSpinner.setValue(channelConfig.sampleRate)

        scalingMode = channelConfig.scalingMode

        checkRaw = kvFind(self, 'rcid','smRaw')
        checkLinear = kvFind(self, 'rcid', 'smLinear')
        checkMapped = kvFind(self, 'rcid', 'smMapped')
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
        
        mapEditor = kvFind(self, 'rcid', 'mapEditor')
        mapEditor.on_config_changed(channelConfig.scalingMap)
        
        self.channelConfig = channelConfig
        self.channels = channels
        
        
class AnalogScaler(Graph):
    def __init__(self, **kwargs):
        super(AnalogScaler, self).__init__(**kwargs)


class AnalogScalingMapEditor(BoxLayout):
    mapSize = 5
    scalingMap = None
    def __init__(self, **kwargs):
        super(AnalogScalingMapEditor, self).__init__(**kwargs)

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

    def on_config_changed(self, scalingMap):
        graph = kvFind(self, 'rcid', 'scalingGraph')
        editor = kvFind(self, 'rcid', 'mapEditor')

        mapSize = self.mapSize
        self.setTabStops(mapSize)

        plot = MeshLinePlot(color=[0, 1, 0, 1])
        points = []
        maxScaled = None
        minScaled = None
        for i in range(mapSize):
            volts = scalingMap.getVolts(i)
            scaled = scalingMap.getScaled(i)
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
        self.scalingMap = scalingMap

    def on_volts(self, mapBin, instance, value):
        if self.scalingMap:
            self.scalingMap.setVolts(mapBin, value)
        
    def on_scaled(self, mapBin, instance, value):
        if self.scalingMap:
            self.scalingMap.setScaled(mapBin, value)
        

