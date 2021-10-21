#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

inline int mycast(string token) {
    const char *t = token.c_str();
    int value = atoi(t);
    return (value != 0 || (value == 0 && strcmp(t, "0") == 0)) ? value : -1;
}

class Link {
    public:
	string origin;
	string end;
	int delay;
	int f_metric;
	int r_metric;
	Link(string o, string e, int m, int d) {
	    origin = o;
	    end = e;
	    delay = d;
	    f_metric = m;
	    r_metric = 0;
	}
	
	friend ostream& operator<<(ostream& os, const Link &l) {
	    os << "origin <" << l.origin 
		<< "> end <" << l.end 
		<< "> | delay <" << l.delay
		<< "> | f_metric <" << l.f_metric 
		<< "> r_metric <" << l.r_metric
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
	void dumpLinks();
	size_t nNodes();
	int getNodeId(string node);
};
