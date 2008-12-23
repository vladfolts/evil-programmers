/*
    case.cpp
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004 Vadim Yegorov and Alex Yaroslavsky
    Copyright (C) 2008 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dt.hpp"
#include <limits.h>

#ifdef UNICODE
#define t_memchr wmemchr
#else
#define t_memchr memchr
#endif

enum
{
  CCLower,
  CCUpper,
  CCTitle,
  CCToggle,
  CCCyclic
};

TCHAR WordDiv[80];
int WordDivLen;
DWORD ProcessWholeLine=1;

BOOL FindBounds(TCHAR *Str, int Len, int Pos, int *Start, int *End);
int FindEnd(TCHAR *Str, int Len, int Pos);
int FindStart(TCHAR *Str, int Len, int Pos);
int GetNextCCType(TCHAR *Str, int StrLen, int Start, int End);
int ChangeCase(TCHAR *NewString, int Start, int End, int CCType);

// What we consider as letter
BOOL MyIsAlpha(TCHAR c)
{
  return (t_memchr(WordDiv,c,WordDivLen)==NULL ? TRUE : FALSE);
}

void InitCase(void)
{
  WordDivLen=Info.AdvControl(Info.ModuleNumber,ACTL_GETSYSWORDDIV,WordDiv);
  WordDivLen+=sizeof(" \n\r\t");
  _tcscat(WordDiv,_T(" \n\r\t"));
}

void DoCase(HANDLE aDlg)
{
  FarMenuItem MenuItems[7];
  memset(MenuItems,0,sizeof(MenuItems));
  MenuItems[0].Selected=1;
  INIT_MENU_TEXT(0,GetMsg(mLower));
  INIT_MENU_TEXT(1,GetMsg(mUpper));
  INIT_MENU_TEXT(2,GetMsg(mTitle));
  INIT_MENU_TEXT(3,GetMsg(mToggle));
  INIT_MENU_TEXT(4,GetMsg(mCyclic));
  MenuItems[5].Separator=1;
  INIT_MENU_TEXT(6,GetMsg(mProcessWholeLine));
  HKEY hKey;
  if(RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_READ,&hKey)==ERROR_SUCCESS)
  {
    DWORD Type,DataSize=sizeof(ProcessWholeLine);
    RegQueryValueEx(hKey,_T("ProcessWholeLine"),0,&Type,(BYTE *)&ProcessWholeLine,&DataSize);
    RegCloseKey(hKey);
  }
  MenuItems[6].Checked=(ProcessWholeLine?1:0);
  int BreakKeys[2]={VK_SPACE,0};
  int MenuCode;
  while (6==(MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE,GetMsg(mNameCase),NULL,NULL,BreakKeys,NULL,MenuItems,ArraySize(MenuItems))))
  {
    if (MenuItems[6].Checked)
    {
      ProcessWholeLine=FALSE;
      MenuItems[6].Checked=0;
    }
    else
    {
      ProcessWholeLine=TRUE;
      MenuItems[6].Checked=1;
    }
    MenuItems[0].Selected=0;
    MenuItems[6].Selected=1;
  }
  DWORD Disposition;
  if(RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition)==ERROR_SUCCESS)
  {
    RegSetValueEx(hKey,_T("ProcessWholeLine"),0,REG_DWORD,(BYTE *)&ProcessWholeLine,sizeof(ProcessWholeLine));
    RegCloseKey(hKey);
  }
  if(MenuCode>=0)
  {
    LONG_PTR itemID=Info.SendDlgMessage(aDlg,DM_GETFOCUS,0,0);
    FarDialogItem DialogItem;
    Info.SendDlgMessage(aDlg,DM_GETDLGITEMSHORT,itemID,(LONG_PTR)&DialogItem);
    if(DialogItem.Type==DI_EDIT)
    {
      long length=Info.SendDlgMessage(aDlg,DM_GETTEXTLENGTH,itemID,0)+1;
      TCHAR *buffer=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,length*sizeof(TCHAR));
      if(buffer)
      {
        EditorSelect es;
        es.BlockType=BTYPE_NONE;
        Info.SendDlgMessage(aDlg,DM_GETSELECTION,itemID,(LONG_PTR)&es);
        COORD Pos; Info.SendDlgMessage(aDlg,DM_GETCURSORPOS,itemID,(LONG_PTR)&Pos);
        Info.SendDlgMessage(aDlg,DM_GETTEXTPTR,itemID,(LONG_PTR)buffer);
        int Start=0,End=length-1;
        if (es.BlockType==BTYPE_NONE||es.BlockStartPos<0)
        {
          if (!ProcessWholeLine)
            FindBounds(buffer, length-1, Pos.X, &Start, &End);
        }
        else
        {
          Start=es.BlockStartPos;
          End=es.BlockStartPos+es.BlockWidth;
        }
        if(MenuCode==CCCyclic)
        {
          //Define Conversion Type
          MenuCode=GetNextCCType(buffer, length-1, Start, End);
        }
        //Buffer contains no words
        if(MenuCode!=CCCyclic)
        {
          //Do the conversion
          ChangeCase(buffer, Start, End, MenuCode);
        }
        Info.SendDlgMessage(aDlg,DM_SETTEXTPTR,itemID,(LONG_PTR)buffer);
#ifdef UNICODE
        FarDialogItem* pDialogItem;
        pDialogItem=(FarDialogItem*)(LONG_PTR)Info.SendDlgMessage(aDlg,DM_GETDLGITEM,itemID,0);
        Info.SendDlgMessage(aDlg,DN_EDITCHANGE,itemID,(LONG_PTR)pDialogItem);
        Info.SendDlgMessage(aDlg,DM_FREEDLGITEM,0,(LONG_PTR)pDialogItem);
#else
        Info.SendDlgMessage(aDlg,DM_GETDLGITEM,itemID,(LONG_PTR)&DialogItem);
        Info.SendDlgMessage(aDlg,DN_EDITCHANGE,itemID,(LONG_PTR)&DialogItem);
#endif
        Info.SendDlgMessage(aDlg,DM_SETCURSORPOS,itemID,(LONG_PTR)&Pos);
        Info.SendDlgMessage(aDlg,DM_SETSELECTION,itemID,(LONG_PTR)&es);
        HeapFree(GetProcessHeap(),0,buffer);
      }
    }
  }
}

// Finding word bounds (what'll be converted) (Str is in OEM)
BOOL FindBounds(TCHAR *Str, int Len, int Pos, int *Start, int *End)
{
  int i = 1;
  int ret = FALSE;
  int r = INT_MAX;
  // If line isn't empty
  if( Len>*Start )
  {
    *End=min(*End,Len);

    // Pos between [Start, End] ?
    Pos=max(Pos,*Start);
    Pos=min(*End,Pos);

    // If current character is non letter
    if(!MyIsAlpha(Str[Pos]))
    {
      // Looking for letter on the left and counting radius
      while((*Start<=Pos-i) && (!MyIsAlpha(Str[Pos-i])))
        i++;

      // Letter was found on the left
      if(*Start<=Pos-i)
        r=i; // Storing radius

      i=1;
      // Looking for letter on the right and counting radius
      while((Pos+i<=*End) && (!MyIsAlpha(Str[Pos+i])))
        i++;

      // Letter was not found
      if(Pos+i>*End)
        i=INT_MAX;

      // Here r is left radius and i is right radius

      // If no letters was found
      if( min(r,i)!=INT_MAX )
      {
        // What radius is less? Left?
        if( r <= i )
        {
          *End=Pos-r+1;
          *Start=FindStart(Str, *Start, *End);
        }
        else // Right!
        {
          *Start=Pos+i;
          *End=FindEnd(Str, *Start, *End);
        }
        ret=TRUE;
      }
    }
    else // Current character is letter!
    {
      *Start=FindStart(Str, *Start, Pos);
      *End=FindEnd(Str, Pos, *End);
      ret=TRUE;
    }
  }

  if(!ret)
    *Start=*End=-1;

  return ret;
}

int FindStart(TCHAR *Str, int Start, int End)
{
  // Current pos in Str
  int CurPos=End-1;
  // While current character is letter
  while( CurPos>=Start && MyIsAlpha(Str[CurPos]) )
    CurPos--; // Moving to left

  return CurPos+1;
}

int FindEnd(TCHAR *Str, int Start, int End)
{
  // Current pos in Str
  int CurPos=Start;
  // While current character is letter
  while( CurPos<End && MyIsAlpha(Str[CurPos]))
    CurPos++; // Moving to right

  return CurPos;
}

// Changes Case of NewString from position Start till End
// to CCType and returns amount of changes
int ChangeCase(TCHAR *NewString, int Start, int End, int CCType)
{
  // If previous symbol is letter, then IsPrevSymbAlpha!=0
  BOOL IsPrevSymbAlpha=FALSE;
  // Amount of changes
  int ChangeCount=0;
  // Main loop (position inside line)
  for(int i=Start; i<End; i++)
  {
    if (MyIsAlpha(NewString[i]))// && ReverseOem==NewString[i])
    {
      switch(CCType)
      {
        case CCLower:
          NewString[i]=FSF.LLower(NewString[i]);
          break;

        case CCTitle:
          if(IsPrevSymbAlpha)
            NewString[i]=FSF.LLower(NewString[i]);
          else
            NewString[i]=FSF.LUpper(NewString[i]);
          break;

        case CCUpper:
          NewString[i]=FSF.LUpper(NewString[i]);
          break;

        case CCToggle:
          if(FSF.LIsLower(NewString[i]))
            NewString[i]=FSF.LUpper(NewString[i]);
          else
            NewString[i]=FSF.LLower(NewString[i]);
          break;

      }
      // Put converted letter back to string
      IsPrevSymbAlpha=TRUE;
      ChangeCount++;
    }
    else
      IsPrevSymbAlpha=FALSE;
  }

  return ChangeCount;
}

// Return CCType by rule: lower->UPPER->Title
// If Str contains no letters, then return CCCyclic
int GetNextCCType(TCHAR *Str, int StrLen, int Start, int End)
{
  int SignalWordLen;
  // Default conversion is to lower case
  int CCType;
  TCHAR *WrappedWord;
  TCHAR *SignalWord;
  int Counter;
  int i;

  SignalWordLen=max(Start,End);
  Counter=SignalWordLen/2+1;
  SignalWordLen=min(Start,End);

  if (StrLen<SignalWordLen)
    return CCCyclic;

  // Looking for SignalWord (the 1-st word)
  if (!FindBounds(Str,StrLen,Start,&Start,&End))
    return CCCyclic;

  SignalWordLen=End-Start;

  SignalWord=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(SignalWordLen+1)*sizeof(TCHAR));

  CCType=CCLower;
  if( SignalWord != NULL )
  {
    WrappedWord=(TCHAR *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(SignalWordLen+1)*sizeof(TCHAR));

    if (WrappedWord != NULL)
    {
      _tcsncpy(SignalWord,&Str[Start],SignalWordLen);
      SignalWord[SignalWordLen]='\0';
      _tcscpy(WrappedWord,SignalWord);

      // if UPPER then Title
      FSF.LUpperBuf(WrappedWord, SignalWordLen);
      if (SignalWordLen == 1 && _tcscmp(SignalWord,WrappedWord)==0)
        CCType=CCLower;
      else
      {
        if (SignalWordLen == 1)
          CCType=CCUpper;
        else
        {
          if (_tcscmp(SignalWord,WrappedWord)==0)
            CCType=CCTitle;
          else
          {
            // if lower then UPPER
            FSF.LLowerBuf(WrappedWord, SignalWordLen);
            if(_tcscmp(SignalWord,WrappedWord)==0)
              CCType=CCUpper;
            else
            {
              // if Title then lower
              WrappedWord[0]=FSF.LUpper(WrappedWord[0]);
              if(_tcscmp(SignalWord,WrappedWord)==0)
                CCType=CCLower;
              else
              {
                // if upper case letters amount more than lower case letters
                // then tOGGLE
                FSF.LUpperBuf(WrappedWord, SignalWordLen);
                for(i=0; i<SignalWordLen && Counter; i++)
                  if(SignalWord[i]==WrappedWord[i])
                    Counter--;
                if(!Counter)
                  CCType=CCToggle;
              }
            }
          }
        }
      }
      HeapFree(GetProcessHeap(),0,WrappedWord);
    }
    HeapFree(GetProcessHeap(),0,SignalWord);
  }
  return CCType;
}