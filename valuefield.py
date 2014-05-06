import re
from kivy.uix.textinput import TextInput

class ValueField(TextInput):

    def __init__(self, *args, **kwargs):
        self.next = kwargs.pop('next', None)
        self.multiline = False
        super(ValueField, self).__init__(*args, **kwargs)

    def set_next(self, next):
        self.next = next

    def _keyboard_on_key_down(self, window, keycode, text, modifiers):
        key, key_str = keycode
        if key in (9, 13) and self.next is not None:
            self.next.focus = True
            self.next.select_all()
        else:
            super(ValueField, self)._keyboard_on_key_down(window, keycode, text, modifiers)

class IntegerValueField(ValueField):
    pat = re.compile('[^0-9]')
    def insert_text(self, substring, from_undo=False):
        if '-' == substring and not '-' in self.text:
            return super(IntegerValueField, self).insert_text(substring, from_undo=from_undo)
        pat = self.pat
        s = re.sub(pat, '', substring)
        super(IntegerValueField, self).insert_text(s, from_undo=from_undo)
    
    
class FloatValueField(ValueField):
    pat = re.compile('[^0-9]')
    def insert_text(self, substring, from_undo=False):
        if '-' == substring and not '-' in self.text:
            return super(FloatValueField, self).insert_text(substring, from_undo=from_undo)
        pat = self.pat
        if '.' in self.text:
            s = re.sub(pat, '', substring)
        else:
            s = '.'.join([re.sub(pat, '', s) for s in substring.split('.', 1)])
        super(FloatValueField, self).insert_text(s, from_undo=from_undo)
            
