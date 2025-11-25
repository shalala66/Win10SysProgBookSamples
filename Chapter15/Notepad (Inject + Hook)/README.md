First, since it's a .dll file, I found out how many times "GetSysColor" was called 
via "MessageBox" - 8 times. Then, I logged the indices to see which indices Notepad 
used to call "GetSysColor" - "GetSysColor(4)". This is "COLOR_MENU" and is not related 
to the window title, but only for the menu bar colors. The "DLL" is injected and the 
"Hooker" is triggered, but Notepad only calls "GetSysColor" with this flag. The 8 "GetSysColor" 
calls are also due to the fact that it is "asked" several times to change the color.
