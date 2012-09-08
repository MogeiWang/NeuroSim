main: main.cpp
	g++ -o NeuroSim main.cpp -lpthread
run: main.cpp
	g++ -o NeuroSim main.cpp -lpthread
	./NeuroSim
