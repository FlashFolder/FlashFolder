/*
Copyright (c) 2005 zett42 (zett42@users.sourceforge.net)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software, to deal in the Software without restriction,
including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the source code.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

//---------------------------------------------------------------------------------------

class CSizeGrip : public CScrollBar
{
	DECLARE_DYNAMIC(CSizeGrip)

public:
	CSizeGrip() : m_isVisible(true), m_bInternalShow(false) {}
	virtual ~CSizeGrip() {}

	// use WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS for dwStyle
	BOOL Create( CWnd* pParent, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, UINT nID = -1 );
	
	// call in rare circumstances when the visibility state doesn't update
	// automatically
	void UpdateVisible();

protected:
	afx_msg UINT OnNcHitTest( CPoint pt );
	afx_msg void OnMove( int x, int y );
	afx_msg void OnWindowPosChanging( WINDOWPOS* pw );
	DECLARE_MESSAGE_MAP()

private:
	bool ShouldBeVisible();

	bool m_isVisible;
	bool m_bInternalShow;
};