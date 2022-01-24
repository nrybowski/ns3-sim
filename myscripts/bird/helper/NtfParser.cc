#include "NtfParser.h"

const string NtfContent::delimiter = " ";
const string NtfContent::comment_tag = "#";

int NtfContent::getNodeId(string node) {
    map<string, unsigned int>::iterator it = nodes.find(node);
    if (it == nodes.end()) return -1;
    return (int) it->second;
}

/*size_t NtfContent::nNodes() {
    return nodes.size();
}*/

size_t NtfContent::nLinks() {
    return links.size();
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
	cout << *it;
    }
}

/*void insertLink(
	map<tuple<string, string>, Link> *links2,
	map<string, unsigned int> *nodes,
	tuple<string, string> forward_key, 
	int metric, 
	int delay, 
	int start, 
	int duration)
{
    string origin = get<0>(forward_key),
	   end = get<1>(forward_key);
    Link link(origin, end, metric, delay);
    link.failure_start = start;
    link.failure_duration = duration;

    links2->insert(
	    pair<tuple<string, string>, Link>(
		forward_key,
		link
	    )
    );
    nodes->insert(pair<string, int>(origin, nodes->size()));
    nodes->insert(pair<string, int>(end, nodes->size()));
}*/

NtfContent::NtfContent(string filename) {

    nNodes = 0;

    // run python parser
    string cmd = "python3 ntf_parser.py " + filename;
    FILE *f = popen(cmd.c_str(), "r");
    if (!f) {
	cerr << "Failed to run python parser" <<endl;
	exit(1);
    }

    // get output
    char buf[1];
    //memset(buf, 0, sizeof(buf));
    string content, line, token;

    while (fread((void*) buf, sizeof(char), sizeof(buf), f) > 0) {
	content.append(buf);
	//memset(buf, 0, sizeof(buf));
    }
    pclose(f);

    // parse output line by line
    istringstream in;
    in.str(content);
    while (getline(in, line)) {

	// skip comments
	if (line.substr(0, 1) == comment_tag) continue;

	// parse line
	size_t pos, count = 0;
	vector<string> parsed_line;
	while ((pos = line.find(delimiter))) {
	    token = line.substr(0, pos);
	    parsed_line.push_back(token);
	    line.erase(0, pos + delimiter.length());

	    // EOL or we parsed the 4 elements we are looking for
	    // (ignore remaining elements)
	    if (pos == string::npos || ++count > 7) break;
	}

	// add new link
	Link link(
	    mycast(parsed_line.at(0).c_str()),
	    mycast(parsed_line.at(1).c_str()),
	    mycast(parsed_line.at(2).c_str()),
	    mycast(parsed_line.at(3).c_str()),
	    mycast(parsed_line.at(4).c_str()),
	    mycast(parsed_line.at(5).c_str()),
	    mycast(parsed_line.at(6).c_str())
	);
	links.push_back(link);

	// get highest node id
	if (link.origin > nNodes) nNodes = link.origin;
	if (link.end > nNodes) nNodes = link.end;
    }
    // increment to have node count
    nNodes++;

    /*// Open NTF file
    ifstream ntf(filename.c_str());
    if (!ntf) {
	cout << "Can't open " << filename << " file." << endl;
	exit(1);
    }
    
    // Parse NFT file line by line
    string line, token;
    // key = <src_node, dst_node>, value = Link
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
	int metric = atoi(parsed_line.at(2).c_str()),
	    delay = mycast(parsed_line.at(3)),
	    start = -1,
	    duration = 0;
	if (parsed_line.size() >= 5)
	    start = mycast(parsed_line.at(4));
	if (parsed_line.size() >= 6)
	    duration = mycast(parsed_line.at(5));

	// create forward and reverse keys
	tuple<string, string> forward_key = make_tuple(origin, end), reverse_key = make_tuple(end, origin);

	// search if links exists in forward or reverse direction
	auto forward = links2.find(forward_key), reverse = links2.find(reverse_key);

	// Link logic
	if (forward == links2.end()) {
	    // link not known in forward direction
	    if (reverse == links2.end()) {
		// link not known in reverse direction so link does not exist, insert forward
		insertLink(&links2, &nodes, forward_key, metric, delay, start, duration);
	    } else {
		// current link is reverse direction of a known forward
		// B A M_BA D_BA
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
	} else {
	    // link already known in forward direction
	    cerr << "already known" <<endl;
	    Link l = forward->second;
	    if (!l.r_metric) {
		cerr << "no reverse" <<endl;
	    } else {
		cerr << "full link" <<endl;
	    }
	}
    }*/
    /*cout << links2.size() << endl;
    for (map<tuple<string, string>, Link>::iterator it = links2.begin(); it != links2.end(); it++) {
	cout << it->second << endl;
    }*/
    /*for (auto it = links2.begin(); it != links2.end(); it++) {
	links.push_back(it->second);
    }
    printf("n_links %lu\n", links.size());*/
}

int main(int argc, char **argv) {
    if (argc < 2) {
	printf("Usage %s\n", argv[0]);
	exit(1);
    }
    char buf[100];
    strncpy(buf, argv[1], sizeof(buf));
    NtfContent ntf(buf);
    printf("Nnodes %lu\nNLinks %lu\n", ntf.nNodes, ntf.nLinks());
    ntf.dumpLinks();
    ntf.dumpNodes();
}
