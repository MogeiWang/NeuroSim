# This file is part of NeuroSim.
# NeuroSim is free software and comes with ABSOLUTELY NO WARANTY.
# See LICENSE for more details.

main: main.cpp
	g++ -o NeuroSim main.cpp -lpthread
run: main.cpp
	g++ -o NeuroSim main.cpp -lpthread
	./NeuroSim
