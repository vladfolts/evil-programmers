import P from lpeg
{
  Keywords:{
    {
      P'LOADONCALL'+P'MOVEABLE'+P'DISCARDABLE'+P'PURE'+
      P'OWNERDRAW'+P'NOT'+P'NONSHARED'+P'PRELOAD'+P'MENUBREAK'+P'MENUBARBREAK'+
      P'SEPARATOR'+P'IMPURE'+P'FIXED'+P'GRAYED'+P'CHECKED'+P'BLOCK'+P'HELP'+
      P'INACTIVE'+P'SEGALIGN'+P'NONDISCARDABLE'+P'ASCII'+P'POPUP'+P'MENUITEM'+
      P'SHIFT'+P'CONTROL'+P'NOINVERT'+P'ALT'+P'BITMAP'+P'MENU'+P'ACCELERATOR'+
      P'CURSOR'+P'ICON'+P'CAPTION'+P'VERSIONINFO'+P'VALUE'+P'DIALOGEX'+
      P'STYLE'+P'DIALOG'+P'PUSHBUTTON'+P'FONT'+P'EXSTYLE'+P'EDITTEXT'+
      P'VIRTKEY'+P'VERSION'+P'USERBUTTON'+P'STATIC'+P'STATE'+P'SHARED'+
      P'SCROLLBAR'+P'RTEXT'+P'RCDATA'+P'RADIOBUTTON'+P'PUSHBOX'+
      P'PRODUCTVERSION'+P'MESAGETABLE'+P'LTEXT'+P'LISTBOX'+P'LANGUAGE'+
      P'IEDIT'+P'HEDIT'+P'GROUPBOX'+P'FILEVERSION'+P'FILETYPE'+P'FILESUBTYPE'+
      P'FILEOS'+P'FILEFLAGSMASK'+P'FILEFLAGS'+P'EDIT'+P'DLGINIT'+P'DLGINCLUDE'+
      P'DEFPUSHBUTTON'+P'CTEXT'+P'TEXTINCLUDE'+P'COMBOBOX'+P'CLASS'+
      P'CHECKBOX'+P'CHARACTERISTICS'+P'BUTTON'+P'BEDIT'+P'AUTORADIOBUTTON'+
      P'AUTOCHECKBOX'+P'AUTO'+P'STATE'+P'HTML'+P'ANI'+P'ANICURSOR'+P'VXD'+
      P'PLUGPLAY'+P'GROUP'+P'ICON'+P'GROUP'+P'CURSOR'+P'MESSAGETABLE'+
      P'FONTDIR'+P'STRING'+P'TOOLBAR'+P'MENUEX'+P'ACCELERATORS'+P'STRINGTABLE'
      Color:15
    }
    {'BEGIN',Color:15,Open:"BEGIN/END"}
    {'END',Color:15,Close:"BEGIN/END"}
    {'#[a-z]+',Color:2}
    {P'\0',Color:{15,0}}
    {'{',Color:15,Open:"{}"}
    {'}',Color:15,Close:"{}"}
    {'[%w]+'}
  }
  Pairs:{Color:12}
  Regions:{
    {Left:'"',Right:'"',Color:14,Keywords:{{P[[\\]]+P[[\"]],Color:6}}}
    {Left:'/%*',Right:'%*/',Color:3}
    {Left:'//',Color:3}
  }
}
