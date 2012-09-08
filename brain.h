/*
* Copyright (c) 2012 piluke <pikingqwerty@gmail.com>
*
* This file is part of NeuroSim.
* NeuroSim is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#include <iostream>
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
		Brain(string); //From file
        Brain(Neuron*, string); //From neuron array
        Neuron getThought();
        void doActive(); //Lower activity levels and strengthen links
        int update(); //Update to next thought
        int process(); //Handles last two functions
        int process(int); //Repeats given number of times
        int start(); //Begins input thread
        string gword(string); //Get sentence
        string pword(Neuron, string, int); //Process sentence
};

Brain::Brain(string f)
{
	FILE *file;
	char buf [64];
	file = fopen((f+".ini").c_str(), "r");
	if (file == NULL)
	{
		cout << "Could not open file.";
		return;
	}
	int b = -1;
	for (int i=0;i<MAXBR;i++)
	{
		if (brn[i].compare("") == 0)
		{
			brn[i] = f;
			b = i;
			break;
		}
	}
	Neuron *neu = new Neuron[MAXMEM];
	char sec[64] = "nosec";
	char name[64] = "noname";
	char type[64] = "notype";
	char alev[8] = "nolev";
	int i = 0;
	while (fgets(buf, 128, file) != NULL)
	{
		if (i >= MAXMEM)
		{
			cout << "Not enough neural memory.";
			break;
		}
		if (buf[0] == '[') //Sections
		{
			memset(sec, 0, 64); //Clear section
			for (int e=0;e<64;e++) //Copy section name without brackets
			{
				if (buf[e+1] == ']')
				{
					break;
				}
				sec[e] = buf[e+1];
			}
		}
		else //Keys
		{
			memset(name, 0, 64); //Reset to blank arrays
			memset(type, 0, 64);
			if (strcmp(sec, "nlist") == 0) //nlist section
			{
				char *ep = strstr(buf, "=");
				strncpy(name, buf, ep-buf); //Text before '='
				strcpy(type, buf);
				memmove(type, type+(ep-buf+1), 64-(ep-buf+1)); //Text after '='
				ep = strstr(type, "\n");
				memset(type+(ep-type), 0, 64-(ep-type)); //Remove newline
				string n = name; //Convert to string
				string t = type;
				neu[i].reset(t, n, 1000); //Add to list
				i++;
			}
			else //Other sections for links
			{
				i = -1;
				for (int e=0;e<MAXMEM;e++)
				{
					if (neu[e].getName().compare(sec) == 0) //Finds section by name
					{
						i = e;
					}
				}
				if (i == -1) //Section doesn't exist
				{
					continue;
				}
				char *ep = strstr(buf, "=");
				strncpy(name, buf, ep-buf); //Text before '='
				strncpy(alev, buf, 8);
				strncpy(alev, ep+1, 8); //Text after '='
				ep = strstr(alev, "\n");
				memset(alev+(ep-alev), 0, 8-(ep-alev)); //Remove newline
				string n = name; //Convert to string
				neu[i].setData(n, atoi(alev)); //Add link
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
	if (b == -1)
	{
		cout << "Not enough brain memory.\n";
		return;
	}
	brl[b] = this;
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
	int b = 0;
	int q = 0;
	while (true)
	{
		strcpy(buf, "");
		fputs((brn[b]+"@brain$ ").c_str(), stdout);
		fflush(stdout);
		fgets(buf, 128, stdin);
		char cmd[8];
		strncpy(cmd, buf, strcspn(buf, " \n"));
		if (strcmp(cmd, "quit") == 0)
		{
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(cmd, "this") == 0)
		{
			char n[8] = "";
			strncpy(n, strstr(buf, "this")+5, 8);
			if (strcmp(n, "name\n") == 0)
			{
				cout << (*brl[b]).getThought().getName() << "\n";
			}
			else if (strcmp(n, "type\n") == 0)
			{
				cout << (*brl[b]).getThought().getType() << "\n";
			}
			else if (strcmp(n, "level\n") == 0)
			{
				cout << (*brl[b]).getThought().getActive() << "\n";
			}
			else if (strcmp(n, "link\n") == 0)
			{
				for (int i=0;i<MAXMEM;i++)
				{
					if ((*brl[b]).getThought().nlink[i].compare("") != 0)
					{
						cout << (*brl[b]).getThought().nlink[i] << ", " << (*brl[b]).getThought().llink[i] << "\n";
					}
				}
			}
			else
			{
				cout << "Not a valid option.\n";
			}
		}
		else if (strcmp(cmd, "add") == 0)
		{
			char n[160] = "";
			strncpy(n, strstr(buf, "add")+4, 160);
			char *p = strtok(n, " \n");
			int a = -1;
			for (int i=0;i<MAXMEM;i++)
			{
				if ((*brl[b]).ngroup[i].getName().compare(p) == 0)
				{
					cout << "Neuron already exists.\n";
					continue;
				}
				if ((*brl[b]).ngroup[i].getName().compare("") == 0)
				{
					a = i;
				}
			}
			char name[128];
			strncpy(name, p, 128);
			char *type;
			type = strtok(NULL, " \n");
			if (a != -1)
			{
				(*brl[b]).ngroup[a].reset(type, name, 1000);
				cout << "Neuron '" << (*brl[b]).ngroup[a].getName() << "' added.\n";
			}
			else
			{
				cout << "Could not add neuron.\n";
			}
		}
		else if (strcmp(cmd, "get") == 0)
		{
			char n[160] = "";
			strncpy(n, strstr(buf, "get")+4, 160);
			char *p = strtok(n, " ");
			char cmd[128];
			strncpy(cmd, p, 128);
			char *name;
			name = strtok(NULL, " \n");
			bool a = false;
			for (int i=0;i<MAXMEM;i++)
			{
				if ((*brl[b]).ngroup[i].getName().compare(name) == 0)
				{
					if (strcmp(cmd, "name") == 0)
					{
						cout << (*brl[b]).ngroup[i].getName() << "\n";
					}
					else if (strcmp(cmd, "type") == 0)
					{
						cout << (*brl[b]).ngroup[i].getType() << "\n";
					}
					else if (strcmp(cmd, "level") == 0)
					{
						cout << (*brl[b]).ngroup[i].getActive() << "\n";
					}
					else if (strcmp(cmd, "link") == 0)
					{
						bool b = false;
						for (int e=0;e<MAXMEM;e++)
						{
							if ((*brl[b]).ngroup[i].nlink[e].compare("") != 0)
							{
								cout << (*brl[b]).ngroup[i].nlink[e] << ", " << (*brl[b]).ngroup[i].llink[e] << "\n";
								b = true;
							}
						}
						if (!b)
						{
							cout << "No links.\n";
						}
					}
					else
					{
						cout << "Not a valid option.\n";
					}
					a = true;
				}
			}
			if (!a)
			{
				cout << "Neuron '" << name << "' does not exist.\n";
			}
		}
		else if (strcmp(cmd, "list") == 0)
		{
			for (int i=0;i<MAXMEM;i++)
			{
				if ((*brl[b]).ngroup[i].getName().compare("") != 0)
				{
					cout << (*brl[b]).ngroup[i].getName() << "\n";
				}
			}
		}
		else if (strcmp(cmd, "proc") == 0)
		{
			char n[8] = "";
			strncpy(n, strstr(buf, "proc")+5, 8);
			for (int i=0;i<atoi(n);i++)
			{
				cout << (*brl[b]).getThought().getName() << "\n";
				(*brl[b]).process();
				if (i != atoi(n)-1)
				{
					wait(CLOCKS_PER_SEC/(FPS/CLOCKS_PER_SEC));
				}
			}
		}
		else if (strcmp(cmd, "speed") == 0)
		{
			char n[8] = "";
			strncpy(n, strstr(buf, "speed")+6, 8);
			if (strcmp(n, "") != 0)
			{
				FPS = atof(n)*CLOCKS_PER_SEC;
			}
			cout << "Will process at " << FPS/CLOCKS_PER_SEC << " frames per second.\n";
		}
		else if (strcmp(cmd, "save") == 0)
		{
			cout << "Saving...\n";
			cout << "Done.\n";
		}
		else if (strcmp(cmd, "word") == 0)
		{
			char n[64] = "";
			strncpy(n, strstr(buf, "word")+5, 64);
			strtok(n, " \n");
			cout << (*brl[b]).gword(string(n)) << "\n";
		}
		else if (strcmp(cmd, "help") == 0)
		{
			string h = "Right now there are no command-line options.\n\
Type \"help cmd\" for more help on certain commands.\n\
\n\
	this	Get information about the current neuron.\n\
	add 	Add a new neuron.\n\
	get 	Get information about a certain neuron.\n\
	list	List all neurons.\n\
	proc	Process through a number of links.\n\
	speed	Set or display the processing speed.\n\
	word	Construct a sentence about a neuron.\n\
	quit	Close the program, not saving changes.\n";
			cout << h;
		}
		else
		{
			cout << "\"" << cmd << "\" is not a valid command. Type \"help\" for a list.\n";
		}
	}
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
