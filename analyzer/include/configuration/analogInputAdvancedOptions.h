#ifndef ANALOGINPUTADVANCEDOPTIONS_H_
#define ANALOGINPUTADVANCEDOPTIONS_H_

#include "wx/wx.h"
#include "StripChart.h"
#include "wx/grid.h"
#include "raceCapture/raceCaptureConfig.h"

#define ADVANCED_ANALOG_INPUT_SETTINGS_TITLE "Advanced Analog Input Settings"

class AnalogScalingMapGrid : public wxGrid{

	public:
		AnalogScalingMapGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr)
			: wxGrid(parent, id, pos, size, style, name)
			{ }

		void SetScrollbar (int orientation, int position, int thumbSize, int range, bool refresh) {
		    wxGrid::SetScrollbar (orientation, 0, 0, 0);
		}
		wxGridCellCoordsArray GetSelectedCellCoords(){

			int numberRows = GetNumberRows();
			int numberCols = GetNumberCols();
			wxGridCellCoordsArray selectedCells;
			for (int row = 0; row < numberRows; row++){
				for (int col = 0; col < numberCols; col++){
					 if (IsInSelection(row,col)) selectedCells.Add(wxGridCellCoords(row,col));
				}
			}
			return selectedCells;

		}
};

class AnalogInputAdvancedDialog : public wxDialog
  {
  	DECLARE_CLASS( OptionsDialog )
  	DECLARE_EVENT_TABLE();

  	public:
  		AnalogInputAdvancedDialog();

  		AnalogInputAdvancedDialog(wxWindow* parent,
  			wxWindowID id = wxID_ANY,
  			const wxString& caption = wxT(ADVANCED_ANALOG_INPUT_SETTINGS_TITLE),
  			const wxPoint& pos = wxDefaultPosition,
  			const wxSize& size = wxDefaultSize,
  			long style = wxCAPTION | wxSYSTEM_MENU );

  		bool Create (wxWindow* parent,
  			wxWindowID id = wxID_ANY,
  			const wxString& caption = wxT(ADVANCED_ANALOG_INPUT_SETTINGS_TITLE),
  			const wxPoint& pos = wxDefaultPosition,
  			const wxSize& size = wxDefaultSize,
  			long style = wxCAPTION | wxSYSTEM_MENU );

  		void SetConfig(AnalogConfig *config);
  		AnalogConfig GetConfig();
  		void CreateControls();

  	private:
  		void RefreshControls();
  		void RefreshScalingChart();
  		AnalogScalingMapGrid * GetAnalogScalingMapGrid();
  		StripChart * GetAnalogScalingChart();
  		double GetScaledValue(int rawValue);
  		double LinearInterpolate(double x, double x1, double y1, double x2, double y2);
  		void OnScalingMapGridChanged(wxGridEvent &event);
  		void OnLinearScalingButton(wxCommandEvent &event);
  		void OnRawScalingButton(wxCommandEvent &event);
  		void OnMapScalingButton(wxCommandEvent &event);
  		void OnLinearScaleChanged(wxCommandEvent &event);
  		AnalogConfig m_config;
  		AnalogScalingMapGrid * m_analogScalingMapGrid;
  		StripChart * m_analogScalingChart;

  		int m_chartScaleId;
  		ChartScale * m_chartScale;
  };

enum{
	ANALOG_SCALING_RAW_RADIO			= wxID_HIGHEST + 1,
	ANALOG_SCALING_LINEAR_RADIO,
	ANALOG_SCALING_MAP_RADIO,
	SIMPLE_LINEAR_SCALE_TEXTBOX,
	ANALOG_SCALING_MAP_GRID,
	ANALOG_SCALING_MAP_CHART
};

#endif /* ANALOGINPUTADVANCEDOPTIONS_H_ */
