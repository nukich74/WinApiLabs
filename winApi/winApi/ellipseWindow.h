#ifndef ELLIPSE_WINDOW
#define ELLIPSE_WINDOW

#include <string>
#include <Windows.h>
#include <utility>

struct Color {
	int red;
	int green;
	int blue;

	Color( int t_red, int t_green, int t_blue ) : red( t_red ), green( t_green ), blue( t_blue ) {} 
};

class EllipseWindow {
public:
	//speed, left-top coordinate, radius ...
	EllipseWindow( std::pair< int, int >, std::pair<int, int>,  int, Color bgColor, Color figColor);
	~EllipseWindow();
	static bool RegisterClass( std::wstring, HINSTANCE );
	bool Create( std::wstring, HINSTANCE ); // создать простое окно вопрос
	bool Create( std::wstring, HINSTANCE, HWND, int ); // создать окно через handle родителя
	void Show( int cmdShow );
	HWND GetHandle();
	int GetRadius();
	void SetRadius( int );

protected:
	void onDestroy();

private:
	const Color selectFigColor, selectBgColor;
	Color curFigColor, curBgColor;
	UINT_PTR timer;
	std::pair<int, int> center, speed;
	int radius;
	HWND handle;

	static LRESULT _stdcall windowProc( HWND, UINT, WPARAM, LPARAM ); 
	void drawEllipse();
	void moveEllipse();
	void invalidate();
};
#endif