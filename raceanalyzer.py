from kivy.app import App, Builder
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

class AnalogChannel(BoxLayout):
    def __init__(self, **kwargs):
        # make sure we aren't overriding any important functionality
        super(AnalogChannel, self).__init__(**kwargs)



#    	for child in self.children[2].children:
 #   		print(child)

  #      graph = self.children[0].children[0]
        plot = MeshLinePlot(color=[1, 0, 0, 1])
        plot.points = [[0,0],[5,10]]
     #   graph.add_plot(plot)

    
class RaceAnalyzerApp(App):

    def build(self):

	accordion = Accordion(orientation='vertical', size_hint=(None, None), size=(500, 60 * 8))

        # add button into that grid
        for i in range(8):
		channel = AccordionItem(title='Analog ' + str(i + 1))
		editor = AnalogChannel()
		channel.add_widget(editor)
		accordion.add_widget(channel)
		

        # create a scroll view, with a size < size of the grid
        root = ScrollView(size_hint=(None, None), size=(500, 320), pos_hint={'center_x': .5, 'center_y': .5}, do_scroll_x=False)
        root.add_widget(accordion)

        return root

if __name__ == '__main__':

    RaceAnalyzerApp().run()
