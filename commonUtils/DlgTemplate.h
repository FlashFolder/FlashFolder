// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2006 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
class CDlgTemplate
{

public:

	CDlgTemplate(){};
    CDlgTemplate(LPCTSTR caption, DWORD style, short x, short y, short w, short h,
        LPCTSTR font = NULL, LONG fontSize = 8)
    {

        usedBufferLength = sizeof(DLGTEMPLATE );
        totalBufferLength = usedBufferLength;

        dialogTemplate = (DLGTEMPLATE*)malloc(totalBufferLength);

        dialogTemplate->style = style;

        dialogTemplate->style |= DS_SETFONT;

        dialogTemplate->x     = x;
        dialogTemplate->y     = y;
        dialogTemplate->cx    = w;
        dialogTemplate->cy    = h;
        dialogTemplate->cdit  = 0;

        dialogTemplate->dwExtendedStyle = 0;

        //the dialog box doesn't have a menu or a special class

        AppendData(_T("\0"), 2);
        AppendData(_T("\0"), 2);

        //add the dialog's caption to the template

        AppendString(caption);

        AppendData(&fontSize, sizeof(WORD));
        AppendString(font);

    }

    void AddComponent(LPCTSTR type, LPCTSTR caption, DWORD style, DWORD exStyle,
        short x, short y, short w, short h, WORD id)
    {

        DLGITEMTEMPLATE item;

        item.style = style;
        item.x     = x;
        item.y     = y;
        item.cx    = w;
        item.cy    = h;
        item.id    = id;

        item.dwExtendedStyle = exStyle;

        AppendData(&item, sizeof(DLGITEMTEMPLATE));

        AppendString(type);
        AppendString(caption);

        WORD creationDataLength = 0;
        AppendData(&creationDataLength, sizeof(WORD));

        //increment the component count

        dialogTemplate->cdit++;

    }

    void AddButton(LPCTSTR caption, DWORD style, DWORD exStyle, short x, short y,
        short w, short h, WORD id)
    {

        AddStandardComponent(0x0080, caption, style, exStyle, x, y, w, h, id);

        WORD creationDataLength = 0;
        AppendData(&creationDataLength, sizeof(WORD));

    }

    void AddEditBox(LPCTSTR caption, DWORD style, DWORD exStyle, short x, short y,
        short w, short h, WORD id)
    {

        AddStandardComponent(0x0081, caption, style, exStyle, x, y, w, h, id);

        WORD creationDataLength = 0;
        AppendData(&creationDataLength, sizeof(WORD));

    }

    void AddStatic(LPCTSTR caption, DWORD style, DWORD exStyle, short x, short y,
        short w, short h, WORD id)
    {

        AddStandardComponent(0x0082, caption, style, exStyle, x, y, w, h, id);

        WORD creationDataLength = 0;
        AppendData(&creationDataLength, sizeof(WORD));

    }

    void AddListBox(LPCTSTR caption, DWORD style, DWORD exStyle, short x, short y,
        short w, short h, WORD id)
    {

        AddStandardComponent(0x0083, caption, style, exStyle, x, y, w, h, id);

        WORD creationDataLength = 0;
        AppendData(&creationDataLength, sizeof(WORD));

    }

    void AddScrollBar(LPCTSTR caption, DWORD style, DWORD exStyle, short x, short y,
        short w, short h, WORD id)
    {

        AddStandardComponent(0x0084, caption, style, exStyle, x, y, w, h, id);

        WORD creationDataLength = 0;
        AppendData(&creationDataLength, sizeof(WORD));

    }

    void AddComboBox(LPCTSTR caption, DWORD style, DWORD exStyle, short x, short y,
        short w, short h, WORD id)
    {

        AddStandardComponent(0x0085, caption, style, exStyle, x, y, w, h, id);

        WORD creationDataLength = 0;
        AppendData(&creationDataLength, sizeof(WORD));

    }

    /**
     * Returns a pointer to the Win32 dialog template which the object
     * represents. This pointer may become invalid if additional
     * components are added to the template.
     */

    operator const DLGTEMPLATE*() const
    {
        return dialogTemplate;
    }

    virtual ~CDlgTemplate()
    {
        free(dialogTemplate);
    }

protected:

    void AddStandardComponent(WORD type, LPCTSTR caption, DWORD style,
        DWORD exStyle, short x, short y, short w, short h, WORD id)
    {

        DLGITEMTEMPLATE item;

        // DWORD algin the beginning of the component data

        AlignData(sizeof(DWORD));

        item.style = style;
        item.x     = x;
        item.y     = y;
        item.cx    = w;
        item.cy    = h;
        item.id    = id;

        item.dwExtendedStyle = exStyle;

        AppendData(&item, sizeof(DLGITEMTEMPLATE));

        WORD preType = 0xFFFF;

        AppendData(&preType, sizeof(WORD));
        AppendData(&type, sizeof(WORD));

        AppendString(caption);

        // Increment the component count

        dialogTemplate->cdit++;

    }

    void AlignData(int size)
    {

        int paddingSize = usedBufferLength % size;

        if (paddingSize != 0)
        {
            EnsureSpace(paddingSize);
            usedBufferLength += paddingSize;
        }

    }

    void AppendString(LPCTSTR string)
    {
#ifndef _UNICODE
        int length = MultiByteToWideChar(CP_ACP, 0, string, -1, NULL, 0);
#else
		int length = (int)wcslen(string)+1;
#endif
        WCHAR* wideString = (WCHAR*)malloc(sizeof(WCHAR) * length);
#ifndef _UNICODE
        MultiByteToWideChar(CP_ACP, 0, string, -1, wideString, length);
#else
		wcscpy_s(wideString, length, string);
#endif
        AppendData(wideString, length * sizeof(WCHAR));
        free(wideString);
    }

    void AppendData(void* data, int dataLength)
    {

        EnsureSpace(dataLength);

        memcpy((char*)dialogTemplate + usedBufferLength, data, dataLength);
        usedBufferLength += dataLength;

    }

    void EnsureSpace(int length)
    {

        if (length + usedBufferLength > totalBufferLength)
        {

            totalBufferLength += length * 2;

            void* newBuffer = malloc(totalBufferLength);
            memcpy(newBuffer, dialogTemplate, usedBufferLength);

            free(dialogTemplate);
            dialogTemplate = (DLGTEMPLATE*)newBuffer;

        }

    }

private:

    DLGTEMPLATE* dialogTemplate;

    int totalBufferLength;
    int usedBufferLength;

};
