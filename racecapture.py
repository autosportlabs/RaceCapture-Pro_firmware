import kivy
import argparse
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
from kivy.uix.treeview import TreeView, TreeViewLabel

from spacer import *
from fieldlabel import FieldLabel
from boundedlabel import BoundedLabel
from rcpserial import *
from analogchannelsview import *
from imuchannelsview import *
from gpschannelsview import *
from timerchannelsview import *
from gpiochannelsview import *
from pwmchannelsview import *
from trackconfigview import *
from obd2channelsview import *
from canchannelsview import *
from rcpconfig import *
from channels import *
from scriptview import LuaScriptingView
from samplerateview import SampleRateSelectorView
from channelnamespinner import ChannelNameSpinner
from toolbarview import ToolbarView

class SplashView(BoxLayout):
    pass
    
class CellTelemetryView(BoxLayout):
    def __init__(self, **kwargs):
        super(CellTelemetryView, self).__init__(**kwargs)
#        Builder.load_file('analogchannelsview.kv')
        self.register_event_type('on_config_updated')

    def on_config_updated(self, rcpCfg):
        pass


class BluetoothTelemetryView(BoxLayout):
    def __init__(self, **kwargs):
        super(BluetoothTelemetryView, self).__init__(**kwargs)
#        Builder.load_file('analogchannelsview.kv')
        self.register_event_type('on_config_updated')

    def on_config_updated(self, rcpCfg):
        pass


class LinkedTreeViewLabel(TreeViewLabel):
    view = None



class RaceCaptureApp(App):
    def __init__(self, **kwargs):
        self.register_event_type('on_config_updated')
        self.register_event_type('on_channels_updated')
        super(RaceCaptureApp, self).__init__(**kwargs)

        #RaceCapture serial I/O 
        self.rcp = RcpSerial()
        
        #Central configuration object
        self.rcpConfig  = RcpConfig()

        #List of Channels
        self.channels = Channels()
        
        #List of config views
        self.configViews = []
    
        self.processArgs()

    def processArgs(self):
        parser = argparse.ArgumentParser(description='Autosport Labs Race Capture App')
        parser.add_argument('-p','--port', help='Port', required=False)
        args = vars(parser.parse_args())
        self.rcp.setPort(args['port'])

    def on_read_config(self, instance, *args):
        config = self.rcp.getRcpCfg()
        self.rcpConfig.fromJson(config)
        self.dispatch('on_config_updated', self.rcpConfig)

    def on_config_updated(self, rcpConfig):
        for view in self.configViews:
            view.dispatch('on_config_updated', rcpConfig)                

    def on_channels_updated(self, channels):
        for view in self.configViews:
            channelWidgets = list(kvquery(view, __class__=ChannelNameSpinner))
            for channelWidget in channelWidgets:
                channelWidget.dispatch('on_channels_updated', channels)

    def build(self):
        def create_tree(text):
            return tree.add_node(LinkedTreeViewLabel(text=text, is_open=True, no_selection=True))

        def on_select_node(instance, value):
            # ensure that any keyboard is released
            self.content.get_parent_window().release_keyboard()

            try:
                self.content.clear_widgets()
                self.content.add_widget(value.view)
            except Exception, e:
                print e

        def attach_node(text, n, view):
            label = LinkedTreeViewLabel(text=text)
            label.view = view
            label.color_selected =   [1.0,0,0,0.6]
            tree.add_node(label, n)
            self.configViews.append(view)

        def createConfigViews(tree):
            n = create_tree('Channels')
            attach_node('GPS', n, GPSChannelsView())
            attach_node('Track Channels', n, TrackConfigView())
            attach_node('Analog Inputs', n, AnalogChannelsView(channelCount=8, channels=self.channels))
            attach_node('Pulse Inputs', n, PulseChannelsView(channelCount=3, channels=self.channels))
            attach_node('Digital Input/Outputs', n, GPIOChannelsView(channelCount=3, channels=self.channels))
            attach_node('Accelerometer / Gyro', n, ImuChannelsView())
            attach_node('Pulse / Analog Outputs', n, AnalogPulseOutputChannelsView(channelCount=4, channels=self.channels))
            n = create_tree('CAN bus')
            attach_node('CAN Channels', n, CANChannelsView())
            attach_node('OBD2 Channels', n, OBD2ChannelsView())
            n = create_tree('Telemetry')
            attach_node('Cellular Telemetry', n, CellTelemetryView())
            attach_node('Bluetooth Link', n, BluetoothTelemetryView())
            n = create_tree('Scripting / Logging')
            attach_node('Lua Script', n, LuaScriptingView())

        tree = TreeView(size_hint=(None, 1), width=200, hide_root=True, indent_level=0)
        tree.bind(selected_node=on_select_node)
        createConfigViews(tree)

        content = SplashView()

        main = BoxLayout(orientation = 'horizontal', size_hint=(1.0, 0.95))
        main.add_widget(tree)
        main.add_widget(content)

        toolbar = ToolbarView(size_hint=(None, 0.05), rcp=self.rcp, app=self)
        toolbar.bind(on_read_config=self.on_read_config)
        
        self.content = content
        self.tree = tree
        self.toolbar = toolbar

        outer = BoxLayout(orientation='vertical', size=(1024,576), 
                            size_hint=(None, None), pos_hint={'center_x': .5, 'center_y': .5})

        outer.add_widget(toolbar)
        outer.add_widget(main)

        self.dispatch('on_channels_updated', self.channels)

        return outer

if __name__ == '__main__':

    RaceCaptureApp().run()
