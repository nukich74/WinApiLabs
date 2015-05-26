#pragma once
#include <string>
#include <Windows.h>

class SettingsKeeper {
public:
	SettingsKeeper() : fontColor( RGB(0, 0, 0) ), bgColor( RGB(255, 255, 255) ), 
		fontSize( 40 ), transparency( 250 ), font( NULL ), isPreview( 0 )
	{
		this->font = reinterpret_cast<HFONT>( GetStockObject( DEFAULT_GUI_FONT ) );
	}

	~SettingsKeeper()
	{
		DeleteObject( this->GetFont() );
	}

	SettingsKeeper( const SettingsKeeper& keeper )
	{
		this->bgColor = keeper.GetBgColor();
		this->fontColor = keeper.GetFontColor();
		LOGFONT logFont;
		::GetObject( keeper.GetFont(), sizeof( LOGFONT ), &logFont );
		this->font = ::CreateFontIndirect(&logFont);
		this->fontSize = keeper.GetFontSize();
		this->transparency = keeper.GetTransparency();
	}

	SettingsKeeper& operator=( const SettingsKeeper& keeper )
	{
		this->bgColor = keeper.GetBgColor();
		this->fontColor = keeper.GetFontColor();
		LOGFONT logFont;
		::GetObject( keeper.GetFont(), sizeof( LOGFONT ), &logFont );
		this->font = ::CreateFontIndirect(&logFont);
		this->fontSize = keeper.GetFontSize();
		this->transparency = keeper.GetTransparency();
		return *this;
	}

	BYTE GetTransparency() const { return this->transparency; }
	int GetFontSize() const { return this->fontSize; }
	HFONT GetFont() const { return this->font; }
	int havePreview() const { return this->isPreview; }
	COLORREF GetBgColor() const { return this->bgColor; }
	COLORREF GetFontColor() const { return this->fontColor; }
	void SetTransparency( const BYTE& value ) { this->transparency = value; }
	void SetFontSize( const int& value ) { this->fontSize = value; }
	void SetFont( const HFONT& value ) { this->font = value; }
	void SetHavePreview( const int& value ) { this->isPreview = value; }
	void SetBgColor( const COLORREF& value ) { this->bgColor = value; }
	void SetFontColor( const COLORREF& value ) { this->fontColor = value; }

private:
	COLORREF fontColor;
	COLORREF bgColor;
	LONG fontSize;
	HFONT font;
	BYTE transparency;
	int isPreview;
};

