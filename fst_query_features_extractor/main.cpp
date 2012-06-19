#include <fst/fst.h>
#include <fst/vector-fst.h>
//#include <fst/main.h>
#include <iostream>
#include <climits>
#include <float.h>

#include "path_pool.h"
#include "features_generator_path_query.h"
#include "fst_properties.h"
#include "nodes.h"
#include "path_terminator.h"
#include "path_generator_forward.h"

using namespace std;
using namespace fst;

void help(char *pAppname)
{
	cerr << "Usage: " << pAppname << " [flags] in.fst" << endl;
	cerr << "Flags: " << endl;
	cerr << "  --symbols: type = string, default = \"\"" << endl;
    cerr << "    Symbol table" << endl;
	cerr << "  --term: type = string, default = \"\"" << endl;
    cerr << "    Term identifier used in detections output" << endl;
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
	char * pfeatures_out = 0;

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
		else if (strcmp(argv[i], "--features-out") == 0) {
			i++;
			pfeatures_out = argv[i];
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

	if (!psyms_filename || !pfst_filename || !pfeatures_out) {
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
		Path::SetSymbols(syms);

		Nodes<Arc> nodes(*fst, syms);

		PathTerminator<Arc> path_terminator;
		PathGeneratorForward<Path> path_gen(*fst, nodes, path_terminator, PathGenerator<Path>::FINAL_NODE_ADD_PATH);
		OverlappingPathGroupList<Path> paths;
		path_gen.GeneratePaths(0, 0, &paths);

//		DBG("Generated paths:");
//		OverlappingPathGroup<Path>::PrintAllPathsInGroup(false);
//		OverlappingPathGroup<Path>::PrintBestPathInGroup(true);
//		paths.Print("_DETECTION_");
//		DBG("Generated paths end");

		PrintType pt = Path::GetPrintType();
		Path::SetPrintType(PRINT_PHONEMES_ONLY);
		{
			FeaturesGenerator_PathQuery<Path> features(pfeatures_out);
			features.PrintHeader();
			features.Generate(paths);
		}
		Path::SetPrintType(pt);
	}
	delete fst;

	if (syms) {
		delete syms;
	}
}
