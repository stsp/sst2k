#!/usr/bin/env python
#
# Generate an on-line help file for SST 2K from the text generated from
# the XML documentation.
#
# By Eric S. Raymond for the Super Star Trek project
import os, re, sys

enddelim = "********\n"

# This is the part most likely to bit-rot
beginmarker1 = "Mnemonic:"
endmarker1 = "Miscellaneous Notes"
beginmarker2 = " ABBREV"
endmarker2 = "Game History and Modifications"

fp = open("sst-doc.txt", "r")
savetext = []
state = 0
while True:
    line = fp.readline()
    if not line:
        break
    if state == 0 and line.startswith(beginmarker1):
        line = "% " + line[12:].lstrip()
        state = 1
    if state == 0 and line.startswith(beginmarker2):
        savetext.append(enddelim + "%% ABBREV\n")
        state = 2
    if state == 1:
        if line.find(endmarker1) > -1:
            state = 0
    if state == 2:
        if line.find(endmarker2) > -1:
            state = 0
    if state:
        line = line.replace("%", "%%")
        # Hack Unicode non-breaking spaces into ordinary spaces
        line = line.replace("\xc2\xa0", " ").replace("\240", "")
        if line.startswith("Mnemonic:"):
            while not savetext[-1].strip():
                savetext.pop()
        savetext.append(line)
savetext = "".join(savetext)

# Remove the section titles
savetext = re.sub("\n+.*\n*Mnemonic:\\s*", "\n********\n%% ", savetext)

sys.stdout.write(savetext + enddelim)
