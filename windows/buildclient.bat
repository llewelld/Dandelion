cd dandeclientsrc

del *.o

gcc -DHAVE_CONFIG_H -DDANDEDIR=\"./assets\" -c -Wall -mwindows -mms-bitfields -I"%GTK_BASEPATH%\include\gtk-2.0" -I"%GTK_BASEPATH%\include\cairo" -I"%GTK_BASEPATH%\include\libglade-2.0" -I"%GTK_BASEPATH%\include\glib-2.0" -I"%GTK_BASEPATH%\include\pango-1.0" -I"%GTK_BASEPATH%\lib\gtk-2.0\include" -I"%GTK_BASEPATH%\lib\glib-2.0\include" -I"%GTK_BASEPATH%\include\atk-1.0" -I"%GTK_BASEPATH%\include" -I"%GTK_BASEPATH%\include\gtkglext-1.0" -I"%GTK_BASEPATH%\lib\gtkglext-1.0\include" -I"%GTK_BASEPATH%\gnet" *.c

dlltool --output-def dandeclient.def dandeclient.o
dlltool --dllname dandeclient.exe --def dandeclient.def --output-exp dandeclient.exp

gcc -DUSE_GLADE -g -O2 -mwindows *.o dandeclient.exp -L"%GTK_BASEPATH%\lib" -Wl,-luuid -lgtkglext-win32-1.0 -lgdkglext-win32-1.0 -lglu32 -luser32 -lkernel32 -lopengl32 -lgtk-win32-2.0 -lglade-2.0 -lglib-2.0 -lgdk-win32-2.0 -lgdk_pixbuf-2.0 -limm32 -lshell32 -lole32 -latk-1.0 -lpangocairo-1.0 -lcairo -lpangoft2-1.0 -lpangowin32-1.0 -lgdi32 -lz -lpango-1.0 -lgobject-2.0 -lm -lgmodule-2.0 -lintl -L"%GTK_BASEPATH%\gnet" -lgnet-2.0 -o ..\dandeclient.exe
cd ..



rem cd dandeclientsrc
rem gcc -DHAVE_CONFIG_H -c -Wall -mwindows -mno-cygwin -mms-bitfields -I"%GTK_BASEPATH%\include\gtk-2.0" -I"%GTK_BASEPATH%\include\cairo" -I"%GTK_BASEPATH%\include\libglade-2.0" -I"%GTK_BASEPATH%\include\glib-2.0" -I"%GTK_BASEPATH%\include\pango-1.0" -I"%GTK_BASEPATH%\lib\gtk-2.0\include" -I"%GTK_BASEPATH%\lib\glib-2.0\include" -I"%GTK_BASEPATH%\include\atk-1.0" -I"%GTK_BASEPATH%\include" -I"%GTK_BASEPATH%\include\gtkglext-1.0" -I"%GTK_BASEPATH%\lib\gtkglext-1.0\include" -I"%GTK_BASEPATH%\gnet" -I"..\..\glut\include" -o dandeclient.o dandeclient.c

rem gcc -DHAVE_CONFIG_H -c -Wall -mwindows -mno-cygwin -mms-bitfields -I"%GTK_BASEPATH%\include\gtk-2.0" -I"%GTK_BASEPATH%\include\cairo" -I"%GTK_BASEPATH%\include\libglade-2.0" -I"%GTK_BASEPATH%\include\glib-2.0" -I"%GTK_BASEPATH%\include\pango-1.0" -I"%GTK_BASEPATH%\lib\gtk-2.0\include" -I"%GTK_BASEPATH%\lib\glib-2.0\include" -I"%GTK_BASEPATH%\include\atk-1.0" -I"%GTK_BASEPATH%\include" -I"%GTK_BASEPATH%\include\gtkglext-1.0" -I"%GTK_BASEPATH%\lib\gtkglext-1.0\include" -I"%GTK_BASEPATH%\gnet" -I"..\..\glut\include" -o dlclient.o dlclient.c

rem dlltool --output-def dandeclient.def dandeclient.o
rem dlltool --dllname dandeclient.exe --def dandeclient.def --output-exp dandeclient.exp

rem gcc -DUSE_GLADE -g -O2 -mwindows dandeclient.o dlclient.o dandeclient.exp -L"%GTK_BASEPATH%\lib" -Wl,-luuid -lgtkglext-win32-1.0 -lgdkglext-win32-1.0 -lglu32 -luser32 -lkernel32 -lopengl32 -lgtk-win32-2.0 -lglade-2.0 -lglib-2.0 -lgdk-win32-2.0 -lgdk_pixbuf-2.0 -limm32 -lshell32 -lole32 -latk-1.0 -lpangocairo-1.0 -lcairo -lpangoft2-1.0 -lpangowin32-1.0 -lgdi32 -lz -lpango-1.0 -lgobject-2.0 -lm -lgmodule-2.0 -L"..\..\glut\lib" -lglut32 -lintl -L"%GTK_BASEPATH%\gnet" -lgnet-2.0 -o ..\dandeclient.exe
rem cd ..

