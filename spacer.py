from kivy.uix.widget import Widget

class HorizontalSpacer(Widget):
    def __init__(self, **kwargs):
        super(HorizontalSpacer, self).__init__( **kwargs)
        self.size_hint_y = None
        self.height=0

class VerticalSpacer(Widget):
    def __init__(self, **kwargs):
        super(VerticalSpacer, self).__init__( **kwargs)
        self.size_hint_x = None
        self.width=0
