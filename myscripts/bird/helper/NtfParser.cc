#include "NtfParser.h"

const string NtfContent::delimiter = " ";
const string NtfContent::comment_tag = "#";

size_t NtfContent::nNodes() {
    return nodes.size();
}

const vector<Link>::iterator NtfContent::getLinks() {
    return links.begin();
}

const map<string, unsigned int>::iterator NtfContent::getNodes() {
    return nodes.begin();
}

void NtfContent::dumpNodes() {
    for (map<string, unsigned int>::iterator it = nodes.begin(); it != nodes.end(); it++) {
	cout << it->second << ": " << it->first << endl;
    }
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
    while (getline(ntf, line)) {

	// Skip comments
	if (line.substr(0,1) == comment_tag) continue;

	size_t pos, count = 0;
	int token2int;
	Link link;

	while ((pos = line.find(delimiter))) {
	    token = line.substr(0, pos);
	    line.erase(0, pos + delimiter.length());

	    switch (count) {
		case 0:
		    link.origin = token;
		    if (nodes.find(token) == nodes.end())
			nodes.insert(pair<string, unsigned int>(token, nodes.size()));
		    break;
		case 1:
		    link.end = token;
		    break;
		case 2:
		    token2int = mycast(token);
		    if (token2int == -1) {
			cout << "invalid metric" << endl;
			exit(1);
		    }
		    link.metric = token2int;
		    break;
		case 3:
		    token2int = mycast(token);
		    if (token2int == -1) {
			cout << "invalid delay" << endl;
			exit(1);
		    }
		    link.delay = token2int;
		    break;	
		default:
		    {};
	    }
	    
	    // EOL or we parsed the 4 elements we are looking for
	    // (ignore remaining elements)
	    if (pos == string::npos || ++count >= 4) break;
	}
	links.push_back(link);
    }
}

int main(void) {
    NtfContent ntf("geant.ntf");
    //ntf.dumpLinks();
    ntf.dumpNodes();
}
