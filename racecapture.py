#!/usr/bin/python

import kivy
import logging
import sys
import argparse
from autosportlabs.racecapture.views.util.alertview import alertPopup
from functools import partial
from kivy.clock import Clock
kivy.require('1.8.0')
from kivy.config import Config
Config.set('graphics', 'width', '1024')
Config.set('graphics', 'height', '576')

from kivy.app import App, Builder
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.popup import Popup

from installfix_garden_navigationdrawer import NavigationDrawer

from rcpserial import *
from channels import *
from utils import *
from autosportlabs.racecapture.tracks.trackmanager import TrackManager
from autosportlabs.racecapture.views.channels.channelsview import ChannelsView
from autosportlabs.racecapture.views.tracks.tracksview import TracksView
from autosportlabs.racecapture.views.configuration.rcp.configview import ConfigView
from autosportlabs.racecapture.menu.mainmenu import MainMenu

from toolbarview import ToolbarView

class AppConfig():
    userDir = "."
    
    def setUserDir(self, userDir):
        print('using user storage directory: ' + userDir)
        self.userDir = userDir
        pass
    
class RaceCaptureApp(App):
    
    appConfig = AppConfig()
    
    #Central RCP configuration object
    rcpConfig  = RcpConfig()
    
    #List of Channels
    channels = Channels()
    
    #RaceCapture serial I/O 
    rcpComms = RcpSerial()
    
    #Track database manager
    trackManager = None

    #Application Status bars    
    statusBar = None
    
    #Main Views
    configView = None
    channelsView = None
    
    #main navigation menu 
    mainNav = None
    
    #main content view
    mainView = None
        
    #collection of main views to be swapped into mainView 
    mainViews = {}
    
    def __init__(self, **kwargs):
        self.register_event_type('on_channels_updated')
        self.register_event_type('on_tracks_updated')
        self.register_event_type('on_read_channels')
        super(RaceCaptureApp, self).__init__(**kwargs)
        self.processArgs()
        self.rcpComms.initSerial()
        self.appConfig.setUserDir(self.user_data_dir)
        self.trackManager = TrackManager(user_dir=self.user_data_dir)
        self.initData()

        
    def processArgs(self):
        parser = argparse.ArgumentParser(description='Autosport Labs Race Capture App')
        parser.add_argument('-p','--port', help='Port', required=False)
        args = vars(parser.parse_args())
        self.rcpComms.setPort(args['port'])

        if not self.rcpComms.port:
            self.rcpComms.autoDetect()


    def loadCurrentTracksSuccess(self):
        print('Curent Tracks Loaded')
        Clock.schedule_once(lambda dt: self.notifyTracksUpdated())        
        
    def loadCurrentTracksError(self, details):
        alertPopup('Error Loading Tracks', str(details))        
        
    def initData(self):
        self.trackManager.init(None, self.loadCurrentTracksSuccess, self.loadCurrentTracksError)

    def _serial_warning(self):
        alertPopup('Warning', 'Command failed. Ensure you have selected a correct serial port')
   
    #Logfile
    def on_poll_logfile(self, instance):
        self.rcpComms.getLogfile()
        
    #Run Script
    def on_run_script(self, instance):
        self.rcpComms.runScript(self.on_run_script_complete, self.on_run_script_error)

    def on_run_script_complete(self, result):
        print('run script complete: ' + str(result))
            
    def on_run_script_error(self, detail):
        alertPopup('Error Running', 'Error Running Script:\n\n' + str(detail))
    
    
    
    
    
    
    
    
    
        
    #Write Configuration        
    def on_write_config(self, instance, *args):
        rcpConfig = self.rcpConfig
        try:
            self.rcpComms.writeRcpCfg(rcpConfig, self.on_write_config_complete, self.on_write_config_error)
        except:
            logging.exception('')
            self._serial_warning()
            
    def on_write_config_complete(self, result):
        print('Write config complete: ' + str(result))
        
    def on_write_config_error(self, detail):
        alertPopup('Error Writing', 'Could not write configuration:\n\n' + str(detail))

    
    
    
    #Read Configuration        
    def on_read_config(self, instance, *args):
        try:
            if not self.channels.isLoaded():
                self.on_read_channels()
            self.rcpComms.getRcpCfg(self.on_read_config_complete, self.on_read_channels_error)
        except:
            logging.exception('')
            self._serial_warning()

    def on_read_config_complete(self, rcpConfigJson):
        self.rcpConfig.fromJson(rcpConfigJson)
        Clock.schedule_once(lambda dt: self.notifyReadComplete())
        
    def on_read_config_error(self, detail):
        alertPopup('Error Reading', 'Could not read configuration:\n\n' + str(detail))

    def notifyReadComplete(self):
        self.configView.dispatch('on_config_updated', self.rcpConfig)





    #Read Channels                
    def on_read_channels(self, *args):
        self.rcpComms.getChannelList(self.on_read_channels_complete, self.on_read_channels_error)
        
    def on_read_channels_complete(self, channelsList):
        self.channels.fromJson(channelsList)
        Clock.schedule_once(lambda dt: self.notifyChannelsUpdated())
    
    def on_read_channels_error(self, detail):
        alertPopup('Error Reading', 'Error reading channels:\n\n' + str(detail))
        
    def on_channels_updated(self, channels):
        for view in self.mainViews.itervalues():
            view.dispatch('on_channels_updated', channels)
            
    def on_tracks_updated(self, trackManager):
        for view in self.mainViews.itervalues():
            view.dispatch('on_tracks_updated', trackManager)
            
            
    #Write Channels
    def on_write_channels(self, *args):
        self.rcpComms.setChannelList(self.channels.toJson(), self.on_write_channels_complete, self.on_write_channels_error)

    def on_write_channels_complete(self, response):
        print('write channels complete')
        
    def on_write_channels_error(self, detail):
        alertPopup('Error Writing', 'Error writing channels:\n\n' + str(detail))
    
    def notifyChannelsUpdated(self):
        self.dispatch('on_channels_updated', self.channels)

    def notifyTracksUpdated(self):
        self.dispatch('on_tracks_updated', self.trackManager)


    def on_main_menu_item(self, instance, value):
        self.mainNav.toggle_state()
        self.switchMainView(value)
        
    def on_main_menu(self, instance, *args):
        self.mainNav.toggle_state()

    def switchMainView(self, viewKey):
        mainView = self.mainViews.get(viewKey)
        if mainView:
            self.mainView.clear_widgets()
            self.mainView.add_widget(mainView)

        
    def build(self):
        Builder.load_file('racecapture.kv')
        statusBar = kvFind(self.root, 'rcid', 'statusbar')
        statusBar.bind(on_main_menu=self.on_main_menu)
        
        mainMenu = kvFind(self.root, 'rcid', 'mainMenu')
        mainMenu.bind(on_main_menu_item=self.on_main_menu_item)

        self.mainView = kvFind(self.root, 'rcid', 'main')
        
        self.mainNav = kvFind(self.root, 'rcid', 'mainNav')
        
        #reveal_below_anim
        #reveal_below_simple
        #slide_above_anim
        #slide_above_simple
        #fade_in
        self.mainNav.anim_type = 'slide_above_anim'
        
        configView = ConfigView(channels=self.channels, rcpConfig=self.rcpConfig)
        configView.bind(on_read_config=self.on_read_config)
        configView.bind(on_write_config=self.on_write_config)
        configView.bind(on_run_script=self.on_run_script)
        configView.bind(on_poll_logfile=self.on_poll_logfile)
        
        self.rcpComms.addListener('logfile', lambda value: Clock.schedule_once(lambda dt: configView.on_logfile(value)))
        self.rcpComms.on_progress = lambda value: statusBar.dispatch('on_progress', value)
        self.rcpComms.on_rx = lambda value: statusBar.dispatch('on_rc_rx', value)
        self.rcpComms.on_tx = lambda value: statusBar.dispatch('on_rc_tx', value)
        
        channelsView = ChannelsView(channels=self.channels, rcpComms = self.rcpComms)
        channelsView.bind(on_read_channels=self.on_read_channels)
        channelsView.bind(on_write_channels=self.on_write_channels)
        
        tracksView = TracksView()
        
        self.mainViews = {'config' : configView, 
                          'channels' : channelsView,
                          'tracks': tracksView}

        self.configView = configView
        self.channelsView = channelsView
        self.statusBar = statusBar
        self.icon = ('resource/race_capture_icon_large.ico' if sys.platform == 'win32' else 'resource/race_capture_icon.png')
        
if __name__ == '__main__':

    RaceCaptureApp().run()
