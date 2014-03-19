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
from kivy.uix.treeview import TreeView, TreeViewLabel


class AnalogScaler(Graph):
    def __init__(self, **kwargs):
        # make sure we aren't overriding any important functionality
        super(AnalogScaler, self).__init__(**kwargs)

        plot = MeshLinePlot(color=[0, 1, 0, 1])
        plot.points = [[0,-100],[5,100]]
        self.add_plot(plot)
        

class AnalogChannel(BoxLayout):
    def __init__(self, **kwargs):
        # make sure we aren't overriding any important functionality
        super(AnalogChannel, self).__init__(**kwargs)

class SampleRateSelectorView(BoxLayout):
    pass

class SplashView(BoxLayout):
    pass

class GPSChannelsView(BoxLayout):
    pass
    
class GPIOChannelsView(BoxLayout):
    pass

class AccelGyroChannelsView(BoxLayout):
    pass

class AnalogPulseOutputChannelsView(BoxLayout):
    pass

class AnalogChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogChannelsView, self).__init__(**kwargs)
        print('init!')
        accordion = Accordion(orientation='vertical', size_hint=(None, None), size=(500, 70 * 8))
    
        # add button into that grid
        for i in range(8):
            channel = AccordionItem(title='Analog ' + str(i + 1))
            editor = AnalogChannel()
            channel.add_widget(editor)
            accordion.add_widget(channel)
		
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), pos_hint={'center_x': .5, 'center_y': .5}, do_scroll_x=False)
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)
        
class PulseChannelsView(BoxLayout):
    pass

class LinkedTreeViewLabel(TreeViewLabel):
    view = None
    
class RaceAnalyzerApp(App):

    def on_select_node(self, instance, value):
        # ensure that any keyboard is released
        self.content.get_parent_window().release_keyboard()

        try:
            self.content.clear_widgets()
            self.content.add_widget(value.view)
        except Exception, e:
            print e


    def build(self):

        tree = TreeView(size_hint=(None, 1), width=200, hide_root=True, indent_level=0)

        def create_tree(text):
            return tree.add_node(LinkedTreeViewLabel(text=text, is_open=True, no_selection=True))

        def attach_node(text, n, view):
            label = LinkedTreeViewLabel(text=text)
            label.view = view
            tree.add_node(label, n)

        tree.bind(selected_node=self.on_select_node)
        n = create_tree('Channels')
        attach_node('GPS', n, GPSChannelsView())
        attach_node('Analog Inputs', n, AnalogChannelsView())
        attach_node('Pulse Inputs', n, PulseChannelsView())
        attach_node('Digital Input/Outputs', n, GPIOChannelsView())
        attach_node('Accelerometer / Gyro', n, AccelGyroChannelsView())
        attach_node('Analog / Pulse Outputs', n, AnalogPulseOutputChannelsView())
        n = create_tree('Telemetry')
        attach_node('Cellular Telemetry', n, PulseChannelsView())
        attach_node('Bluetooth Link', n, PulseChannelsView())
        n = create_tree('Scripting / Logging')
        attach_node('Lua Script', n, PulseChannelsView())


        content = SplashView()

        root = BoxLayout(orientation='horizontal', padding=20, spacing=20)
        root.add_widget(tree)
        root.add_widget(content)

        self.content = content
        self.tree = tree        
        return root

if __name__ == '__main__':

    RaceAnalyzerApp().run()
