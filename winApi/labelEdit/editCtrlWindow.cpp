#pragma once
#include "editCtrlWindow.h"

EditCtrlWindow::EditCtrlWindow() { }
EditCtrlWindow::~EditCtrlWindow() { }

bool EditCtrlWindow::Create( HINSTANCE hInstance, HWND parent, int ID_EDITCHILD ) {
	handle = CreateWindowEx(
		NULL,
		L"Edit",
        NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        parent, (HMENU) ID_EDITCHILD, hInstance, this );
	return handle != NULL;
}

void EditCtrlWindow::Show( int cmdShow ) {
	ShowWindow( handle, cmdShow );
	UpdateWindow( handle );
}

HWND EditCtrlWindow::GetHandle() const{
	return handle;
}

void EditCtrlWindow::SetHandle( HWND handle ) {
	this->handle = handle;
}

