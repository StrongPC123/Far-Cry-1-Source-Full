

#define TVN_CURRTIMECHANGE 1


// CTrackViewTimeBar

class CTrackViewTimeBar : public CWnd
{
	DECLARE_DYNAMIC(CTrackViewTimeBar)
public:
	CTrackViewTimeBar();
	virtual ~CTrackViewTimeBar();

	BOOL Create( DWORD dwStyle,const RECT &rect,CWnd *pParentWnd,UINT nID );

	void SetTimeScale( float timeScale );
	float GetTimeScale() { return m_timeScale; }

	void SetTimeRange( float start,float end );
	
	void SetCurrTime( float currTime );
	float GetCurrTime() const { return m_currTime; };
	
	int TimeToItem( float time );
	
	float TimeFromPoint( CPoint point );

protected:
	DECLARE_MESSAGE_MAP()

	void ChangeTime( float time );
public:
	afx_msg void OnPaint();
	CBrush m_bkgBrush;

	Range m_timeRange;
	float m_timeScale;
	float m_ticksStep;
	float m_currTime;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	bool m_draggingTime;
};