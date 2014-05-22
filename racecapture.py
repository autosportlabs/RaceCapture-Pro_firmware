#!/usr/bin/python

import kivy
import logging
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
from kivy.uix.popup import Popup

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
from canconfigview import *
from rcpconfig import *
from channels import *
from telemetryconfigview import *
from wirelessconfigview import *
from scriptview import *
from samplerateview import *
from channelnamespinner import *
from toolbarview import ToolbarView
from splashview import SplashView
from separator import *


class LinkedTreeViewLabel(TreeViewLabel):
    view = None

class RaceCaptureApp(App):
    #Central configuration object
    rcpConfig  = RcpConfig()
    
    #List of Channels
    channels = Channels()
    
    #List of config views
    configViews = []
    
    #RaceCapture serial I/O 
    rcpComms = RcpSerial()
    
    def __init__(self, **kwargs):
        self.register_event_type('on_config_updated')
        self.register_event_type('on_channels_updated')
        super(RaceCaptureApp, self).__init__(**kwargs)
    
        self.processArgs()

    def processArgs(self):
        parser = argparse.ArgumentParser(description='Autosport Labs Race Capture App')
        parser.add_argument('-p','--port', help='Port', required=False)
        args = vars(parser.parse_args())
        self.rcpComms.setPort(args['port'])

        if not self.rcpComms.port:
            self.rcpComms.autoDetect()

    def _serial_warning(self):
        popup = Popup(title='Warning',
                      content=Label(text='You have not selected a serial port'),
                      size_hint=(None, None), size=(400, 400))
        popup.open()

    def on_write_config(self, instance, *args):
        rcpConfig = self.rcpConfig
        rcpJson = rcpConfig.toJson()

        try:
            self.rcpComms.writeRcpCfg(rcpJson)
        except:
            logging.exception('')
            self._serial_warning()
        
    def on_read_config(self, instance, *args):
        try:
            if not self.channels.isLoaded():
                channelsList = self.rcpComms.getChannels()
                self.channels.fromJson(channelsList)
                self.dispatch('on_channels_updated', self.channels)
                self.notifyChannelsUpdated()
                
                
            config = self.rcpComms.getRcpCfg()
            self.rcpConfig.fromJson(config)
            self.dispatch('on_config_updated', self.rcpConfig)
        except:
            logging.exception('')
            self._serial_warning()

    def notifyChannelsUpdated(self):
        self.dispatch('on_channels_updated', self.channels)

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
            attach_node('CAN Settings', n, CANConfigView())
            attach_node('OBDII Channels', n, OBD2ChannelsView(channels=self.channels))
            n = create_tree('Connections')
            attach_node('Wireless', n, WirelessConfigView())
            attach_node('Telemetry', n, TelemetryConfigView())
            n = create_tree('Scripting / Logging')
            attach_node('Lua Script', n, LuaScriptingView())

        tree = TreeView(size_hint=(None, 1), width=200, hide_root=True, indent_level=0)
        tree.bind(selected_node=on_select_node)
        createConfigViews(tree)

        content = SplashView()

        main = BoxLayout(orientation = 'horizontal', size_hint=(1.0, 0.95))
        main.add_widget(tree)
        main.add_widget(content)

        toolbar = ToolbarView(size_hint=(None, 0.05), rcp=self.rcpComms, app=self)
        toolbar.bind(on_read_config=self.on_read_config)
        toolbar.bind(on_write_config=self.on_write_config)
        
        self.content = content
        self.tree = tree
        self.toolbar = toolbar

        outer = BoxLayout(orientation='vertical', size=(1024,576), 
                            size_hint=(None, None), pos_hint={'center_x': .5, 'center_y': .5})

        outer.add_widget(toolbar)
        outer.add_widget(main)
        return outer

if __name__ == '__main__':

    RaceCaptureApp().run()
