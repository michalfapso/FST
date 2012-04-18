#ifndef _FOREACH_H_
#define _FOREACH_H_

#include <boost/foreach.hpp>

#define BOOST_FOREACH_PAIR(KEY, VALUE, COL) BOOST_FOREACH_PREAMBLE() if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_col) = BOOST_FOREACH_CONTAIN(COL)) {} else if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_cur) = BOOST_FOREACH_BEGIN(COL)) {} else if (boost::foreach_detail_::auto_any_t BOOST_FOREACH_ID(_foreach_end) = BOOST_FOREACH_END(COL)) {} else for (bool BOOST_FOREACH_ID(_foreach_continue) = true, BOOST_FOREACH_ID(_foreach_key_loop) = true; BOOST_FOREACH_ID(_foreach_continue) && !BOOST_FOREACH_DONE(COL); BOOST_FOREACH_ID(_foreach_continue) ? BOOST_FOREACH_NEXT(COL) : (void)0) if (boost::foreach_detail_::set_false(BOOST_FOREACH_ID(_foreach_continue))) {} else if (boost::foreach_detail_::set_false(BOOST_FOREACH_ID(_foreach_key_loop))) {} else for (KEY = BOOST_FOREACH_DEREF(COL).first; !BOOST_FOREACH_ID(_foreach_key_loop); BOOST_FOREACH_ID(_foreach_key_loop) = true) for (VALUE = BOOST_FOREACH_DEREF(COL).second; !BOOST_FOREACH_ID(_foreach_continue); BOOST_FOREACH_ID(_foreach_continue) = true) 

#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH
#define foreach_pair    BOOST_FOREACH_PAIR

#endif
