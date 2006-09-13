#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>

XtAppContext app_context;
Widget toplevel, form, text; 
Widget buttons, phasers, photons, destruct, quit;

String fallback[] = {
    /* button labels */
    "*phasers.label: Phasers",
    "*photons.label: Torps",
    "*destruct.label: Destruct",
    "*quit.label: Quit",
    /* text window resources */
    "*text.resizable: true",
    "*text.resize: ResizeBoth",
    /* layout constraints */
    "*photons.fromHoriz: phasers",
    "*destruct.fromHoriz: photons",
    "*quit.fromHoriz: destruct",
    NULL,
};

static void quit_proc (Widget w, XtPointer client_data, XtPointer call_data)
{ 
    XtDestroyApplicationContext (app_context);
    exit (0);
}

int main (int argc, char **argv)
{ 
    toplevel = XtVaOpenApplication (&app_context, "sst2k", NULL, 0, &argc,
				    argv, fallback, 
				    applicationShellWidgetClass,
				    XtNallowShellResize, True, NULL);
    form = XtVaCreateManagedWidget ("form", formWidgetClass, toplevel, NULL);
    /* The button panel */
    buttons  = XtVaCreateManagedWidget ("form", formWidgetClass, form, NULL); 
    phasers  = XtVaCreateManagedWidget("phasers", 
				       commandWidgetClass, buttons, 
				       NULL);
    photons  = XtVaCreateManagedWidget("photons", 
				       commandWidgetClass, buttons, 
				       NULL);
    destruct = XtVaCreateManagedWidget("destruct", 
				       commandWidgetClass, buttons, 
				       NULL);
    quit     = XtVaCreateManagedWidget("quit", 
				       commandWidgetClass, buttons, 
				       NULL);
    XtAddCallback (quit, XtNcallback, quit_proc, NULL);
    /* the command window */
    text = XtVaCreateManagedWidget ("text", asciiTextWidgetClass, form, 
				    XtNfromVert, buttons, 
				    NULL);

    /* sample text so the widget will be identifiable */
    XtVaSetValues (text, XtNtype, XawAsciiString, 
		       XtNstring, "Command window", NULL); 
    XtVaSetValues (text, XtNeditType, XawtextRead, XtNdisplayCaret, False, NULL);
    XtRealizeWidget (toplevel);
    XtAppMainLoop (app_context);
    /* loop may be interrupted */
    XtDestroyApplicationContext (app_context);
    exit(0);
}
