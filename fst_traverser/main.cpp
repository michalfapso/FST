#include <fst/fst.h>
//#include <fst/main.h>
#include <iostream>
#include <climits>
#include <float.h>

#include "path_pool.h"
#include "forward_traverser.h"
#include "features_generator.h"

#include "mlf.h"
#include "hypotheses.h"

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
	char * pterm = 0;
	char * pfeatures_out = 0;
	char * preference_mlf = 0;
	char * putterance_filename = 0;
//	OverlappedScoreType::Enum overlapped_score_method = OverlappedScoreType::logadd;

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
		else if (strcmp(argv[i], "--term") == 0) {
			i++;
			pterm = argv[i];
		}
		else if (strcmp(argv[i], "--features-out") == 0) {
			i++;
			pfeatures_out = argv[i];
		}
		else if (strcmp(argv[i], "--reference-mlf") == 0) {
			i++;
			preference_mlf = argv[i];
		}
		else if (strcmp(argv[i], "--utterance-filename") == 0) {
			i++;
			putterance_filename = argv[i];
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

	if (!psyms_filename || !pterm || !pfst_filename || !pfeatures_out || !preference_mlf || !putterance_filename) {
		cerr << "ERROR: missing arguments." << endl;
		help(argv[0]);
		return 1;
	}

	mlf::Mlf<ReferenceMlfRecord>             mlf_ref(preference_mlf);
	mlf::MlfRecords<ReferenceMlfRecord>*     recs_ref_all_terms = mlf_ref.GetFile(putterance_filename);
	mlf::MlfRecords<ReferenceMlfRecord>      recs_ref;
	if (!recs_ref_all_terms) {
		THROW("ERROR: File '"<<putterance_filename<<"' was not found in reference MLF '"<<preference_mlf<<"'");
	}
	// Keep only the given term in reference records
	foreach(ReferenceMlfRecord* rec, *recs_ref_all_terms) {
		assert(rec);
		if (rec->GetWord() == (string)pterm) {
			recs_ref.AddRecord(rec);
		}
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

		OverlappingPathGroupList<Path> paths;

		ForwardTraverser<Path> trav(fst, syms);
		trav.Traverse(&paths);

		//DBG("Generated paths:");
		//OverlappingPathGroup<Path>::PrintAllPathsInGroup(false);
		//OverlappingPathGroup<Path>::PrintBestPathInGroup(true);
		//paths.Print("_DETECTION_");
		//DBG("Generated paths end");

		PrintType pt = Path::GetPrintType();
		Path::SetPrintType(PRINT_PHONEMES_ONLY);
		{
			FeaturesGenerator<Path> features(pterm, pfeatures_out, recs_ref);
			features.Generate(paths);
		}
		Path::SetPrintType(pt);
	}
	delete fst;

	if (syms) {
		delete syms;
	}
}
