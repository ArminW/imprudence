/** 
 * @file llsdutil.cpp
 * @author Phoenix
 * @date 2006-05-24
 * @brief Implementation of classes, functions, etc, for using structured data.
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "linden_common.h"

#include "llsdutil.h"

#if LL_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	include <winsock2.h>	// for htonl
#elif LL_LINUX || LL_SOLARIS
#	include <netinet/in.h>
#elif LL_DARWIN
#	include <arpa/inet.h>
#endif

#include "llsdserialize.h"

// U32
LLSD ll_sd_from_U32(const U32 val)
{
	std::vector<U8> v;
	U32 net_order = htonl(val);

	v.resize(4);
	memcpy(&(v[0]), &net_order, 4);		/* Flawfinder: ignore */

	return LLSD(v);
}

U32 ll_U32_from_sd(const LLSD& sd)
{
	U32 ret;
	std::vector<U8> v = sd.asBinary();
	if (v.size() < 4)
	{
		return 0;
	}
	memcpy(&ret, &(v[0]), 4);		/* Flawfinder: ignore */
	ret = ntohl(ret);
	return ret;
}

//U64
LLSD ll_sd_from_U64(const U64 val)
{
	std::vector<U8> v;
	U32 high, low;

	high = (U32)(val >> 32);
	low = (U32)val;
	high = htonl(high);
	low = htonl(low);

	v.resize(8);
	memcpy(&(v[0]), &high, 4);		/* Flawfinder: ignore */
	memcpy(&(v[4]), &low, 4);		/* Flawfinder: ignore */

	return LLSD(v);
}

U64 ll_U64_from_sd(const LLSD& sd)
{
	U32 high, low;
	std::vector<U8> v = sd.asBinary();

	if (v.size() < 8)
	{
		return 0;
	}

	memcpy(&high, &(v[0]), 4);		/* Flawfinder: ignore */
	memcpy(&low, &(v[4]), 4);		/* Flawfinder: ignore */
	high = ntohl(high);
	low = ntohl(low);

	return ((U64)high) << 32 | low;
}

// IP Address (stored in net order in a U32, so don't need swizzling)
LLSD ll_sd_from_ipaddr(const U32 val)
{
	std::vector<U8> v;

	v.resize(4);
	memcpy(&(v[0]), &val, 4);		/* Flawfinder: ignore */

	return LLSD(v);
}

U32 ll_ipaddr_from_sd(const LLSD& sd)
{
	U32 ret;
	std::vector<U8> v = sd.asBinary();
	if (v.size() < 4)
	{
		return 0;
	}
	memcpy(&ret, &(v[0]), 4);		/* Flawfinder: ignore */
	return ret;
}

// Converts an LLSD binary to an LLSD string
LLSD ll_string_from_binary(const LLSD& sd)
{
	std::vector<U8> value = sd.asBinary();
	std::string str;
	str.resize(value.size());
	memcpy(&str[0], &value[0], value.size());
	return str;
}

// Converts an LLSD string to an LLSD binary
LLSD ll_binary_from_string(const LLSD& sd)
{
	std::vector<U8> binary_value;

	std::string string_value = sd.asString();
	for (std::string::iterator iter = string_value.begin();
		 iter != string_value.end(); ++iter)
	{
		binary_value.push_back(*iter);
	}

	binary_value.push_back('\0');

	return binary_value;
}

char* ll_print_sd(const LLSD& sd)
{
	const U32 bufferSize = 10 * 1024;
	static char buffer[bufferSize];
	std::ostringstream stream;
	//stream.rdbuf()->pubsetbuf(buffer, bufferSize);
	stream << LLSDOStreamer<LLSDXMLFormatter>(sd);
	stream << std::ends;
	strncpy(buffer, stream.str().c_str(), bufferSize);
	buffer[bufferSize - 1] = '\0';
	return buffer;
}

char* ll_pretty_print_sd(const LLSD& sd)
{
	const U32 bufferSize = 10 * 1024;
	static char buffer[bufferSize];
	std::ostringstream stream;
	//stream.rdbuf()->pubsetbuf(buffer, bufferSize);
	stream << LLSDOStreamer<LLSDXMLFormatter>(sd, LLSDFormatter::OPTIONS_PRETTY);
	stream << std::ends;
	strncpy(buffer, stream.str().c_str(), bufferSize);
	buffer[bufferSize - 1] = '\0';
	return buffer;
}

