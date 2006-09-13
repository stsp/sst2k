#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include "sst.h"

static XtAppContext app_context;
static Widget toplevel, form, text, buttons; 

String fallback[] = {
    /* text window resources */
    "*text.resizable: true",
    "*text.resize: ResizeBoth",
    /* layout constraints */
    /* navigation row */
    //"*Move.fromHoriz:",
    "*Impulse.fromHoriz: Move",
    "*Rest.fromHoriz: Impulse",
    "*Warp.fromHoriz: Rest",
    "*Dock.fromHoriz: Warp",
    "*Chart.fromHoriz: Dock",
    // Weapons row
    "*Phasers.fromVert: Move",
    "*Torpedo.fromHoriz: Phasers",
    "*Shields.fromHoriz: Torpedo",
    "*Damages.fromHoriz: Shields",
    "*Crystals.fromHoriz: Damages",
    "*Deathray.fromHoriz: Damages",
    "*Mayday.fromHoriz: Deathray",
    "*Abandon.fromHoriz: Mayday",
    // Planet row
    "*Sensors.fromVert: Phasers",
    "*Orbit.fromHoriz: Sensors",
    "*Transport.fromHoriz: Orbit",
    "*Mine.fromHoriz: Transport",
    "*Shuttle.fromHoriz: Transport",
    "*Planets.fromHoriz: Shuttle",
    // Miscellany row
    "*Report.fromVert: Sensors",
    "*Computer.fromHoriz: Report",
    "*Probe.fromHoriz: Computer",
    "*Help.fromHoriz: Computer",
    // Ending it all
    "*Score.fromVert: Report",
    "*Destruct.fromHoriz: Score",
    "*Quit.fromHoriz: Destruct",
    "*Emexit.fromHoriz: Quit",
    "*Save.fromHoriz: Emexit",
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

static struct cmd_t commands[] = {
    {"Phasers",		NULL,		0},
    {"Torpedo",		NULL,		0},
    {"Move",		NULL,		0},
    {"Shields",		NULL,		0},
    {"Dock",		NULL,		0},
    {"Damages",		NULL,		0},
    {"Chart",		NULL,		0},
    {"Impulse",		NULL,		0},
    {"Rest",		NULL,		0},
    {"Warp",		NULL,		0},
    {"Score",		NULL,		0},
    {"Sensors",		NULL,		OPTION_PLANETS},
    {"Orbit",		NULL,		OPTION_PLANETS},
    {"Transport",	NULL,		OPTION_PLANETS},
    {"Mine",		NULL,		OPTION_PLANETS},
    {"Crystals",	NULL,		OPTION_PLANETS},
    {"Shuttle",		NULL,		OPTION_PLANETS},
    {"Planets",		NULL,		OPTION_PLANETS},
    {"Report",		NULL,		0},
    {"Computer",	NULL,      	0},
    {"Emexit",		NULL,		0},
    {"Probe",		NULL,		OPTION_PROBE},
    {"Save",		NULL,		0},
    {"Abandon",		NULL,		0},
    {"Destruct",	NULL,		0},
    {"Deathray",	NULL,		0},
    {"Mayday",		NULL,		0},
    {"Quit",		quit_proc,	0},
    {"Help",		NULL,		0},
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
				    NULL);

    /* The button panel */
    buttons  = XtVaCreateManagedWidget("form", 
					formWidgetClass, form, 
					XtNfromVert, text, 
					NULL); 
    for (cp = commands; cp < commands + sizeof(commands)/sizeof(commands[0]); cp++) {
	cp->widget = XtVaCreateManagedWidget(cp->name, 
					     commandWidgetClass, buttons, 
					     XtNlabel, cp->name,
					     NULL);
	if (cp->callback)
	    XtAddCallback (cp->widget, XtNcallback, cp->callback, NULL);
    }
    /* sample text so the widget will be identifiable */
    XtVaSetValues(text, XtNtype, XawAsciiString, 
		       XtNstring, "Command window", NULL); 
    XtVaSetValues(text, XtNeditType, XawtextRead, XtNdisplayCaret, False, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
    /* loop may be interrupted */
    XtDestroyApplicationContext(app_context);
    exit(0);
}
