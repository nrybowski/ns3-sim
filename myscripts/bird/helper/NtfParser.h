#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

inline int mycast(string token) {
    const char *t = token.c_str();
    int value = atoi(t);
    if (value != 0 || (value == 0 && strcmp(t, "0") == 0)) 
	return value;
    else {
	printf("Failed to cast string <%s> into int <%i>\n", t, value);
	exit(1);
    }
}

class Link {
    public:
	//string origin;
	int origin;
	//string end;
	int end;
	int delay;
	int f_metric;
	int r_metric;
	int failure_start;
	int failure_duration;
	Link(int src, int dst, int f_metric, int r_metric, int delay, int f_start, int duration) {
	    this->origin = src;
	    this->end = dst;
	    this->f_metric = f_metric;
	    this->r_metric = r_metric;
	    this->delay = delay;
	    this->failure_start = f_start;
	    this->failure_duration = duration;
	}

	/*Link(string o, string e, int m, int d) {
	    //origin = o;
	    //end = e;
	    delay = d;
	    f_metric = m;
	    r_metric = 0;
	    failure_start = -1;
	    failure_duration = 0;
	}*/
	
	friend ostream& operator<<(ostream& os, const Link &l) {
	    os << "origin <" << l.origin 
		<< "> end <" << l.end 
		<< "> | delay <" << l.delay
		<< "> | f_metric <" << l.f_metric 
		<< "> r_metric <" << l.r_metric
		<< "> | failure_start <" << l.failure_start
		<< "> failure_duration <" <<l.failure_duration
		<< ">"
		<< endl;
	    return os;
	}

};

class NtfContent {
    private:
	vector<Link> links;
	map<string, unsigned int> nodes;
	static const string delimiter, comment_tag;

    public:
	NtfContent(string filename);
	const vector<Link>::iterator getLinks();
	const vector<Link>::iterator getLastLink();
	const map<string, unsigned int>::iterator getLastNode();
	const map<string, unsigned int>::iterator getNodes();
	void dumpNodes();
	void dumpNodes(string filename);
	void dumpLinks();
	//size_t nNodes();
	size_t nNodes;
	int getNodeId(string node);
};
