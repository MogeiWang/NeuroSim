/*
* Copyright (c) 2012-13 Luke Montalvo <pikingqwerty@gmail.com>
*
* This file is part of NeuroSim.
* NeuroSim is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#include "brain.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    Brain br((argv[1]) ? argv[1] : "main"); //Conf file name
    br.start(); //Starts brain command line
    return 0;
}
