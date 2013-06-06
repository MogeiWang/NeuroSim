# This file is part of NeuroSim.
# NeuroSim is free software and comes with ABSOLUTELY NO WARANTY.
# See LICENSE for more details.

LIBS = -lpthread
BUILD = g++ -Wall -o NeuroSim main.cpp $(LIBS)
DEPS = main.cpp
DEBUG = gdb NeuroSim

main: $(DEPS)
	$(BUILD)
run: $(DEPS)
	$(BUILD)
	./NeuroSim
debug: $(DEPS)
	$(BUILD) -ggdb
	$(DEBUG)
