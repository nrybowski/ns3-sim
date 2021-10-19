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
	int metric;
	int delay;
	
	friend ostream& operator<<(ostream& os, const Link &l) {
	    os << "Origin: " << l.origin 
		<< "\nEnd: " << l.end 
		<< "\nMetric: " << l.metric 
		<< "\nDelay: " << l.delay 
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
	const map<string, unsigned int>::iterator getNodes();
	void dumpNodes();
	void dumpLinks();
	size_t nNodes();
};
