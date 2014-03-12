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
from kivy.garden.graph import Graph, MeshLinePlot
from math import sin

class AnalogScaler(BoxLayout):
    def __init__(self, **kwargs):
        # make sure we aren't overriding any important functionality
        super(AnalogScaler, self).__init__(**kwargs)
        
        graph = Graph(xlabel='X', ylabel='Y', x_ticks_minor=5, x_ticks_major=25, y_ticks_major=1,
            y_grid_label=True, x_grid_label=True, padding=5,
            x_grid=True, y_grid=True, xmin=-0, xmax=100, ymin=-1, ymax=1)
        plot = MeshLinePlot(color=[1, 0, 0, 1])
       # plot.points = [(x, sin(x / 10.)) for x in xrange(0, 101)]
        graph.add_plot(plot)
        self.add_widget(graph)


class SampleRateSpinner(Spinner):
	def __init__(self, **kwargs):
		super(SampleRateSpinner, self).__init__(**kwargs)

class ChannelSpinner(Spinner):
	def __init__(self, **kwargs):
        	# make sure we aren't overriding any important functionality
	       super(ChannelSpinner, self).__init__(**kwargs)
	
class AnalogChannelEditor(BoxLayout):
    def __init__(self, **kwargs):
        # make sure we aren't overriding any important functionality
        super(AnalogChannelEditor, self).__init__(**kwargs)

        self.add_widget(Label(text=kwargs['theTitle']))


        scaler = AnalogScaler(size=(200,100))
        channelName = ChannelSpinner(text='EngineTemp', values=('EngineTemp', 'RPM', 'EGT', 'AFR', 'Boost', 'OilPress', 'OilTemp'))
        sampleRate = SampleRateSpinner(text='Disabled', values=('Disabled', '1 Hz', '5 Hz', '10 Hz', '25 Hz', '50 Hz', '100 Hz', '200 Hz'))
    
        channelConfig = BoxLayout()
        channelConfig.add_widget(channelName)
        channelConfig.add_widget(sampleRate)
       # channelConfig.add_widget(scaler)
        self.add_widget(channelConfig)



class RootWidget(FloatLayout):

    def __init__(self, **kwargs):
        # make sure we aren't overriding any important functionality
        super(RootWidget, self).__init__(**kwargs)

	scroller = ScrollView(do_scroll_x=False, size=(500,500), size_hint=(0,0), pos_hint={'center_x': .5, 'center_y': .5})

	accordion = Accordion(orientation='vertical', size_hint=(None, None), size=(500, 50 * 30))

    
	for i in range(30):
		channel = AccordionItem(title='Analog ' + str(i + 1))
		channelEditor = AnalogChannelEditor(size=(480, 40), size_hint=(None, None), theTitle= 'Analog ' + str(i + 1))
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

