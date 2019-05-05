#ifndef XML_MACROS_H_INCLUDED
#define XML_MACROS_H_INCLUDED

#include "conf_exception.h"

#define IF_NULL_THROW(a, what, which) if ( (a) == 0 ) throw conf_exception((what), (which))
#define IF_NULL_THROW_INVALID_XML(a, what) IF_NULL_THROW((a), (what), CONF_INVALID_XML)

#endif // !XML_MACROS_H_INCLUDED