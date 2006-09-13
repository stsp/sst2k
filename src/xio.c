#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>

XtAppContext app_context;
Widget toplevel, form, buttons, quit, destruct, text;

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
    buttons = XtVaCreateManagedWidget ("form", formWidgetClass, form, NULL); 
    quit     = XtVaCreateManagedWidget("quit", 
				       commandWidgetClass, buttons, 
				       XtNlabel, "Quit", NULL);
    destruct = XtVaCreateManagedWidget("destruct", 
				       commandWidgetClass, buttons, 
				       XtNfromHoriz, quit,
				       XtNlabel, "Destruct", NULL);
    text = XtVaCreateManagedWidget ("text", asciiTextWidgetClass, form, 
				    XtNfromVert, buttons, XtNresize,
				    XawtextResizeBoth, XtNresizable, True, NULL);
    XtAddCallback (quit, XtNcallback, quit_proc, NULL);

    /* sample text so the widget will be identifiable */
    XtVaSetValues (text, XtNtype, XawAsciiString, 
		       XtNstring, "Command window", NULL); 
    XtVaSetValues (text, XtNeditType, XawtextRead, XtNdisplayCaret, False, NULL);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app_context);
    exit(0);
}
