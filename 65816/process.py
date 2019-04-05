# ************************************************************************************************************************
#
#		   	Hacks the cpuops.cpp from snes9x.c reducing it to a slow-only 65816 emulator without 6502 emulation.
#			getset.h in include is replaced with routines that just read and write a byte and word.
#			These routines are implemented in the test.c file.
#
# ************************************************************************************************************************
#
#											Read in 65816 code.
#
src = [x.rstrip() for x in open("cpuops.cpp.original").readlines()]
#
#											Find insert point.
#
n = 0
while src[n].find("cpumacro.h") < 0:
	n += 1
#
#											insert insert.txt
#
for l in [x.strip() for x in open("insert.txt").readlines()]:
	n += 1
	src.insert(n,l)
#
#											Write out result txt
#	
h = open("cpuops.cpp","w")
isOn = True
for l in src:
	#
	#		Replace this so we can dump the settings structure
	#
	l = l.replace("Settings.SA1","(-1)")
	isComment = False
	#
	#		Check for FilLRAM on IRQ/NMI, this patches that so it does nothing.
	#
	l = l.replace("Memory.FillRAM[0x22","IRQJunk[0x")
	#
	#		Dump all the fast code, just use slow definitions
	#
	if l.startswith("mOP16") or l.startswith("mOP8"):
		isComment = True
	if l.startswith("rOP16") or l.startswith("rOP8"):
		isComment = True
	if l.startswith("wOP16") or l.startswith("wOP8"):
		isComment = True
	if l.startswith("bOP") and l.find("Slow") < 0:
		isComment = True	
	#
	#		Remove any line accessing timings.
	#
	if l.find("Timings") >= 0:
		isComment = True
	#
	#		Between these two, remove the data lines that begin with tab, so we don't get errors
	#		in this tables.
	#
	if l.strip().startswith("struct SOpcodes S9xOpcodesM1X1[256] ="):
		isOn = False
	if l.strip().startswith("struct SOpcodes S9xOpcodesSlow[256] ="):
		isOn = True
	if isOn or (not l.startswith("\t")):
		if not isComment:
			h.write(l+"\n")
