#ifndef _READFST_H_
#define _READFST_H_

#include <iostream>
#include <fst/fst.h>
//#include <fst/vector-fst.h>

/* ReadFst()
 * reads fst without a header from the given stream
 */
template <class A>
void ReadFst(fst::Fst<A> **ppFst, std::ifstream &strm, const fst::FstHeader &hdr, const fst::FstReadOptions &opts)
{
	*ppFst = 0;
	fst::FstReadOptions ropts(opts);
	fst::FstRegister<A> *registr = fst::FstRegister<A>::GetRegister();
	const typename fst::FstRegister<A>::Reader reader =
	registr->GetReader(hdr.FstType());
	if (!reader) {
		cerr << "ERROR: Fst::Read: Unknown FST type \"" << hdr.FstType()
			<< "\" (arc type = \"" << hdr.ArcType()
			<< "\"): " << ropts.source << endl;
		return;
	}
	*ppFst = reader(strm, ropts);
}

bool readfstheader(const char *pFstFilename, fst::FstReadOptions *pOpts, fst::FstHeader *pHdr, std::ifstream *pStrm)
{
	pStrm->open(pFstFilename);
	if (!pStrm->good()) {
		cerr << "ERROR: Can not open FST file '"<<pFstFilename<<"'" << endl;
		return false;
	}
	pOpts->source = pFstFilename;
	if (!pHdr->Read(*pStrm, pFstFilename)) {
		return false;
	}
	pOpts->header = pHdr;
	//cout << "FstType: " << hdr.FstType() << endl;
	//cout << "ArcType: " << hdr.ArcType() << endl;
	return true;
}

#endif

