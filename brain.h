/*
* Copyright (c) 2012-13 Luke Montalvo <pikingqwerty@gmail.com>
*
* This file is part of NeuroSim.
* NeuroSim is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

using namespace std;

const int MAXMEM = 16;
const int MAXBR = 10;
int FPS = 5*CLOCKS_PER_SEC; //Frames per second
string brn[MAXBR];
int CBRAIN = 0;

class Brain;

Brain *brl[MAXBR];

class Neuron
{
		string type, name;
	        int active;
	        time_t lact; //Last active time; why is this needed?
	public:
		string nlink [MAXMEM]; //Name and level links
	        int llink [MAXMEM];
	        Neuron();
	        Neuron(string, string, int);
	        string getType() {return type;}
	        int setType(string t) {type=t;return 0;}
	        string getName() {return name;}
	        int setName(string n) {name=n;return 0;}
	        int getActive() {return active;}
	        int setActive(int a) {active=a;return 0;}
	        int getDataByName(string);
	        int getDataPart(string);
	        int setData(string, int); //Sets a link
	        int addLink(string); //Adds one to link with specified name
	        int reset(string, string, int); //Resets neuron with type, name, and activity level
	        int transfer(Neuron*); //Copies neuron
        friend class Brain; //Brain can access private values
};

Neuron::Neuron()
{
	type = "";
	name = "";
	active = 0;
}

Neuron::Neuron(string t, string n, int a)
{
	type = t;
	name = n;
	active = a;
	lact = time(NULL);
}

int Neuron::getDataByName(string n)
{
	for (int i=0;i<MAXMEM;i++)
	{
	        if (nlink[i].length() == 0) {break;}
	        if (nlink[i].compare(n)+1)
	        {
		return llink[i];
	        }
	}
	return -1;
}

int Neuron::setData(string n, int a)
{
	int de = -1;
	for (int i=0;i<MAXMEM;i++)
	{
		if (nlink[i] == n)
		{
			de = i;
		}
	}
	if (de >= 0)
	{
		nlink[de] = n;
		llink[de] = a;
		return 0;
	}
	for (int i=0;i<MAXMEM;i++)
	{
		if (nlink[i].length() == 0)
	        {
			nlink[i] = n;
			llink[i] = a;
			break;
		}
	}
	return 0;
}

int Neuron::addLink(string n)
{
	for (int i=0;i<MAXMEM;i++)
	{
		if (nlink[i].compare(n) == 0)
		{
			llink[i]++;
			break;
		}
	}
	return 0;
}

int Neuron::reset(string t, string n, int a)
{
	type = t;
	name = n;
	active = a;
	lact = time(NULL);
	return 0;
}

int Neuron::transfer(Neuron* n)
{
	type = n->type;
	name = n->name;
	active = n->active;
	lact = n->lact;
	for (int i=0;i<MAXMEM;i++)
	{
	        nlink[i] = n->nlink[i];
	        llink[i] = n->llink[i];
	}
	return 0;
}

class Brain
{
	public:
		Neuron ngroup [MAXMEM]; //Neuron group
		int lthought; //Last thought
		Neuron* cthought; //Current thought
		Brain(string, bool); //Read from file
		Brain(Neuron*, string); //Read from neuron array
		Neuron getThought();
		void doActive(); //Lower activity levels and strengthen links
		int update(); //Update to next thought
		int process(); //Handles last two functions
		int process(int); //Repeats given number of times
		int start(); //Begins input thread
		string gword(string); //Get sentence
		string pword(Neuron, string, int); //Process sentence
		int save(string); //Save brain to file
};

Brain::Brain(string f, bool r=false)
{
	FILE *file;
	char buf[128];
	if (f.find(".conf") == string::npos)
	{
		f += ".conf";
	}
	file = fopen(f.c_str(), "r");
	if (file == NULL)
	{
		cout << "Could not open file.";
		return;
	}
	int b = -1;
	for (int i=0;i<MAXBR;i++)
	{
		if (brn[i] == f.substr(0, f.find(".conf")))
		{
			if (r)
			{
				//Figure out how to properly reload
			}
			else
			{
				CBRAIN = i;
				b = -2;
				break;
			}
		}
		else if (brn[i] == "")
		{
			brn[i] = f.substr(0, f.find(".conf"));
			b = i;
			break;
		}
	}
	if (b == -1)
	{
		cout << "Not enough brain memory.\n";
		return;
	}
	else if (b == -2)
	{
		cout << "Brain already loaded, to reload from disk type \"reload\".\n";
		return;
	}
	else
	{
		CBRAIN = b;
	}
	Neuron *neu = new Neuron[MAXMEM];
	string sec, name, type, alev;
	int i = 0;
	while (fgets(buf, 128, file) != NULL)
	{
		if (i >= MAXMEM)
		{
			cout << "Not enough neural memory.";
			break;
		}
		if (buf[0] == '#') //Ignore comments
		{
			continue;
		}
		if (buf[0] == '[') //Sections
		{
			sec = string(buf).substr(string(buf).find("[")+1, string(buf).find("]")-1);
		}
		else //Keys
		{
			if (sec == "nlist") //nlist section
			{
				if (string(buf).find("=") != string::npos)
				{
					name = string(buf).substr(0, string(buf).find("="));
					type = string(buf).substr(string(buf).find("=")+1);
					type = type.erase(type.find("\n"), 1);
					neu[i].reset(type, name, 1000);
					i++;
				}
			}
			else //Other sections for links
			{
				int i = -1;
				for (int e=0;e<MAXMEM;e++)
				{
					if (neu[e].getName() == sec) //Finds section by name
					{
						i = e;
					}
				}
				if (i == -1) //Section doesn't exist
				{
					continue;
				}
				if (string(buf).find("=") != string::npos)
				{
					name = string(buf).substr(0, string(buf).find("="));
					alev = string(buf).substr(string(buf).find("=")+1, string(buf).find("\n"));
					neu[i].setData(name, atoi(alev.c_str())); //Add link
				}
			}
		}
	}
	fclose(file);
	for (int i=0;i<MAXMEM;i++)
	{
		ngroup[i].transfer(&neu[i]);
	}
	lthought = 0;
	cthought = &ngroup[0];
	brl[CBRAIN] = this;
}

Brain::Brain(Neuron* n, string nm)
{
	for (int i=0;i<MAXMEM;i++)
	{
		ngroup[i].transfer(&n[i]);
	}
	lthought = 0;
	cthought = &ngroup[0];
	for (int i=0;i<MAXBR;i++)
	{
		if (brn[i].compare("") == 0)
		{
			brn[i] = nm;
			brl[i] = this;
			break;
		}
	}
}

Neuron Brain::getThought()
{
	return *cthought;
}

void Brain::doActive()
{
	Neuron link [MAXMEM];
	bool rl = false;
	for (int i=0;i<MAXMEM;i++)
	{
		if (ngroup[i].getActive() == 1000) //Strengthen recent links
		{
			rl = true;
			link[i].setName(ngroup[i].getName());
		}
		if (ngroup[i].getActive() > 0) //Lower activity level
		{
			ngroup[i].setActive(ngroup[i].getActive()-1);
		}
	}
	if (rl)
	{
		for (int i=0;i<MAXMEM;i++)
		{
			for (int e=0;e<MAXMEM;e++)
			{
				if ((link[i].getName() != "")&&(link[i].getName().compare(ngroup[e].getName()) != 0)) //Do all links with correct name
				{
					ngroup[e].addLink(link[i].getName()); //Strengthen link by one
				}
			}
		}
	}
}

bool arrayContains(Neuron* ng, string n)
{
	for (int i=0;i<MAXMEM;i++)
	{
		if (ng[i].getName().compare(n) == 0) //If name exists
		{
			return true;
		}
	}
	return false;
}

int Brain::update()
{
    int ltop = 0; //Top link level
    int wtop = 0; //Array element of top level
    for (int i=0;i<MAXMEM;i++)
    {
		if ((getThought().llink[i] > ltop)&&(arrayContains(ngroup, getThought().nlink[i]))) //Top link? & In array?
		{
			ltop = getThought().llink[i];
			wtop = i;
		}
	}
    for (int i=0;i<MAXMEM;i++)
    {
        if (ngroup[i].getName().compare(getThought().nlink[wtop]) == 0) //If name exists
        {
            return i;
        }
    }
    return lthought; //If not, just return last thought
}

string Brain::gword(string w)
{
	Neuron t;
	bool b = false;
	string str = w;
	int i = 0;
	while (i<2)
	{
		b = false;
		for (int e=0;e<MAXMEM;e++)
		{
			if (ngroup[e].getName().compare(w) == 0)
			{
				t.transfer(&ngroup[e]);
				b = true;
				break;
			}
		}
		if (!b)
		{
			return str+".";
		}
		w = pword(t, w, i);
		str += " "+w;
		i++;
	}
	return str+".";
}

string Brain::pword(Neuron t, string w, int n)
{
	int ltop = 0; //Top link level
	int wtop = 0; //Array element of top level
	for (int i=0;i<MAXMEM;i++)
	{
		if ((t.llink[i] > ltop)&&(arrayContains(ngroup, t.nlink[i]))) //Top link? & In array?
		{
			ltop = getThought().llink[i];
			wtop = i;
		}
	}
	srand(time(NULL));
	for (int i=0;i<MAXMEM;i++)
	{
	        if (ngroup[i].getName().compare(getThought().nlink[wtop]) == 0) //If name exists
	        {
				if ((n == 0)&&(ngroup[i].getType().compare("verb") == 0))
				{
					return ngroup[i].getName();
				}
				else if ((n == 1)&&(ngroup[i].getType().compare("adjective") == 0))
				{
					return ngroup[i].getName();
				}
				return "";
		}
	}
	return "";
}

int Brain::save(string fname)
{
	if (fname == "save")
	{
		fname = brn[CBRAIN];
	}
	if (fname.find(".conf") == string::npos)
	{
		fname += ".conf";
	}
	
	ofstream file;
	file.open(fname.c_str(), ios::out | ios::trunc);
	if (file.is_open())
	{
		file << "# Copyright (c) 2012-13 Luke Montalvo <pikingqwerty@gmail.com>\n\
# \n\
# This file is a part of NeuroSim.\n\
# NeuroSim is free software and comes with ABSOLUTELY NO WARRANTY.\n\
# See LICENSE for more details.\n\
\n\
# This is an example brain configuration file\n\
# All neurons and their types go in [nlist]\n\
# All neurons are then listed with their connections and corresponding connection strengths\n\
# If a connection does not exist within the network, it will be ignored\n\
# \n\
# All lines within this file that start with a # will be ignored\n\
# Comments may not start in the middle of a line\n\n";
		file << "[nlist]\n";
		for (int i=0;i<MAXMEM;i++)
		{
			if (ngroup[i].getName() != "")
			{
				file << ngroup[i].getName() << "=" << ngroup[i].getType() << "\n";
			}
		}
		file << "\n";
		for (int i=0;i<MAXMEM;i++)
		{
			if (ngroup[i].getName() == "")
			{
				continue;
			}
			file << "[" << ngroup[i].getName() << "]\n";
			for (int e=0;e<MAXMEM;e++)
			{
				if (ngroup[i].nlink[e] != "")
				{
					file << ngroup[i].nlink[e] << "=" << ngroup[i].llink[e] << "\n";
				}
			}
		}
		file.close();
	}
	else
	{
		cout << "Failed to open file.\n";
		return 1;
	}
	
	return 0;
}

int wait(int t) //Wait specified number of milliseconds
{
	time_t start = clock();
	time_t now = clock();
	while ((now-start) < t)
	{
	        now = clock();
	}
	return 0;
}

void *parseInput(void *arg)
{
	char buf[128];
	while (true)
	{
		strcpy(buf, "");
		fputs((brn[CBRAIN]+"@brain$ ").c_str(), stdout);
		fflush(stdout);
		fgets(buf, 128, stdin);
		stringstream cmd(buf);
		string c;
		cmd >> c;
		if (c == "quit")
		{
			exit(EXIT_SUCCESS);
		}
		else if (c == "this")
		{
			cmd >> c;
			if (c == "name")
			{
				cout << (*brl[CBRAIN]).getThought().getName() << "\n";
			}
			else if (c == "type")
			{
				cout << (*brl[CBRAIN]).getThought().getType() << "\n";
			}
			else if (c == "level")
			{
				cout << (*brl[CBRAIN]).getThought().getActive() << "\n";
			}
			else if (c == "link")
			{
				for (int i=0;i<MAXMEM;i++)
				{
					if ((*brl[CBRAIN]).getThought().nlink[i] != "")
					{
						cout << (*brl[CBRAIN]).getThought().nlink[i] << ", " << (*brl[CBRAIN]).getThought().llink[i] << "\n";
					}
				}
			}
			else
			{
				cout << "Invalid option, type \"help this\".\n";
			}
		}
		else if (c == "add")
		{
			cmd >> c;
			if (c == "neuron")
			{
				cmd >> c;
				string name = c;
				cmd >> c;
				string type = c;
				int a = -1;
				for (int i=0;i<MAXMEM;i++)
				{
					if ((*brl[CBRAIN]).ngroup[i].getName().compare(name) == 0)
					{
						cout << "Neuron already exists.\n";
						continue;
					}
					if ((*brl[CBRAIN]).ngroup[i].getName().compare("") == 0)
					{
						a = i;
					}
				}
				if (a != -1)
				{
					(*brl[CBRAIN]).ngroup[a].reset(type, name, 1000);
					cout << "Neuron '" << (*brl[CBRAIN]).ngroup[a].getName() << "' added.\n";
				}
				else
				{
					cout << "Could not add neuron.\n";
				}
			}
			else if (c == "link")
			{
				//Add code to add new links
			}
			else
			{
				cout << "Invalid option, type \"help add\".\n";
			}
		}
		else if (c == "get")
		{
			cmd >> c;
			string name = c;
			cmd >> c;
			bool a = false;
			for (int i=0;i<MAXMEM;i++)
			{
				if ((*brl[CBRAIN]).ngroup[i].getName() == name)
				{
					if (c == "name")
					{
						cout << (*brl[CBRAIN]).ngroup[i].getName() << "\n";
					}
					else if (c == "type")
					{
						cout << (*brl[CBRAIN]).ngroup[i].getType() << "\n";
					}
					else if (c == "level")
					{
						cout << (*brl[CBRAIN]).ngroup[i].getActive() << "\n";
					}
					else if (c == "link")
					{
						for (int e=0;e<MAXMEM;e++)
						{
							if ((*brl[CBRAIN]).ngroup[i].nlink[e] != "")
							{
								cout << (*brl[CBRAIN]).ngroup[i].nlink[e] << ", " << (*brl[CBRAIN]).ngroup[i].llink[e] << "\n";
							}
						}
					}
					else
					{
						cout << "Invalid option, type \"help get\".\n";
					}
					a = true;
				}
			}
			if (!a)
			{
				cout << "Neuron '" << name << "' does not exist.\n";
			}
		}
		else if (c == "list")
		{
			cmd >> c;
			if (c != "list")
			{
				for (int i=0;i<MAXMEM;i++)
				{
					if ((*brl[CBRAIN]).ngroup[i].getType() == c)
					{
						cout << (*brl[CBRAIN]).ngroup[i].getName() << "\n";
					}
				}
			}
			else
			{
				for (int i=0;i<MAXMEM;i++)
				{
					if ((*brl[CBRAIN]).ngroup[i].getName() != "")
					{
						cout << (*brl[CBRAIN]).ngroup[i].getName() << "\n";
					}
				}
			}
		}
		else if (c == "proc")
		{
			cmd >> c;
			if (c != "proc")
			{
				for (int i=0;i<atoi(c.c_str());i++)
				{
					cout << (*brl[CBRAIN]).getThought().getName() << "\n";
					(*brl[CBRAIN]).process();
					if (i != atoi(c.c_str())-1)
					{
						wait(CLOCKS_PER_SEC/(FPS/CLOCKS_PER_SEC));
					}
				}
			}
			else
			{
				cout << "Invalid option, type \"help proc\".\n";
			}
		}
		else if (c == "speed")
		{
			cmd >> c;
			if (c != "speed")
			{
				FPS = atof(c.c_str())*CLOCKS_PER_SEC;
			}
			cout << "Processing at " << FPS/CLOCKS_PER_SEC << " frames per second.\n";
		}
		else if (c == "save")
		{
			cmd >> c;
			cout << "Saving...\n";
			if ((*brl[CBRAIN]).save(c))
			{
				cout << "Failed to save.\n";
			}
			else
			{
				cout << "Save complete.\n";
			}
		}
		else if (c == "switch")
		{
			cmd >> c;
			if (c != "switch")
			{
				cout << "Saving current brain...\n";
				if ((*brl[CBRAIN]).save("save"))
				{
					cout << "Failed to save, aborting switch.\n";
					continue;
				}
				else
				{
					cout << "Loading \"" << c << "\"...\n";
					Brain btmp(c);
					cout << "Load complete.\n";
					btmp.start();
					break;
				}
			}
			else
			{
				cout << "Invalid option, type \"help switch\".\n";
			}
		}
		else if (c == "reload")
		{
			cout << "Reloading from disk...\n";
			Brain btmp(brn[CBRAIN], true);
			cout << "Reload complete.\n";
			btmp.start();
			break;
		}
		else if (c == "word")
		{
			cmd >> c;
			cout << (*brl[CBRAIN]).gword(string(c)) << "\n";
		}
		else if (c == "help")
		{
			cmd >> c;
			if (c == "this")
			{
				cout << "To use this command, type \"this cmd\" where cmd is one of the following:\n\
\n\
	name	Get the current neuron's name.\n\
	type	Get the current neuron's type.\n\
	level	Get the current neuron's activation level.\n\
	link	Get the current neuron's link levels.\n";
			}
			else if (c == "add")
			{
				cmd >> c;
				if (c == "neuron")
				{
					cout << "To use this command, type \"add neuron name type\" where name and type are strings.\n\
\n\
	name	The name of the neuron.\n\
	type	The type that the neuron is classified as.\n";
				}
				else if (c == "link")
				{
					cout << "To use this command, type \"add link neuron1 neuron2 level\" where neurons 1 and 2 are the names of the neurons and level is the link level from neuron1 to neuron2.\n\
\n\
Note that links are only set from neuron1 to neuron2, not the other way around.\n";
				}
				else
				{
					cout << "To use this command, type \"add type options\" where type is one of the following:\n\
\n\
	neuron	Add a new neuron\n\
	link	Add a new link between neurons\n\
Required options can be viewed with \"help add type\".\n";
				}
			}
			else if (c == "get")
			{
				cout << "To use this command, type \"get name cmd\" where name is the name of the neuron and cmd is one of the following:\n\
\n\
	name	Gets the neuron's name.\n\
	type	Gets the neuron's type.\n\
	level	Gets the neuron's activation level.\n\
	link	Gets the neuron's link levels.\n";
			}
			else if (c == "list")
			{
				cout << "To use this command, type \"list [type]\" where type is optional and is the type of neurons to list.\n";
			}
			else if (c == "proc")
			{
				cout << "To use this command, type \"proc number\" where number is the number of neurons to process.\n";
			}
			else if (c == "speed")
			{
				cout << "To use this command, type \"speed [fps]\" where fps is optional and is the number of frames to process per second.\n\
If fps is excluded, then the current speed is shown.\n";
			}
			else if (c == "save")
			{
				cout << "To use this command, type \"save [name]\" where name is optional and is the new file to save the brain as.\n\
If name is excluded, the brain will be saved to the same file.\n\
If the file does not end in \".conf\", it will be automatically appended.\n\
\n\
Note that if the name is changed, the new name will not be used until you type \"switch name\". If you type \"save\", without a name, the old name will be used.\n\
This funtionality can be used to create diverging copies of a brain while keeping the original intact.\n";
			}
			else if (c == "switch")
			{
				cout << "To use this command, type \"switch name\" where name is the name of the brain's file.\n\
If the file does not end in \".conf\", it will be automatically appended.\n\
\n\
Note that this automatically saves the current brain before switching.\n\
If you would not like to save it, type \"quit\" then rerun this program using the desired brain configuration file as the first argument.\n";
			}
			else if (c == "word")
			{
				cout << "To use this command, type \"word name\" where name is the name of the neuron.\n\
This command will attempt to form a sentence based on the neuron and its various links. Do not expect much.\n";
			}
			else if (c == "quit")
			{
				cout << "To use this command, type \"quit\". This will exit the program without saving the brain. You have been warned.\n";
			}
			else
			{
				cout << "Type \"help cmd\" for more help on these commands.\n\
\n\
	this	Get information about the current neuron.\n\
	add 	Add a new neuron or neuron links.\n\
	get 	Get information about a certain neuron.\n\
	list	List neurons.\n\
	proc	Process through a number of links.\n\
	speed	Set or display the processing speed.\n\
	save	Save the neuron list to disk.\n\
	switch	Switch to a differecnt brain.\n\
	reload	Reloads the current brain from disk.\n\
	word	Construct a sentence about a neuron.\n\
	quit	Close the program, not saving changes.\n";
			}
		}
		else
		{
			cout << "\"" << c << "\" is not a valid command. Type \"help\" for a list.\n";
		}
	}
	
	return 0;
}

int Brain::process()
{
	return process(1);
}

int Brain::process(int n)
{
	int i=0;
	while (i<n)
	{
		doActive(); //Strengthen recent links and lower activity levels
		int top = update(); //Get new thought
		lthought = top;
		cthought = &ngroup[top];
		i++;
	}
	return 0;
}

int Brain::start()
{
	pthread_t pIth;
	pthread_create(&pIth, NULL, parseInput, NULL);
	pthread_join(pIth, NULL);
	return 0;
}
