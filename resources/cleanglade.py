#!/usr/bin/env python

import os, sys, re

if len (sys.argv) < 2:
	print "cleanglade.py removes instances of the 'swapped' attribute from glade XML files"
	print "Syntax: cleanglade.py <Gladefile>"
else:
	input = open (sys.argv[1]);
	output = open ("temp.dat", 'w');
	for s in input:
		output.write (re.sub (' swapped=".*"', '', s))

	input.close ()
	output.close ()
	os.rename	("temp.dat", sys.argv[1])

