/*
 * lineChart.h
 *
 *  Created on: May 24, 2009
 *      Author: brent
 */

#ifndef LINECHART_H_
#define LINECHART_H_
#include "wx/wxprec.h"
#include "datalogData.h"
#include <wx/dynarray.h>


class Range{

public:

	Range(double min, double max, wxString label) :
		m_min(min),
		m_max(max),
		m_label(label)
	{ }

	double GetMin(){ return m_min; }
	double GetMax(){ return m_max; }
	wxString & GetLabel(){ return m_label; }

private:
	double m_min;
	double m_max;
	wxString m_label;
};

#include <wx/dynarray.h>

WX_DEFINE_ARRAY_DOUBLE(double, SeriesValues);

class Series{

public:

	Series(size_t bufferSize, int rangeId, size_t offset, wxString label, wxColor color, int precision);

	const static double NULL_VALUE = -0xFFFFFFFFFFFFFFF;
	const static size_t DEFAULT_LOOK_DISTANCE = 100;
	SeriesValues * GetSeriesValues();
	size_t GetBufferSize();
	int GetRangeId();
	void SetRangeId(int seriesId);
	size_t GetOffset();
	void SetOffset(size_t offset);
	wxString & GetLabel();
	void SetLabel(wxString label);
	void SetColor(wxColor color);
	wxColor GetColor();
	int GetPrecision();
	void SetPrecision(int precision);

	void SetBufferSize(size_t size);
	double GetValueAtOrNear(size_t index);
	double GetValueAt(size_t index);
	void SetValueAt(size_t index, double value);

private:

	int m_rangeId;
	size_t m_offset;
	wxString m_label;
	wxColor m_color;
	int m_precision;
	SeriesValues m_seriesValues;
};

WX_DECLARE_OBJARRAY(Range*,RangeArray);
WX_DECLARE_STRING_HASH_MAP(Series*,SeriesMap);

class LineChart : public wxWindow
{
	public:
		LineChart();
		LineChart(wxWindow *parent,
			wxWindowID id = -1,
			const wxPoint &pos = wxDefaultPosition,
			const wxSize &size = wxDefaultSize);

		~LineChart();

		static const int TIMESPAN_FROM_NOW = 0;
		static const int TIMESPAN_FROM_LAST_LOG_ENTRY = 1;

		void SetChartHistorySize(unsigned int chartHistorySize);

		void OnEraseBackground(wxEraseEvent& event);

		void SetLogBufferSize(int size);
		int GetLogBufferSize();

		void SetZoom(int zoomPercentage);
		int GetZoom();

		void SetTimespanMode(int mode);
		int GetTimespanMode();

		bool GetShowScale();
		void ShowScale(bool showScale);

		void SetViewOffsetFactor(double offset);
		double GetViewOffsetPercent();

		size_t GetMarkerIndex();
		void SetMarkerIndex(size_t index);

		int AddRange(Range *range);
		void ClearAllSeries();
		void AddSeries(wxString channel, Series *series);
		Series * GetSeries(wxString channel);

		DECLARE_EVENT_TABLE()

	protected:
		void OnPaint( wxPaintEvent &event );
		void OnSize( wxSizeEvent &event );
		void OnMouseEnter(wxMouseEvent &event);
		void OnMouseExit(wxMouseEvent &event);
		void OnMouseMove(wxMouseEvent &event);

	private:

		void DrawGrid(wxMemoryDC &dc);
		void DrawScale(wxMemoryDC &dc);
		void DrawCurrentValues(wxMemoryDC &dc);

		SeriesMap			m_seriesMap;
		RangeArray			m_rangeArray;
		double				m_viewOffsetFactor;
		size_t				m_markerIndex;

		int					_zoomPercentage;
		int					_currentWidth;
		int					_currentHeight;
		wxBitmap 			*_memBitmap;
		wxColor				_backgroundColor;
		bool				_showScale;

		bool				m_showData;
		int					m_mouseX;
		int					m_mouseY;

};
#endif /* LINECHART_H_ */