//compares the structure of an LLSD to a template LLSD and stores the
//"valid" values in a 3rd LLSD.  Default values pulled from the template
//if the tested LLSD does not contain the key/value pair.
//Excess values in the test LLSD are ignored in the resultant_llsd.
//If the llsd to test has a specific key to a map and the values
//are not of the same type, false is returned or if the LLSDs are not
//of the same value.  Ordering of arrays matters
//Otherwise, returns true
BOOL compare_llsd_with_template(
	const LLSD& llsd_to_test,
	const LLSD& template_llsd,
	LLSD& resultant_llsd)
{
	if (
		llsd_to_test.isUndefined() &&
		template_llsd.isDefined() )
	{
		resultant_llsd = template_llsd;
		return TRUE;
	}
	else if ( llsd_to_test.type() != template_llsd.type() )
	{
		resultant_llsd = LLSD();
		return FALSE;
	}

	if ( llsd_to_test.isArray() )
	{
		//they are both arrays
		//we loop over all the items in the template
		//verifying that the to_test has a subset (in the same order)
		//any shortcoming in the testing_llsd are just taken
		//to be the rest of the template
		LLSD data;
		LLSD::array_const_iterator test_iter;
		LLSD::array_const_iterator template_iter;

		resultant_llsd = LLSD::emptyArray();
		test_iter = llsd_to_test.beginArray();

		for (
			template_iter = template_llsd.beginArray();
			(template_iter != template_llsd.endArray() &&
			 test_iter != llsd_to_test.endArray());
			++template_iter)
		{
			if ( !compare_llsd_with_template(
					 *test_iter,
					 *template_iter,
					 data) )
			{
				resultant_llsd = LLSD();
				return FALSE;
			}
			else
			{
				resultant_llsd.append(data);
			}

			++test_iter;
		}

		//so either the test or the template ended
		//we do another loop now to the end of the template
		//grabbing the default values
		for (;
			 template_iter != template_llsd.endArray();
			 ++template_iter)
		{
			resultant_llsd.append(*template_iter);
		}
	}
	else if ( llsd_to_test.isMap() )
	{
		//now we loop over the keys of the two maps
		//any excess is taken from the template
		//excess is ignored in the test
		LLSD value;
		LLSD::map_const_iterator template_iter;

		resultant_llsd = LLSD::emptyMap();
		for (
			template_iter = template_llsd.beginMap();
			template_iter != template_llsd.endMap();
			++template_iter)
		{
			if ( llsd_to_test.has(template_iter->first) )
			{
				//the test LLSD has the same key
				if ( !compare_llsd_with_template(
						 llsd_to_test[template_iter->first],
						 template_iter->second,
						 value) )
				{
					resultant_llsd = LLSD();
					return FALSE;
				}
				else
				{
					resultant_llsd[template_iter->first] = value;
				}
			}
			else
			{
				//test llsd doesn't have it...take the
				//template as default value
				resultant_llsd[template_iter->first] =
					template_iter->second;
			}
		}
	}
	else
	{
		//of same type...take the test llsd's value
		resultant_llsd = llsd_to_test;
	}


	return TRUE;
}

bool llsd_equals(const LLSD& lhs, const LLSD& rhs)
{
    // We're comparing strict equality of LLSD representation rather than
    // performing any conversions. So if the types aren't equal, the LLSD
    // values aren't equal.
    if (lhs.type() != rhs.type())
    {
        return false;
    }

    // Here we know both types are equal. Now compare values.
    switch (lhs.type())
    {
    case LLSD::TypeUndefined:
        // Both are TypeUndefined. There's nothing more to know.
        return true;

#define COMPARE_SCALAR(type)                                    \
    case LLSD::Type##type:                                      \
        /* LLSD::URI has operator!=() but not operator==() */   \
        /* rely on the optimizer for all others */              \
        return (! (lhs.as##type() != rhs.as##type()))

    COMPARE_SCALAR(Boolean);
    COMPARE_SCALAR(Integer);
    // The usual caveats about comparing floating-point numbers apply. This is
    // only useful when we expect identical bit representation for a given
    // Real value, e.g. for integer-valued Reals.
    COMPARE_SCALAR(Real);
    COMPARE_SCALAR(String);
    COMPARE_SCALAR(UUID);
    COMPARE_SCALAR(Date);
    COMPARE_SCALAR(URI);
    COMPARE_SCALAR(Binary);

#undef COMPARE_SCALAR

    case LLSD::TypeArray:
    {
        LLSD::array_const_iterator
            lai(lhs.beginArray()), laend(lhs.endArray()),
            rai(rhs.beginArray()), raend(rhs.endArray());
        // Compare array elements, walking the two arrays in parallel.
        for ( ; lai != laend && rai != raend; ++lai, ++rai)
        {
            // If any one array element is unequal, the arrays are unequal.
            if (! llsd_equals(*lai, *rai))
                return false;
        }
        // Here we've reached the end of one or the other array. They're equal
        // only if they're BOTH at end: that is, if they have equal length too.
        return (lai == laend && rai == raend);
    }

    case LLSD::TypeMap:
    {
        // Build a set of all rhs keys.
        std::set<LLSD::String> rhskeys;
        for (LLSD::map_const_iterator rmi(rhs.beginMap()), rmend(rhs.endMap());
             rmi != rmend; ++rmi)
        {
            rhskeys.insert(rmi->first);
        }
        // Now walk all the lhs keys.
        for (LLSD::map_const_iterator lmi(lhs.beginMap()), lmend(lhs.endMap());
             lmi != lmend; ++lmi)
        {
            // Try to erase this lhs key from the set of rhs keys. If rhs has
            // no such key, the maps are unequal. erase(key) returns count of
            // items erased.
            if (rhskeys.erase(lmi->first) != 1)
                return false;
            // Both maps have the current key. Compare values.
            if (! llsd_equals(lmi->second, rhs[lmi->first]))
                return false;
        }
        // We've now established that all the lhs keys have equal values in
        // both maps. The maps are equal unless rhs contains a superset of
        // those keys.
        return rhskeys.empty();
    }

    default:
        // We expect that every possible type() value is specifically handled
        // above. Failing to extend this switch to support a new LLSD type is
        // an error that must be brought to the coder's attention.
        LL_ERRS("llsd_equals") << "llsd_equals(" << lhs << ", " << rhs << "): "
            "unknown type " << lhs.type() << LL_ENDL;
        return false;               // pacify the compiler
    }
}
