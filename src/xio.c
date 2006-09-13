#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include "sst.h"

static XtAppContext app_context;
static Widget toplevel, form, text, buttons; 

static String fallback[] = {
    /* text window resources */
    "*text.resizable: true",
    "*text.resize: ResizeBoth",
    NULL,
};

struct cmd_t {
    char *name;
    void (*callback)(Widget, XtPointer, XtPointer);
    int enable;
    Widget widget;
};

static void quit_proc(Widget w, XtPointer client_data, XtPointer call_data)
{ 
    XtDestroyApplicationContext (app_context);
    exit (0);
}

static void noargs_proc(Widget w, XtPointer client_data, XtPointer call_data)
/* use this for commands that take no arguments */
{
    /* currently a stub */
}

static struct cmd_t commands[] = {
    {"Phasers",		NULL,		0},
    {"Torpedo",		NULL,		0},
    {"Move",		NULL,		0},
    {"Shields",		NULL,		0},
    {"Dock",		noargs_proc,	0},
    {"Damages",		noargs_proc,	0},
    {"Chart",		noargs_proc,	0},
    {"Impulse",		NULL,		0},
    {"Rest",		NULL,		0},
    {"Warp",		NULL,		0},
    {"Score",		noargs_proc,	0},
    {"Sensors",		noargs_proc,	OPTION_PLANETS},
    {"Orbit",		noargs_proc,	OPTION_PLANETS},
    {"Transport",	noargs_proc,	OPTION_PLANETS},
    {"Mine",		noargs_proc,	OPTION_PLANETS},
    {"Crystals",	noargs_proc,	OPTION_PLANETS},
    {"Shuttle",		noargs_proc,	OPTION_PLANETS},
    {"Planets",		noargs_proc,	OPTION_PLANETS},
    {"Report",		noargs_proc,	0},
    {"Computer",	noargs_proc,   	0},
    {"Emexit",		noargs_proc,	0},
    {"Probe",		NULL,		OPTION_PROBE},
    {"Save",		NULL,		0},
    {"Abandon",		noargs_proc,	0},
    {"Destruct",	noargs_proc,	0},
    {"Deathray",	noargs_proc,	0},
    {"Mayday",		noargs_proc,	0},
    {"Quit",		quit_proc,	0},
    {"Help",		noargs_proc,	0},
};

int main(int argc, char **argv)
{ 
    struct cmd_t *cp;

    toplevel = XtVaOpenApplication(&app_context, "sst2k", NULL, 0, &argc,
				    argv, fallback, 
				    applicationShellWidgetClass,
				    XtNallowShellResize, True, NULL);
    form = XtVaCreateManagedWidget("form", formWidgetClass, toplevel, NULL);
    /* the command window */
    text = XtVaCreateManagedWidget("text", asciiTextWidgetClass, form, 
				   XtNwidth, 400, XtNheight, 200,
				   NULL);
    XtVaSetValues(text, XtNeditType,XawtextRead, XtNdisplayCaret,False, NULL);
    /* The button panel */
    buttons  = XtVaCreateManagedWidget("form", 
				       boxWidgetClass, form,
				       XtNfromVert, text, 
				       XtNorientation, XtorientHorizontal,
				       NULL); 
    for (cp = commands; cp < commands + sizeof(commands)/sizeof(commands[0]); cp++) {
	cp->widget = XtVaCreateManagedWidget(cp->name, 
					     commandWidgetClass, buttons, 
					     XtNlabel, cp->name,
					     NULL);
	if (cp->callback)
	    XtAddCallback (cp->widget, XtNcallback, cp->callback, NULL);
    }
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
    /* loop may be interrupted */
    XtDestroyApplicationContext(app_context);
    exit(0);
}
