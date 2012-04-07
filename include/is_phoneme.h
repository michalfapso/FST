#ifndef _IS_PHONEME_H_
#define _IS_PHONEME_H_

bool is_phoneme(const std::string& s) 
{
	return 
		s != "<eps>" && 
		s != "sil" &&
		s != "sp" &&
		!(s[0] == '_' && s[s.length()] == '_') &&
		s.substr(0,2) != "t=" &&
		s.substr(0,2) != "W=";
}

#endif
