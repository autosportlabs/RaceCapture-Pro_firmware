import kivy
kivy.require('1.8.0')
from kivy.uix.popup import Popup
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.metrics import dp
from kivy.app import Builder
from kivy.properties import StringProperty

__all__ = ('alertPopup, confirmPopup')

Builder.load_string('''
<ConfirmPopup>:
    cols:1
    Label:
        text: root.text
    GridLayout:
        cols: 2
        size_hint_y: None
        height: '44sp'
        spacing: '5sp'
        Button:
            text: 'Yes'
            on_release: root.dispatch('on_answer', True)
        Button:
            text: 'No'
            on_release: root.dispatch('on_answer', False)
''')
 
def alertPopup(title, msg):
    popup = Popup(title = title,
                      content=Label(text = msg),
                      size_hint=(None, None), size=(dp(600), dp(200)))
    popup.open()    
 
def confirmPopup(title, msg, answerCallback):
    content = ConfirmPopup(text=msg)
    content.bind(on_answer=answerCallback)
    popup = Popup(title=title,
                    content=content,
                    size_hint=(None, None),
                    size=(dp(600),dp(200)),
                    auto_dismiss= False)
    popup.open()
    return popup
    
    
class ConfirmPopup(GridLayout):
    text = StringProperty()
    
    def __init__(self,**kwargs):
        self.register_event_type('on_answer')
        super(ConfirmPopup,self).__init__(**kwargs)
        
    def on_answer(self, *args):
        pass    