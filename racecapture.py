#!/usr/bin/python
import kivy
import logging
import sys
import argparse
from autosportlabs.racecapture.views.util.alertview import alertPopup
#sfrom kivy.core.window import Window
from functools import partial
from kivy.clock import Clock
kivy.require('1.8.0')
from kivy.config import Config
Config.set('graphics', 'width', '1024')
Config.set('graphics', 'height', '576')
Config.set('kivy', 'exit_on_escape', 0)

from kivy.core.window import Window

from kivy.app import App, Builder
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.uix.screenmanager import *

from installfix_garden_navigationdrawer import NavigationDrawer

from rcpserial import *
from utils import *
from autosportlabs.racecapture.tracks.trackmanager import TrackManager
from autosportlabs.racecapture.views.tracks.tracksview import TracksView
from autosportlabs.racecapture.views.configuration.rcp.configview import ConfigView
from autosportlabs.racecapture.menu.mainmenu import MainMenu
from autosportlabs.racecapture.menu.homepageview import HomePageView

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
    
    #RaceCapture serial I/O 
    rcpComms = RcpSerial()
    
    #Track database manager
    trackManager = None

    #Application Status bars    
    statusBar = None
    
    #Main Views
    configView = None
    
    #main navigation menu 
    mainNav = None
    
    #Main Screen Manager
    screenMgr = None
    
    #main view references for dispatching notifications
    mainViews = None
    
    def __init__(self, **kwargs):
        super(RaceCaptureApp, self).__init__(**kwargs)
        #self._keyboard = Window.request_keyboard(self._keyboard_closed, self)
        #self._keyboard.bind(on_key_down=self._on_keyboard_down)    
        
        Window.bind(on_key_down=self._on_keyboard_down)
        self.register_event_type('on_tracks_updated')
        self.processArgs()
        self.appConfig.setUserDir(self.user_data_dir)
        self.trackManager = TrackManager(user_dir=self.user_data_dir)
        self.initData()

    def _on_keyboard_down(self, keyboard, keycode, *args):
        if keycode == 27:
            self.switchMainView('home')
            
        
    def processArgs(self):
        parser = argparse.ArgumentParser(description='Autosport Labs Race Capture App')
        parser.add_argument('-p','--port', help='Port', required=False)
        args = vars(parser.parse_args())
        self.rcpComms.setPort(args['port'])


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
        self.rcpConfig.stale = False
        Clock.schedule_once(lambda dt: self.configView.dispatch('on_config_written'))
        
    def on_write_config_error(self, detail):
        alertPopup('Error Writing', 'Could not write configuration:\n\n' + str(detail))

    
    #Read Configuration        
    def on_read_config(self, instance, *args):
        try:
            self.rcpComms.getRcpCfg(self.rcpConfig, self.on_read_config_complete, self.on_read_config_error)
        except:
            logging.exception('')
            self._serial_warning()

    def on_read_config_complete(self, rcpCfg):
        Clock.schedule_once(lambda dt: self.configView.dispatch('on_config_updated', self.rcpConfig))
        
    def on_read_config_error(self, detail):
        alertPopup('Error Reading', 'Could not read configuration:\n\n' + str(detail))


    def on_tracks_updated(self, trackManager):
        for view in self.mainViews.itervalues():
            view.dispatch('on_tracks_updated', trackManager)
            
    def notifyTracksUpdated(self):
        self.dispatch('on_tracks_updated', self.trackManager)

    def on_main_menu_item(self, instance, value):
        self.switchMainView(value)
        
    def on_main_menu(self, instance, *args):
        self.mainNav.toggle_state()

    def showMainView(self, viewKey):
        try:
            self.screenMgr.current = viewKey
        except Exception as detail:
            print('Failed to load main view ' + str(viewKey) + ' ' + str(detail))
        
    def switchMainView(self, viewKey):
            self.mainNav.anim_to_state('closed')
            Clock.schedule_once(lambda dt: self.showMainView(viewKey), 0.25)
            
        
    def build(self):
        Builder.load_file('racecapture.kv')
        statusBar = kvFind(self.root, 'rcid', 'statusbar')
        statusBar.bind(on_main_menu=self.on_main_menu)
        
        mainMenu = kvFind(self.root, 'rcid', 'mainMenu')
        mainMenu.bind(on_main_menu_item=self.on_main_menu_item)

        self.mainNav = kvFind(self.root, 'rcid', 'mainNav')
        
        #reveal_below_anim
        #reveal_below_simple
        #slide_above_anim
        #slide_above_simple
        #fade_in
        self.mainNav.anim_type = 'slide_above_anim'
        
        configView = ConfigView(name='config', rcpConfig=self.rcpConfig)
        configView.bind(on_read_config=self.on_read_config)
        configView.bind(on_write_config=self.on_write_config)
        configView.bind(on_run_script=self.on_run_script)
        configView.bind(on_poll_logfile=self.on_poll_logfile)
        
        self.rcpComms.addListener('logfile', lambda value: Clock.schedule_once(lambda dt: configView.on_logfile(value)))
        self.rcpComms.on_progress = lambda value: statusBar.dispatch('on_progress', value)
        self.rcpComms.on_rx = lambda value: statusBar.dispatch('on_rc_rx', value)
        self.rcpComms.on_tx = lambda value: statusBar.dispatch('on_rc_tx', value)
                
        tracksView = TracksView(name='tracks')
        
        homepageView = HomePageView(name='home')
        homepageView.bind(on_select_view = lambda instance, viewKey: self.switchMainView(viewKey))
        
        screenMgr = kvFind(self.root, 'rcid', 'main')
        
        #NoTransition
        #SlideTransition
        #SwapTransition
        #FadeTransition
        #WipeTransition
        #FallOutTransition
        #RiseInTransition
        screenMgr.transition=NoTransition()
        
        screenMgr.add_widget(homepageView)
        screenMgr.add_widget(configView)
        screenMgr.add_widget(tracksView)
        #screenMgr.add_widget(dashView)
        #screenMgr.add_widget(analysisView)
        
        self.mainViews = {'config' : configView, 
                          'tracks': tracksView}
        
        self.screenMgr = screenMgr

        self.configView = configView
        self.statusBar = statusBar
        self.icon = ('resource/race_capture_icon_large.ico' if sys.platform == 'win32' else 'resource/race_capture_icon.png')
        
        if self.rcpComms.port:
            self.initRcpComms()
        else:
            self.rcpComms.autoDetect(self.rcpDetectWin, self.rcpDetectFail)

    def initRcpComms(self):
        self.rcpComms.initSerial()
        Clock.schedule_once(lambda dt: self.on_read_config(self), 1.0)
        
    def rcpDetectWin(self):
        self.initRcpComms()

        
    def rcpDetectFail(self):
        alertPopup('Could not detect', 'Could not detect RaceCapture/Pro\n\nPlease ensure it is plugged in and appropriate drivers are installed')
if __name__ == '__main__':

    RaceCaptureApp().run()
