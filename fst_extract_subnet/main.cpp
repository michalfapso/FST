#include <fst/fst.h>
#include <fst/vector-fst.h>
//#include <fst/main.h>
#include <iostream>
#include <climits>
#include <float.h>

//#include "fstprintnshortestpaths_mergeoverlapping_forward.h"
#include "fst_subnet_extractor.h"

using namespace std;
using namespace fst;

void help(char *pAppname)
{
	cerr << "Usage: " << pAppname << " [flags] in.fst out.fst" << endl;
	cerr << "Flags: " << endl;
	cerr << "  --start-state: type = integer" << endl;
	cerr << "  --end-state: type = integer" << endl;
	cerr << "  --help:" << endl;
	cerr << "    Show this help" << endl;
}

int main(int argc, char **argv)
{
	char * pfst_in_filename = 0;
	char * pfst_out_filename = 0;
	int start_state = -1;
	int end_state = -1;

	// PARSE COMMAND LINE ARGUMENTS
	if (argc <= 1) {
		help(argv[0]);
		return 1;
	}
	int i=1;
	while (i < argc)
	{
		if (strcmp(argv[i], "--start-state") == 0) {
			i++;
			start_state = atol(argv[i]);
		} 
		else if (strcmp(argv[i], "--end-state") == 0) {
			i++;
			end_state = atol(argv[i]);
		} 
		else if (i == argc-2) {
			pfst_in_filename = argv[i];
		}
		else if (i == argc-1) {
			pfst_out_filename = argv[i];
		}
		else {
			cerr << "ERROR: Unknown argument '"<<argv[i]<<"'" << endl;
			help(argv[0]);
			return 1;
		}
		i++;
	}

	// LOAD SYMBOL TABLES
	if (!pfst_in_filename || !pfst_out_filename || start_state < 0 || end_state < 0) {
		help(argv[0]);
		return 1;
	}

	typedef LogArc Arc;

	Fst<Arc>* fst = Fst<Arc>::Read(strcmp(pfst_in_filename, "-") == 0 ? "" : pfst_in_filename);
	if (fst->Start() < 0) {
		cerr << "ERROR: FST is empty!" << endl;
		return 1;
	} else {
		VectorFst<Arc> fst_out;
		FstSubnetExtractor<Arc> ext(*fst);
		ext.Extract(start_state, end_state, &fst_out);
		fst_out.Write(strcmp(pfst_out_filename, "-") == 0 ? "" : pfst_out_filename);
	}
	delete fst;
}
