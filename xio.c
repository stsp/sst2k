#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>

XtAppContext app_context;
Widget toplevel, form, quit, text;

void quit_proc (Widget w, XtPointer client_data, XtPointer call_data)
{ 
    XtDestroyApplicationContext (app_context);
    exit (0);
}

int main (int argc, char **argv)
{ 
    toplevel = XtVaOpenApplication (&app_context, "XThird", NULL, 0, &argc,
				    argv, NULL, applicationShellWidgetClass,
				    XtNallowShellResize, True, NULL);
    form = XtVaCreateManagedWidget ("form", formWidgetClass, toplevel, NULL); 
    quit = XtVaCreateManagedWidget ("quit", commandWidgetClass, form, XtNlabel,
				    "Quit", NULL);
    text = XtVaCreateManagedWidget ("text", asciiTextWidgetClass, form, 
				    XtNfromVert, quit, XtNresize,
				    XawtextResizeBoth, XtNresizable, True, NULL);
    XtAddCallback (quit, XtNcallback, quit_proc, NULL);
    if (argc <= 1)
	XtVaSetValues (text, XtNtype, XawAsciiString, 
		       XtNstring, "Fool! You should"
		       " supply a file name!", NULL); 
    else
	XtVaSetValues (text, XtNtype, XawAsciiFile, XtNstring, argv [1], NULL);
    XtVaSetValues (text, XtNeditType, XawtextRead, XtNdisplayCaret, False, NULL);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app_context);
    exit(0);
}
