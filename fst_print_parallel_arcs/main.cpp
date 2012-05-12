#include <fst/fst.h>
//#include <fst/main.h>
#include <iostream>
#include <climits>
#include <float.h>
#include <vector>
#include <algorithm>

#include "fst_properties.h"
#include "nodes.h"
#include "path.h"

using namespace std;
using namespace fst;

void help(char *pAppname)
{
	cerr << "Usage: " << pAppname << " [flags] in.fst" << endl;
	cerr << "Flags: " << endl;
	cerr << "  --symbols: type = string, default = \"\"" << endl;
    cerr << "    Symbol table" << endl;
	cerr << "  --help:" << endl;
	cerr << "    Show this help" << endl;
}

struct OverlappedScoreType {
	enum Enum {
		best,
		logadd
	};
};

int main(int argc, char **argv)
{
	char * pfst_filename = 0;
	char * psyms_filename = 0;
	bool print_full_info = false;

	// PARSE COMMAND LINE ARGUMENTS
	if (argc <= 1) {
		help(argv[0]);
		return 1;
	}
	int i=1;
	while (i < argc)
	{
		if (strcmp(argv[i], "--symbols") == 0) {
			i++;
			psyms_filename = argv[i];
		} 
		else if (strcmp(argv[i], "--full-info") == 0) {
			print_full_info = true;
		}
		else if (strcmp(argv[i], "--help") == 0) {
			help(argv[0]);
			return 0;
		}
		else if (i == argc-1) {
			pfst_filename = argv[argc-1];
		}
		else {
			cerr << "ERROR: Unknown argument '"<<argv[i]<<"'" << endl;
			help(argv[0]);
			return 1;
		}
		i++;
	}

	if (!psyms_filename || !pfst_filename) {
		cerr << "ERROR: missing arguments." << endl;
		help(argv[0]);
		return 1;
	}

	SymbolTable* syms = NULL;
	syms = SymbolTable::ReadText(psyms_filename);

	typedef LogArc Arc;

	Fst<Arc>* fst = Fst<Arc>::Read(strcmp(pfst_filename, "-") == 0 ? "" : pfst_filename);
	if (fst->Start() < 0) {
		cerr << "Warning: FST is empty!" << endl;
	} else {
		if (!FstProperties::IsTopologicallySorted(*fst)) {
			cerr << "Warning: FST is not topologically sorted! ...sorting...";
			TopSort((MutableFst<Arc>*)fst);
			cerr << "done" << endl;
		}

		typedef PathMultWeight<Arc> Path; // PathAvgWeight | PathMultWeight

		Path::SetPrintType(PRINT_ALL);

		typedef Nodes<Arc> Nodes;
		Nodes nodes(*fst, syms);
		for (StateIterator<Fst<Arc>> siter(*fst); !siter.Done(); siter.Next())
		{
			cout << fixed << setprecision(2);
			unsigned int state_id = siter.Value();
			Nodes::Node& n = nodes[state_id];
			unsigned int pa_count = 0;
			foreach_pair (int nextstate, const ParallelArcs<Arc>& pa, n.GetParallelArcs()) {
				// skip self-loops
				if (nextstate == state_id) continue;

				if (++pa_count > 1) {
					THROW("ERROR: Arcs of one node should all point to the same next node! (like in a confusion network)");
				}

				string separator = "";
				std::vector<const Arc*> v(pa.size());
				std::copy(pa.begin(), pa.end(), v.begin());
				std::sort(v.begin(), v.end(), 
					[](const Arc* a1, const Arc* a2) {
						return a1->weight.Value() < a2->weight.Value();
					}
				);

				if (print_full_info) {
					cout << pa << endl;
				} else {
					cout << fixed << setprecision(2) << "endt=" << pa.GetEndTime();

					//foreach(const Arc* a, pa) {
					foreach(const Arc* a, v) {
						//cout << separator;
						ostringstream oss;
						oss << fixed << setprecision(2);
						if (syms) {oss << syms->Find(a->ilabel);} else {oss << a->ilabel;}
						oss << "/" << a->weight;
						cout << setw(12) << oss.str();
					}
					//cout << "]/"<<pa.GetWeight()<<" -> " << nextstate;
					cout << endl;
				}
			}
		}
	}
	delete fst;

	if (syms) {
		delete syms;
	}
}
