from kivy.app import App
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
	

class SampleRateSpinner(Spinner):
	def __init__(self, **kwargs):
		super(SampleRateSpinner, self).__init__(**kwargs)

class ChannelSpinner(Spinner):
	def __init__(self, **kwargs):
        	# make sure we aren't overriding any important functionality
	       super(ChannelSpinner, self).__init__(**kwargs)
	
class ChannelEditor(BoxLayout):
	def __init__(self, **kwargs):
		# make sure we aren't overriding any important functionality
		super(ChannelEditor, self).__init__(**kwargs)

		self.add_widget(Label(text=kwargs['theTitle']))

		channelName = ChannelSpinner(text='EngineTemp', values=('EngineTemp', 'RPM', 'EGT', 'AFR', 'Boost', 'OilPress', 'OilTemp'))
		self.add_widget(channelName)

		sampleRate = SampleRateSpinner(text='Disabled', values=('Disabled', '1 Hz', '5 Hz', '10 Hz', '25 Hz', '50 Hz', '100 Hz', '200 Hz'))	
		self.add_widget(sampleRate)


class RootWidget(FloatLayout):

    def __init__(self, **kwargs):
        # make sure we aren't overriding any important functionality
        super(RootWidget, self).__init__(**kwargs)

	scroller = ScrollView(do_scroll_x=False)

	accordion = Accordion(orientation='vertical', size_hint=(None, None), size=(500, 500), pos_hint={'center_x': .5, 'center_y': .5})

	for i in range(8):
		channel = AccordionItem(title='Analog ' + str(i + 1))
		channelEditor = ChannelEditor(size=(480, 40), size_hint=(None, None), theTitle= 'Analog ' + str(i + 1))
		channel.add_widget(channelEditor)
		accordion.add_widget(channel)

	scroller.add_widget(accordion)
	self.add_widget(scroller)


class MainApp(App):

    def build(self):
        self.root = root = RootWidget()
        root.bind(size=self._update_rect, pos=self._update_rect)
        with root.canvas.before:
            Color(0, 0, 0, 1) # green; colors range from 0-1 not 0-255
            self.rect = Rectangle( size=root.size, pos=root.pos)
        return root

    def _update_rect(self, instance, value):
        self.rect.pos = instance.pos
        self.rect.size = instance.size

if __name__ == '__main__':
    MainApp().run()

