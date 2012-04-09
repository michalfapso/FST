#include <fst/fst.h>
//#include <fst/main.h>
#include <iostream>
#include <climits>
#include "fstcompose_spec_symbol.h"
#include "readfst.h"

using namespace std;
using namespace fst;

void help(char *pAppname)
{
	cerr << "Usage: " << pAppname << " [flags] in1.fst in2.fst [out.fst]" << endl;
	cerr << "Flags: " << endl;
	cerr << "  --spec-symbol: type = int, default = -1" << endl;
    cerr << "    Rho symbol number" << endl;
	cerr << "  --help:" << endl;
	cerr << "    Show this help" << endl;
}

int main(int argc, char **argv)
{
	char * pfstin1_filename = 0;
	char * pfstin2_filename = 0;
	char * pfstout_filename = 0;
	char * symbols_filename = 0;
	int spec_symbol = kNoLabel;
	bool do_sort = false;
	bool spec_symbol_in_first_fst = false;
	SpecSymbolType spec_symbol_type = UNSET;

	// PARSE COMMAND LINE ARGUMENTS
	if (argc <= 1) {
		help(argv[0]);
		return 1;
	}
	int i=1;
	while (i < argc)
	{
		if (strcmp(argv[i], "--spec-symbol") == 0) {
			i++;
			spec_symbol = atol(argv[i]);
		} 
		else if (strcmp(argv[i], "--sort") == 0) {
			do_sort = true;
		}
		else if (strcmp(argv[i], "--spec-symbol-in-first-fst") == 0) {
			spec_symbol_in_first_fst = true;
		}
		else if (strcmp(argv[i], "--symbols") == 0) {
			i++;
			symbols_filename = argv[i];
		} 
		else if (strcmp(argv[i], "--spec-symbol-type") == 0) {
			i++;
			if (strcmp(argv[i], "RHO") == 0) {
				spec_symbol_type = RHO;
			} else if (strcmp(argv[i], "SIGMA") == 0) {
				spec_symbol_type = SIGMA;
			} else {
				cerr << "ERROR: Invalid spec symbol type '"<<argv[i]<<"'" << endl;
				help(argv[0]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "--help") == 0) {
			help(argv[0]);
			return 0;
		}
		else if (!pfstin1_filename) {
			pfstin1_filename = argv[i];
		}
		else if (!pfstin2_filename) {
			pfstin2_filename = argv[i];
		}
		else if (!pfstout_filename) {
			pfstout_filename = argv[i];
		}
		else {
			cerr << "ERROR: Unknown argument '"<<argv[i]<<"'" << endl;
			help(argv[0]);
			return 1;
		}
		i++;
	}

	if (!pfstin1_filename || !pfstin2_filename || spec_symbol_type == UNSET || spec_symbol == kNoLabel) {
		cerr << "ERROR: both input FSTs, --spec-symbol, --spec-symbol-type have to be defined" << endl;
		help(argv[0]);
		return 1;
	}

	// LOAD FST HEADER
	FstReadOptions fst1_ropts;
	FstHeader fst1_hdr;
	ifstream fst1_strm;
	if (!readfstheader(pfstin1_filename, &fst1_ropts, &fst1_hdr, &fst1_strm))
		return 1;
	FstReadOptions fst2_ropts;
	FstHeader fst2_hdr;
	ifstream fst2_strm;
	if (!readfstheader(pfstin2_filename, &fst2_ropts, &fst2_hdr, &fst2_strm))
		return 1;
	
	if (fst1_hdr.ArcType() != fst2_hdr.ArcType()) {
		cerr << "ERROR: Arc type of FST1 and FST2 has to be the same. (" << fst1_hdr.ArcType() << " != " << fst2_hdr.ArcType() << ")" << endl;
		return 1;
	}

	// LOAD THE FST AND CALL MAIN()
	if (fst1_hdr.ArcType() == "log") {
		Fst<LogArc> *fst1;
		Fst<LogArc> *fst2;
		ReadFst(&fst1, fst1_strm, fst1_hdr, fst1_ropts);
		ReadFst(&fst2, fst2_strm, fst2_hdr, fst2_ropts);
		FstComposeSpecSymbolMain(fst1, fst2, spec_symbol, spec_symbol_type, do_sort, spec_symbol_in_first_fst, symbols_filename, pfstout_filename);
	}
	else if (fst1_hdr.ArcType() == "standard") {
		Fst<StdArc> *fst1;
		Fst<StdArc> *fst2;
		ReadFst(&fst1, fst1_strm, fst1_hdr, fst1_ropts);
		ReadFst(&fst2, fst2_strm, fst2_hdr, fst2_ropts);
		FstComposeSpecSymbolMain(fst1, fst2, spec_symbol, spec_symbol_type, do_sort, spec_symbol_in_first_fst, symbols_filename, pfstout_filename);
	}
	else {
		cerr << "ERROR: Unsupported arc type '"<<fst1_hdr.ArcType()<<"'" << endl;
		return 1;
	}
}
