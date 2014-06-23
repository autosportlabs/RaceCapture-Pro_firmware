import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from utils import *
from copy import *
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
from autosportlabs.racecapture.views.util.alertview import alertPopup, confirmPopup

from rcpconfig import *
from channels import *

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/configview.kv')

RCP_CONFIG_FILE_EXTENSION = '.rcp'

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
    scriptView = None
    writeStale = False
    def __init__(self, **kwargs):
        self.channels = kwargs.get('channels', None)
        self.rcpConfig = kwargs.get('rcpConfig', None)

        super(ConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        self.register_event_type('on_channels_updated')
        self.register_event_type('on_config_written')
        self.register_event_type('on_tracks_updated')
        self.register_event_type('on_config_modified')
        self.content = kvFind(self, 'rcid', 'content')
        self.menu = kvFind(self, 'rcid', 'menu')
        self.createConfigViews(self.menu)
        self.register_event_type('on_read_config')
        self.register_event_type('on_write_config')
        self.register_event_type('on_run_script')
        self.register_event_type('on_poll_logfile')
        

    def on_config_written(self, *args):
        self.writeStale = False
        self.updateControls()
        
    def on_config_modified(self, *args):
        self.writeStale = True
        self.updateControls()
        
    def updateControls(self):
        kvFind(self, 'rcid', 'writeconfig').disabled = not self.writeStale
        
            
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
            view.bind(on_config_modified=self.on_config_modified)
        
        #n = create_tree('Track')
        attach_node('Race Track / Sectors', None, TrackConfigView())
        n = create_tree('Channels')
        attach_node('GPS', n, GPSChannelsView())
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
        scriptView = LuaScriptingView()
        scriptView.bind(on_run_script=self.runScript)
        scriptView.bind(on_poll_logfile=self.pollLogfile)
        attach_node('Lua Script', n, scriptView)
        self.scriptView = scriptView
        
        tree.bind(selected_node=on_select_node)
        
    def on_channels_updated(self, channels):
        for view in self.configViews:
            channelWidgets = list(kvquery(view, __class__=ChannelNameSpinner))
            for channelWidget in channelWidgets:
                channelWidget.dispatch('on_channels_updated', channels)
        
    def on_config_updated(self, config):
        for view in self.configViews:
            view.dispatch('on_config_updated', config)
        self.writeStale = False
        self.updateControls()
        
    def on_tracks_updated(self, trackManager):
        for view in self.configViews:
            view.dispatch('on_tracks_updated', trackManager)
        pass
    
    def on_read_config(self, instance, *args):
        pass
    
    def on_write_config(self, instance, *args):
        pass
    
    def on_run_script(self):
        pass
        
    def on_logfile(self, logfileJson):
        if self.scriptView:
            logfileText = logfileJson.get('logfile').replace('\r','')
            self.scriptView.dispatch('on_logfile', logfileText)
        
    def runScript(self, instance):
        self.dispatch('on_run_script')

    def on_poll_logfile(self):
        pass

    def pollLogfile(self, instance):
        self.dispatch('on_poll_logfile')
                
    def readConfig(self):
        if self.writeStale:
            popup = None 
            def _on_answer(instance, answer):
                if answer:
                    self.dispatch('on_read_config', None)
                popup.dismiss()
            popup = confirmPopup('Confirm', 'Configuration Modified  - Continue Loading?', _on_answer)
        else:
            self.dispatch('on_read_config', None)

    def writeConfig(self):
        if self.rcpConfig.loaded:
            self.dispatch('on_write_config', None)
        else:
            alertPopup('Warning', 'Please load or read a configuration before writing')

    def openConfig(self):
        if self.writeStale:
            popup = None 
            def _on_answer(instance, answer):
                if answer:
                    self.doOpenConfig()
                popup.dismiss()
            popup = confirmPopup('Confirm', 'Configuration Modified  - Open Configuration?', _on_answer)
        else:
            self.doOpenConfig()
        
    def doOpenConfig(self):
        content = LoadDialog(ok=self.load, cancel=self.dismiss_popup, filters=['*' + RCP_CONFIG_FILE_EXTENSION])
        self._popup = Popup(title="Load file", content=content, size_hint=(0.9, 0.9))
        self._popup.open()        
    
    def saveConfig(self):
        if self.rcpConfig.loaded:
            content = SaveDialog(ok=self.save, cancel=self.dismiss_popup,filters=['*' + RCP_CONFIG_FILE_EXTENSION])
            self._popup = Popup(title="Save file", content=content, size_hint=(0.9, 0.9))
            self._popup.open()
        else:
            alertPopup('Warning', 'Please load or read a configuration before saving')
        
    def load(self, instance):
        self.dismiss_popup()
        try:
            selection = instance.selection
            filename = selection[0] if len(selection) else None
            if filename:
                with open(filename) as stream:
                    rcpConfigJsonString = stream.read()
                    self.rcpConfig.fromJsonString(rcpConfigJsonString)
                    self.dispatch('on_config_updated', self.rcpConfig)
            else:
                alertPopup('Error Loading', 'No config file selected')
        except Exception as detail:
            alertPopup('Error Loading', 'Failed to Load Configuration:\n\n' + str(detail))
            
    def save(self, instance):
        self.dismiss_popup()
        try:        
            filename = instance.filename
            if len(filename):
                filename = os.path.join(instance.path, filename)
                if not filename.endswith(RCP_CONFIG_FILE_EXTENSION): filename += RCP_CONFIG_FILE_EXTENSION
                with open(filename, 'w') as stream:
                    configJson = self.rcpConfig.toJsonString()
                    stream.write(configJson)
        except Exception as detail:
            alertPopup('Error Saving', 'Failed to save:\n\n' + str(detail))

    def dismiss_popup(self, *args):
        self._popup.dismiss()
                