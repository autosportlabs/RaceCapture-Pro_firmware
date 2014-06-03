import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from utils import *
from kivy.metrics import dp
from kivy.uix.treeview import TreeView, TreeViewLabel
from kivy.properties import ObjectProperty
from kivy.uix.popup import Popup
import os

from autosportlabs.racecapture.views.configuration.rcp.analogchannelsview import *
from autosportlabs.racecapture.views.configuration.rcp.imuchannelsview import *
from autosportlabs.racecapture.views.configuration.rcp.gpschannelsview import *
from autosportlabs.racecapture.views.configuration.rcp.timerchannelsview import *
from autosportlabs.racecapture.views.configuration.rcp.gpiochannelsview import *
from autosportlabs.racecapture.views.configuration.rcp.pwmchannelsview import *
from autosportlabs.racecapture.views.configuration.rcp.trackconfigview import *
from autosportlabs.racecapture.views.configuration.rcp.obd2channelsview import *
from autosportlabs.racecapture.views.configuration.rcp.canconfigview import *
from autosportlabs.racecapture.views.configuration.rcp.telemetryconfigview import *
from autosportlabs.racecapture.views.configuration.rcp.wirelessconfigview import *
from autosportlabs.racecapture.views.configuration.rcp.scriptview import *
from autosportlabs.racecapture.views.file.loaddialogview import LoadDialog
from autosportlabs.racecapture.views.file.savedialogview import SaveDialog
from autosportlabs.racecapture.views.util.alertview import alertPopup

from rcpconfig import *
from channels import *

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/configview.kv')

class LinkedTreeViewLabel(TreeViewLabel):
    view = None

class ConfigView(BoxLayout):
    #file save/load
    loadfile = ObjectProperty(None)
    savefile = ObjectProperty(None)
    text_input = ObjectProperty(None)
        
    #List of config views
    configViews = []
    content = None
    menu = None
    channels = None
    rcpConfig = None
    def __init__(self, **kwargs):
        self.channels = kwargs.get('channels', None)
        self.rcpConfig = kwargs.get('rcpConfig', None)

        super(ConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        self.register_event_type('on_channels_updated')
        self.content = kvFind(self, 'rcid', 'content')
        self.menu = kvFind(self, 'rcid', 'menu')
        self.createConfigViews(self.menu)
        self.register_event_type('on_read_config')
        self.register_event_type('on_write_config')
        
        
    def on_channels_updated(self, channels):
        for view in self.configViews:
            channelWidgets = list(kvquery(view, __class__=ChannelNameSpinner))
            for channelWidget in channelWidgets:
                channelWidget.dispatch('on_channels_updated', channels)
        
    def on_config_updated(self, config):
        for view in self.configViews:
            view.dispatch('on_config_updated', config)
        
    def createConfigViews(self, tree):
        
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
        
        tree.bind(selected_node=on_select_node)
        
    def on_read_config(self, instance, *args):
        pass
    
    def on_write_config(self, instance, *args):
        pass
    
    def readConfig(self):
        self.dispatch('on_read_config', None)

    def writeConfig(self):
        if self.rcpConfig.loaded:
            self.dispatch('on_write_config', None)
        else:
            alertPopup('Warning', 'Please load or read a configuration before writing')

    def openConfig(self):
        content = LoadDialog(load=self.load, cancel=self.dismiss_popup)
        self._popup = Popup(title="Load file", content=content, size_hint=(0.9, 0.9))
        self._popup.open()        
    
    def saveConfig(self):
        if self.rcpConfig.loaded:
            content = SaveDialog(save=self.save, cancel=self.dismiss_popup)
            self._popup = Popup(title="Save file", content=content, size_hint=(0.9, 0.9))
            self._popup.open()
        else:
            alertPopup('Warning', 'Please load or read a configuration before saving')
        
    def load(self, path, filename):
        self.dismiss_popup()
        try:
            with open(os.path.join(path, filename[0])) as stream:
                rcpConfigJsonString = stream.read()
                self.rcpConfig.fromJsonString(rcpConfigJsonString)
                self.dispatch('on_config_updated', self.rcpConfig)                
        except Exception as detail:
            alertPopup('Error Loading', 'Failed to Load Configuration:\n\n' + str(detail))
    def save(self, path, filename):
        self.dismiss_popup()
        try:        
            with open(os.path.join(path, filename), 'w') as stream:
                configJson = self.rcpConfig.toJsonString()
                stream.write(configJson)
        except Exception as detail:
            alertPopup('Error Saving', 'Failed to save:\n\n' + str(detail))

    def dismiss_popup(self):
        self._popup.dismiss()
                