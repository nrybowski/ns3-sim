#include "NtfParser.h"

const string NtfContent::delimiter = " ";
const string NtfContent::comment_tag = "#";

int NtfContent::getNodeId(string node) {
    map<string, unsigned int>::iterator it = nodes.find(node);
    if (it == nodes.end()) return -1;
    return (int) it->second;
}

size_t NtfContent::nNodes() {
    return nodes.size();
}

const vector<Link>::iterator NtfContent::getLinks() {
    return links.begin();
}

const vector<Link>::iterator NtfContent::getLastLink() {
    return links.end();
}

const map<string, unsigned int>::iterator NtfContent::getNodes() {
    return nodes.begin();
}

const map<string, unsigned int>::iterator NtfContent::getLastNode() {
    return nodes.end();
}

void NtfContent::dumpNodes() {
    for (map<string, unsigned int>::iterator it = nodes.begin(); it != nodes.end(); it++) {
	cout << it->second << ": " << it->first << endl;
    }
}

void NtfContent::dumpNodes(string filename) {
    ofstream out(filename.c_str());
    for (map<string, unsigned int>::iterator it = nodes.begin(); it != nodes.end(); it++) {
	out << it->second << " " << it->first << endl;
    }
    out.close();
}

void NtfContent::dumpLinks() {
    for (vector<Link>::iterator it = links.begin(); it != links.end(); it++) {
	cout << *it << endl;
    }
}

NtfContent::NtfContent(string filename) {

    // Open NTF file
    ifstream ntf(filename.c_str());
    if (!ntf) {
	cout << "Can't open " << filename << " file." << endl;
	exit(1);
    }
    
    // Parse NFT file line by line
    string line, token;
    map<tuple<string, string>, Link> links2;
    while (getline(ntf, line)) {

	// Skip comments
	if (line.substr(0,1) == comment_tag) continue;

	// Put 4 first line elements in vector
	size_t pos, count = 0;
	vector<string> parsed_line;
	while ((pos = line.find(delimiter))) {
	    token = line.substr(0, pos);
	    parsed_line.push_back(token);
	    line.erase(0, pos + delimiter.length());

	    // EOL or we parsed the 4 elements we are looking for
	    // (ignore remaining elements)
	    if (pos == string::npos || ++count > 6 ) break;
	}

	string origin = parsed_line.at(0),
			end = parsed_line.at(1);
	int metric = mycast(parsed_line.at(2)),
	    delay = mycast(parsed_line.at(3)),
	    start = -1,
	    duration = 0;
	if (parsed_line.size() >= 5)
	    start = mycast(parsed_line.at(4));
	if (parsed_line.size() >= 6)
	    duration = mycast(parsed_line.at(5));

	tuple<string, string> forward_key = make_tuple(origin, end), reverse_key = make_tuple(end, origin);
	auto forward = links2.find(forward_key), reverse = links2.find(reverse_key);

	// Link logic
	if (forward == links2.end()) {
	    if (reverse == links2.end()) {
		// link does not exist, insert forward
		Link link(get<0>(forward_key), get<1>(forward_key), metric, delay);
		link.failure_start = start;
		link.failure_duration = duration;

		links2.insert(
			pair<tuple<string, string>, Link>(
			    forward_key,
			    link
			)
		);
		nodes.insert(pair<string, int>(origin, nodes.size()));
		nodes.insert(pair<string, int>(end, nodes.size()));
	    } else {
		// current link is reverse direction of a known forward
		Link l = reverse->second;
		if (delay != l.delay) {
		    // delay not the same in both directions, its a new link
		    cerr << "not supported" << endl;
		    exit(1);
		} else {
		    // delays are the same in both direction, we consider them as the same link
		    l.r_metric = metric;
		    links2.erase(reverse);
		    links2.insert(pair<tuple<string, string>, Link>(reverse_key, l)); 
		}
	    }
	}
    }
    /*cout << links2.size() << endl;
    for (map<tuple<string, string>, Link>::iterator it = links2.begin(); it != links2.end(); it++) {
	cout << it->second << endl;
    }*/
    for (auto it = links2.begin(); it != links2.end(); it++) {
	links.push_back(it->second);
    }
}

int main(void) {
    NtfContent ntf("geant.ntf");
    ntf.dumpLinks();
    ntf.dumpNodes();
}
