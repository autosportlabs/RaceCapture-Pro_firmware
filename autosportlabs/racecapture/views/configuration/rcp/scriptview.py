import kivy
kivy.require('1.8.0')
from kivy.clock import Clock
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.stacklayout import StackLayout
from kivy.uix.codeinput import CodeInput
from kivy.app import Builder
from kivy.extras.highlight import KivyLexer
#from pygments.lexers.agile import PythonLexer 
from pygments import lexers
from utils import *
from iconbutton import IconButton

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/scriptview.kv')

class LuaScriptingView(BoxLayout):
    scriptCfg = None
    logfileView = None
    logfileScrollView = None
    scriptView = None
    def __init__(self, **kwargs):
        super(LuaScriptingView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        self.register_event_type('on_run_script')
        self.register_event_type('on_poll_logfile')
        self.register_event_type('on_logfile')
        self.logfileView = kvFind(self, 'rcid', 'logfile')
        self.scriptView = kvFind(self, 'rcid', 'script')
        self.logfileScrollView = kvFind(self, 'rcid', 'logfileSv') 

    def on_config_updated(self, rcpCfg):
        scriptCfg = rcpCfg.scriptConfig
        self.scriptView.text = scriptCfg.script
        self.scriptCfg = scriptCfg
   
    def on_script_changed(self, instance, value):
        if self.scriptCfg:
            self.scriptCfg.script = value

    def on_run_script(self):
        pass
    
    def on_poll_logfile(self):
        pass
        
    def on_logfile(self, value):
        text = self.logfileView.text
        self.logfileView.text = text + value
        self.logfileScrollView.scroll_y = 0.0
        
        
    def runScript(self):
        self.dispatch('on_run_script')
        
    def poll_logfile(self, dt):
        self.dispatch('on_poll_logfile')
        
    def enableScript(self, instance, value):
        if value:
            Clock.schedule_interval(self.poll_logfile, 1)
        else:
            Clock.unschedule(self.poll_logfile)
        
        
class LuaCodeInput(CodeInput):
    def __init__(self, **kwargs):
        super(LuaCodeInput, self).__init__(**kwargs)
        self.lexer= lexers.get_lexer_by_name('lua')
        
                
