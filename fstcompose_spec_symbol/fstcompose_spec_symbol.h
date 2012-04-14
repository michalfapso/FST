#ifndef _FSTCOMPOSERHO_H_
#define _FSTCOMPOSERHO_H_

#include <fst/fst.h>
#include <fst/compose.h>
#include <fst/vector-fst.h>
#include <fst/arcsort.h>
#include <list>
#include <vector>
#include <algorithm>

#define DBG(msg) cout << __FILE__ << ":" << __LINE__ << ">\t" << msg << endl;

using namespace fst;
using namespace std;

enum SpecSymbolType {
	UNSET,
	RHO,
	SIGMA
};

/* FstComposeSpecSymbolMain()
 * main
 */
template <class Arc>
void FstComposeSpecSymbolMain(
		Fst<Arc> *pFst1,
		Fst<Arc> *pFst2,
		typename Arc::Label specSymbol,
		SpecSymbolType specSymbolType,
		bool doSort,
		bool specSymbolInFirstFst,
		const char* pSymbolsFilename,
		const char* pFstOutFilename)
{
	typedef RhoMatcher< SortedMatcher<Fst<Arc> > > RM;
	typedef SigmaMatcher< SortedMatcher<Fst<Arc> > > SM;

	MutableFst<Arc> *ofst1 = 0;
	if (pFst1->Properties(kMutable, false)) {
		ofst1 = down_cast<MutableFst<Arc> *>(pFst1);
	} else {
		ofst1 = new VectorFst<Arc>(*pFst1);
		delete pFst1;
	}

	MutableFst<Arc> *ofst2 = 0;
	if (pFst2->Properties(kMutable, false)) {
		ofst2 = down_cast<MutableFst<Arc> *>(pFst2);
	} else {
		ofst2 = new VectorFst<Arc>(*pFst2);
		delete pFst2;
	}

	if (doSort) {
		OLabelCompare<Arc> ocomp;
		ILabelCompare<Arc> icomp;
		ArcSort(ofst1, ocomp);
		ArcSort(ofst2, icomp);
		//ofst1->Write(string(pFstOutFilename)+"_sorted1");
		//ofst2->Write(string(pFstOutFilename)+"_sorted2");
	}

	if (pSymbolsFilename) {
		SymbolTable *syms = SymbolTable::ReadText(pSymbolsFilename);
		ofst1->SetInputSymbols(syms);
		ofst1->SetOutputSymbols(syms);
		ofst2->SetInputSymbols(syms);
		ofst2->SetOutputSymbols(syms);
		DBG("symbols:"<<pSymbolsFilename);
		DBG("symbols:"<<syms->NumSymbols());
		DBG("symbols:"<<ofst1->InputSymbols()->NumSymbols());
	}

	VectorFst<Arc> fstout;
	MatchType match_type_1 = MATCH_NONE;
	MatchType match_type_2 = MATCH_NONE;
	MatcherRewriteMode matcher_rw_mode_1 = MATCHER_REWRITE_AUTO;
	MatcherRewriteMode matcher_rw_mode_2 = MATCHER_REWRITE_AUTO;

	if (specSymbolInFirstFst) {
		match_type_1      = MATCH_OUTPUT;
		matcher_rw_mode_1 = MATCHER_REWRITE_NEVER;
		matcher_rw_mode_2 = MATCHER_REWRITE_NEVER;
	} else {
		match_type_2      = MATCH_INPUT;
		matcher_rw_mode_2 = MATCHER_REWRITE_ALWAYS;
	}

	if (specSymbolType == RHO) {
		ComposeFstOptions<Arc, RM> opts;
		opts.gc_limit = 0;
		opts.matcher1 = new RM(*ofst1, match_type_1, specSymbol, matcher_rw_mode_1);
		opts.matcher2 = new RM(*ofst2, match_type_2, specSymbol, matcher_rw_mode_2);
		fstout = ComposeFst<Arc>(*ofst1, *ofst2, opts);
	} else if (specSymbolType == SIGMA) {
		ComposeFstOptions<Arc, SM> opts;
		opts.gc_limit = 0;
		opts.matcher1 = new SM(*ofst1, match_type_1, specSymbol, matcher_rw_mode_1);
		opts.matcher2 = new SM(*ofst2, match_type_2, specSymbol, matcher_rw_mode_2);
		fstout = ComposeFst<Arc>(*ofst1, *ofst2, opts);
	}

//	if (specSymbolInFirstFst) {
//		DBG("specSymbolInFirstFst");
//		ComposeFstOptions<Arc, SM> opts;
//		opts.gc_limit = 0;
//		opts.matcher1 = new SM(*ofst1, MATCH_OUTPUT, specSymbol, MATCHER_REWRITE_NEVER);
//		opts.matcher2 = new SM(*ofst2, MATCH_NONE, specSymbol, MATCHER_REWRITE_NEVER);
//		fstout = ComposeFst<Arc>(*ofst1, *ofst2, opts);
//	} else {
//		ComposeFstOptions<Arc, RM> opts;
//		opts.gc_limit = 0;
//		opts.matcher1 = new RM(*ofst1, MATCH_NONE, specSymbol);
//		opts.matcher2 = new RM(*ofst2, MATCH_INPUT, specSymbol, MATCHER_REWRITE_ALWAYS);
//		fstout = ComposeFst<Arc>(*ofst1, *ofst2, opts);
//	}

//	if (strcmp(pFstOutFilename, "-") != 0) {
//		fstout.Write((string)pFstOutFilename + ".notconnected");
//	}
	Connect(&fstout);
	fstout.SetInputSymbols(NULL);
	fstout.SetOutputSymbols(NULL);
	fstout.Write(strcmp(pFstOutFilename, "-") == 0 ? "" : pFstOutFilename);
}

#endif
