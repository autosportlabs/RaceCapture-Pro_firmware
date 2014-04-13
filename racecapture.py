import kivy
kivy.require('1.8.0')
from kivy.config import Config
Config.set('graphics', 'width', '1024')
Config.set('graphics', 'height', '576')
from kivy.app import App, Builder
from kivy.graphics import Color, Rectangle
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.button import Button
from kivy.uix.spinner import Spinner
from kivy.uix.gridlayout import GridLayout
from kivy.uix.scrollview import ScrollView
from kivy.uix.spinner import Spinner
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.garden.graph import Graph, MeshLinePlot
from kivy.uix.treeview import TreeView, TreeViewLabel
from kivy.extras.highlight import KivyLexer
from kivy.uix.codeinput import CodeInput
from pygments import lexers

from spacer import *
from boundedlabel import BoundedLabel
from rcpserial import *

class AnalogScaler(Graph):
    def __init__(self, **kwargs):
        super(AnalogScaler, self).__init__(**kwargs)

        plot = MeshLinePlot(color=[0, 1, 0, 1])
        plot.points = [[0,-100],[5,100]]
        self.add_plot(plot)
        

class AnalogChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogChannel, self).__init__(**kwargs)

class GPIOChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(GPIOChannel, self).__init__(**kwargs)

class PulseChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(PulseChannel, self).__init__(**kwargs)

class AnalogPulseOutputChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogPulseOutputChannel, self).__init__(**kwargs)

class OrientationSpinner(Spinner):
    def __init__(self, **kwargs):
        super(OrientationSpinner, self).__init__(**kwargs)
        self.values = ["Normal", "Inverted"]

class ChannelNameSpinner(Spinner):
    def __init__(self, **kwargs):
        super(ChannelNameSpinner, self).__init__(**kwargs)
        self.values = ["OilTemp", "Battery", "AFR", "WingAngle"]

class AccelMappingSpinner(Spinner):
    def __init__(self, **kwargs):
        super(AccelMappingSpinner, self).__init__(**kwargs)
        self.values = ['X', 'Y', 'Z']

class GyroMappingSpinner(Spinner):
    def __init__(self, **kwargs):
        super(GyroMappingSpinner, self).__init__(**kwargs)
        self.values = ['Yaw']

class SampleRateSpinner(Spinner):
    def __init__(self, **kwargs):
        super(SampleRateSpinner, self).__init__(**kwargs)
        self.values = ['Disabled', '1 Hz', '5 Hz', '10 Hz', '25 Hz', '50 Hz', '100 Hz']


class SampleRateSelectorView(BoxLayout):
    pass

class ChannelNameSelectorView(BoxLayout):
    pass

class SplashView(BoxLayout):
    pass

class GPSChannelsView(BoxLayout):
    pass

class TargetConfigView(GridLayout):
    pass

class TrackConfigView(BoxLayout):
    pass
    
class CellTelemetryView(BoxLayout):
    pass

class BluetoothTelemetryView(BoxLayout):
    pass

class LuaScriptingView(BoxLayout):
    pass

class GPIOChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(GPIOChannelsView, self).__init__(**kwargs)
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=90 * 3)
    
        # add button into that grid
        for i in range(3):
            channel = AccordionItem(title='Digital Input/Output ' + str(i + 1))
            editor = GPIOChannel()
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)
    
class AnalogPulseOutputChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogPulseOutputChannelsView, self).__init__(**kwargs)
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=90 * 3)
    
        # add button into that grid
        for i in range(3):
            channel = AccordionItem(title='Pulse / Analog Output ' + str(i + 1))
            editor = AnalogPulseOutputChannel()
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

class CANChannelsView(BoxLayout):
    pass

class OBD2ChannelsView(BoxLayout):
    pass

class AnalogChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogChannelsView, self).__init__(**kwargs)
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=80 * 8)
    
        # add button into that grid
        for i in range(8):
            channel = AccordionItem(title='Analog ' + str(i + 1))
            editor = AnalogChannel()
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)
        
class PulseChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(PulseChannelsView, self).__init__(**kwargs)
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


class AccelGyroChannelsView(BoxLayout):
    pass

class LinkedTreeViewLabel(TreeViewLabel):
    view = None
   
class LuaCodeInput(CodeInput):
    def __init__(self, **kwargs):
        super(LuaCodeInput, self).__init__(**kwargs)
        self.lexer= lexers.get_lexer_by_name('lua')

class LuaScriptingView(BoxLayout):
    #def __init__(self, **kwargs):
     #   print("init")
        
    def readScript(self):
        print("read script")

    def writeScript(self):
        print("write script")

    def runScript(self):
        print("run script")

class ToolbarView(BoxLayout):
    def __init__(self, **kwargs):
        super(ToolbarView, self).__init__(**kwargs)
        self.rcp = kwargs.get('rcp', None)

    def readConfig(self):
        print("read config")
        analog = self.rcp.getAnalogCfg(None)
        print('analog: ' + str(analog))


class ToolbarButton(Button):
    pass

class RaceCaptureApp(App):        
    def __init__(self, **kwargs):
        super(RaceCaptureApp, self).__init__(**kwargs)
        rcp = RcpSerial()
        self.rcp = rcp

    def on_select_node(self, instance, value):
        # ensure that any keyboard is released
        self.content.get_parent_window().release_keyboard()

        try:
            self.content.clear_widgets()
            self.content.add_widget(value.view)
        except Exception, e:
            print e

    def build(self):
        tree = TreeView(size_hint=(None, 1), width=200, hide_root=True, indent_level=0)

        def create_tree(text):
            return tree.add_node(LinkedTreeViewLabel(text=text, is_open=True, no_selection=True))

        def attach_node(text, n, view):
            label = LinkedTreeViewLabel(text=text)
            label.view = view
            label.color_selected =   [1.0,0,0,0.6]
            tree.add_node(label, n)

        tree.bind(selected_node=self.on_select_node)
        n = create_tree('Channels')
        attach_node('GPS', n, GPSChannelsView())
        attach_node('Track Channels', n, TrackConfigView())
        attach_node('Analog Inputs', n, AnalogChannelsView())
        attach_node('Pulse Inputs', n, PulseChannelsView())
        attach_node('Digital Input/Outputs', n, GPIOChannelsView())
        attach_node('Accelerometer / Gyro', n, AccelGyroChannelsView())
        attach_node('Pulse / Analog Outputs', n, AnalogPulseOutputChannelsView())
        n = create_tree('CAN bus')
        attach_node('CAN Channels', n, CANChannelsView())
        attach_node('OBD2 Channels', n, OBD2ChannelsView())
        n = create_tree('Telemetry')
        attach_node('Cellular Telemetry', n, CellTelemetryView())
        attach_node('Bluetooth Link', n, BluetoothTelemetryView())
        n = create_tree('Scripting / Logging')
        attach_node('Lua Script', n, LuaScriptingView())

        content = SplashView()

        main = BoxLayout(orientation = 'horizontal', size_hint=(1.0, 0.95))

        main.add_widget(tree)
        main.add_widget(content)

        toolbar = ToolbarView(size_hint=(None, 0.05), rcp=self.rcp)
        
        self.content = content
        self.tree = tree
        self.toolbar = toolbar

        outer = BoxLayout(orientation='vertical', 
                            padding=5, spacing=5, size=(1024,576), 
                            size_hint=(None, None), pos_hint={'center_x': .5, 'center_y': .5})

        outer.add_widget(toolbar)
        outer.add_widget(main)

        return outer

if __name__ == '__main__':

    RaceCaptureApp().run()
