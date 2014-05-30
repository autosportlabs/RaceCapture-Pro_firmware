import kivy
kivy.require('1.8.0')
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
    def __init__(self, **kwargs):
        super(LuaScriptingView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')

    def on_config_updated(self, rcpCfg):
        scriptCfg = rcpCfg.scriptConfig
        scriptView = kvFind(self, 'rcid', 'script')
        scriptView.text = scriptCfg.script
        self.scriptCfg = scriptCfg
   
    def on_script_changed(self, instance, value):
        if self.scriptCfg:
            self.scriptCfg.script = value
        
class LuaCodeInput(CodeInput):
    def __init__(self, **kwargs):
        super(LuaCodeInput, self).__init__(**kwargs)
        self.lexer= lexers.get_lexer_by_name('lua')
        
    def readScript(self):
        print("read script")

    def writeScript(self):
        print("write script")

    def runScript(self):
        print("run script")
                
