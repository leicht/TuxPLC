/***************************************************************************
 *   Copyright (C) 2007 by TuxPLC					                                 *
 *   Author Stephane JEANNE s.jeanne@tuxplc.net                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//#define INCLUDE_SRC
#include <windows.h>
#include <stdio.h>
#include "TuxEip.h"
//#include "TuxEipDll.h"

EXPORT void HelloWorld ();
EXPORT void About();


EXPORT void HelloWorld ()
{
    MessageBox (0, "Hello World from DLL!\n", "Hi", MB_ICONINFORMATION);
}
EXPORT int _stdcall mul(int a)
{
  return(2*a);
}
EXPORT void _stdcall mulshow(int a)
{ char txt[50];
#if defined(_MSC_VER) || defined(__BORLANDC__)
	_snprintf(txt,sizeof(txt),"result is %d\n",2*a);
#else
	snprintf(txt,sizeof(txt),"result is %d\n",2*a);
#endif
    MessageBox (0, txt, "Hi", MB_ICONINFORMATION);
}

EXPORT void About()
{
    MessageBox (0, "TuxEip Dll by Tuxplc!\n", "", MB_ICONINFORMATION);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
						About();
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
