#include <fst/fst.h>
//#include <fst/main.h>
#include <iostream>
#include <climits>
#include <float.h>

#include "path_pool.h"
#include "forward_traverser.h"

using namespace std;
using namespace fst;

void help(char *pAppname)
{
	cerr << "Usage: " << pAppname << " [flags] in.fst" << endl;
	cerr << "Flags: " << endl;
	cerr << "  --overlapped-score: type = string, default = logadd" << endl;
    cerr << "    Method of merging overlapping hypotheses (best | logadd)" << endl;
	cerr << "  --acceptor: type = bool, default = false" << endl;
	cerr << "    Input in acceptor format" << endl;
	cerr << "  --isymbols: type = string, default = \"\"" << endl;
    cerr << "    Input label symbol table" << endl;
	cerr << "  --osymbols: type = string, default = \"\"" << endl;
    cerr << "    Output label symbol table" << endl;
	cerr << "  --threshold: type = float, default = FLT_MAX" << endl;
    cerr << "    Return paths with weight lower than the threshold" << endl;
	cerr << "  --nshortest: type = int64, default = LLONG_MAX" << endl;
    cerr << "    Return N-shortest paths" << endl;
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
	unsigned int min_phonemes_count_threshold = 0;
	char * pfst_filename = 0;
	char * psyms_filename = 0;
	float threshold = FLT_MAX;
	int64 nshortest = LLONG_MAX;
//	OverlappedScoreType::Enum overlapped_score_method = OverlappedScoreType::logadd;

	// PARSE COMMAND LINE ARGUMENTS
	if (argc <= 1) {
		help(argv[0]);
		return 1;
	}
	int i=1;
	while (i < argc)
	{
/*		if (strcmp(argv[i], "--overlapped-score") == 0) {
			i++;
			if (strcmp(argv[i], "best")) {
				overlapped_score_method = OverlappedScoreType::best;
			} else if (strcmp(argv[i], "logadd")) {
				overlapped_score_method = OverlappedScoreType::logadd;
			} else {
				cerr << "ERROR: Unknown overlapped score type: '" << argv[i] << "'" << endl;
				exit(1);
			}
		}
*/
		if (strcmp(argv[i], "--min-phonemes-count-threshold") == 0) {
			i++;
			min_phonemes_count_threshold = atol(argv[i]);
		} 
		else if (strcmp(argv[i], "--symbols") == 0) {
			i++;
			psyms_filename = argv[i];
		} 
		else if (strcmp(argv[i], "--threshold") == 0) {
			i++;
			threshold = atof(argv[i]);
		} 
		else if (strcmp(argv[i], "--nshortest") == 0) {
			i++;
			nshortest = atol(argv[i]);
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

	// LOAD SYMBOL TABLES
	if (!psyms_filename) {
		cerr << "ERROR: Symbols file has not been specified." << endl;
		exit(1);
	}
	SymbolTable* syms = NULL;
	syms = SymbolTable::ReadText(psyms_filename);

	typedef LogArc Arc;

	Fst<Arc>* fst = Fst<Arc>::Read(pfst_filename);
	if (fst->Start() < 0) {
		cerr << "Warning: FST is empty!" << endl;
	} else {
		if (!FstProperties::IsTopologicallySorted(*fst)) {
			cerr << "Warning: FST is not topologically sorted! ...sorting...";
			TopSort((MutableFst<Arc>*)fst);
			cerr << "done" << endl;
		}

		typedef PathAvgWeight<Arc> Path;

		PathPool<Path> pathpool(min_phonemes_count_threshold);

		ForwardTraverser<Path> trav(fst, syms, &pathpool);

		trav.Traverse();

//		pathpool.Print();
	}
	delete fst;

	if (syms) {
		delete syms;
	}
}
