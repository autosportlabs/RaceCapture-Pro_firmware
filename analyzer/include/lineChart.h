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

class Series{

public:
	Series(DatalogStoreRows *dataBuffer, size_t bufferCol, int rangeId, size_t offset, wxString label, wxColor color) :
		m_dataBuffer(dataBuffer),
		m_bufferCol(bufferCol),
		m_rangeId(rangeId),
		m_offset(offset),
		m_label(label),
		m_color(color)
		{}

	DatalogStoreRows * GetDataBuffer(){ return m_dataBuffer; }
	size_t GetBufferCol(){return m_bufferCol;}
	void SetBufferCol(size_t bufferCol){m_bufferCol = bufferCol;}
	int GetRangeId(){ return m_rangeId; }
	void SetRangeId(int seriesId){m_rangeId = seriesId; }
	size_t GetOffset(){ return m_offset; }
	void SetOffset(size_t offset){ m_offset = offset; }
	wxString & GetLabel(){ return m_label; }
	void SetLabel(wxString label){ m_label = label; }
	void SetColor(wxColor color){m_color = color; }
	wxColor GetColor(){return m_color;}

private:

	DatalogStoreRows *m_dataBuffer;
	size_t m_bufferCol;
	int m_rangeId;
	size_t m_offset;
	wxString m_label;
	wxColor m_color;
};

WX_DECLARE_OBJARRAY(Range*,RangeArray);
WX_DECLARE_OBJARRAY(Series*,SeriesArray);

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

		void SetViewOffsetPercent(double offset);
		double GetViewOffsetPercent();

		int GetMarkerOffset();
		void SetMarkerOffset(int offset);

		int AddRange(Range *range);
		int AddSeries(Series *series);

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

		SeriesArray			m_seriesArray;
		RangeArray			m_rangeArray;
		double				m_viewOffsetPercent;
		int					m_markerOffset;

		int					_zoomPercentage;
		int					_currentWidth;
		int					_currentHeight;
		wxBitmap 			*_memBitmap;
		wxColor				_backgroundColor;
		bool				_showScale;

		bool				_showData;
		int					_mouseX;
		int					_mouseY;

};
#endif /* LINECHART_H_ */
