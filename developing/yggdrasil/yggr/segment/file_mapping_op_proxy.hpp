//file_mapping_op_proxy.hpp

/****************************************************************************
Copyright (c) 2014-2018 yggdrasil

author: yang xu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __YGGR_SEGMENT_FILE_MAPPING_OP_PROXY_HPP__
#define __YGGR_SEGMENT_FILE_MAPPING_OP_PROXY_HPP__

#ifndef YGGR_SEGMENT_OPERATOR_INCLUDE
#	error "include error file_mapping_op_proxy.hpp please include segment_op_proxy.hpp."
#endif // YGGR_SEGMENT_OPERATOR_INCLUDE

#include <yggr/base/yggrdef.h>
#include <yggr/nonable/noncreateable.hpp>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/exceptions.hpp>

#include <yggr/segment/segment_container.hpp>
#include <yggr/segment/segment_op.hpp>
#include <yggr/segment/segment_deleter.hpp>

#include <yggr/ppex/foo_params.hpp>
#include <yggr/ppex/symbols.hpp>

#include <yggr/base/exception.hpp>

#include <yggr/file_system/local_file_operator.hpp>


#ifdef _MSC_VER
#	pragma warning( push )
#	pragma warning (disable : 4624)
#endif //_MSC_VER

namespace yggr
{
namespace segment
{

template<typename Key>
class segment_op_proxy<Key, boost::interprocess::file_mapping>
		: private nonable::noncreateable
{
public:
	typedef Key key_type;
	typedef boost::interprocess::file_mapping seg_type;

private:
	typedef segment_deleter<key_type, seg_type> seg_deleter_type;

private:
	typedef segment_op_proxy this_type;

public:
    // swap create
    static void create(seg_type& seg)
	{
		seg_type tmp;
		seg.swap(tmp);
		return;
	}

#	define BOOST_PP_LOCAL_MACRO(__n__) \
	template< YGGR_PP_FOO_TYPES_DEF(__n__) > \
	static bool create(seg_type& seg, const key_type& name YGGR_PP_SYMBOL_IF(__n__, YGGR_PP_SYMBOL_COMMA) \
							YGGR_PP_FOO_PARAMS_DEF(__n__, YGGR_PP_FOO_CREF_PARAMS)) { \
			return prv_create(seg, name YGGR_PP_SYMBOL_IF(__n__, YGGR_PP_SYMBOL_COMMA) \
								YGGR_PP_FOO_PARAMS_OP(__n__, YGGR_PP_SYMBOL_COMMA)); }

#	define YGGR_PP_FOO_ARG_NAME() init_arg
#	define BOOST_PP_LOCAL_LIMITS ( 1, YGGR_PP_FOO_DEFAULT_PARAMS_LEN )
#	include BOOST_PP_LOCAL_ITERATE()
#	undef YGGR_PP_FOO_ARG_NAME

	static void remove(const key_type& name)
	{
		{ seg_deleter_type del(name); }
	}

	static bool is_empty(const seg_type& seg)
	{
		return this_type::size(seg) == 0;
	}

	static size_type size(const seg_type& seg)
	{
		file_system::local_file_operator_type::file_size_type fsize = 0;
		try
		{
			fsize = file_system::local_file_operator_type::file_size(seg.get_name());
		}
		catch(const compatibility::stl_exception&)
		{
			return 0;
		}
		return fsize;
	}

	static bool is_named_object(void)
	{
		return true;
	}

private:
	// swap_create
	static bool prv_create(seg_type& seg, const key_type& name, segment_op::create_only_type op, size_type size, u32 mode)
	{
		try
		{
			file_system::local_file_operator_type::create_file_of_binary(name, size);
		}
		catch(const boost::filesystem::filesystem_error&) {}
		catch(const compatibility::stl_exception&) {}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			remove(name);
		}

		try
		{
			file_system::local_file_operator_type::create_file_of_binary(name, size);
		}
		catch(const boost::filesystem::filesystem_error&) {}
		catch(const compatibility::stl_exception&){}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			exception::exception::throw_error(e.get_error_code(), e.what());
		}
		return false;
	}

	static bool prv_create(seg_type& seg, const key_type& name, segment_op::open_only_type op, u32 mode)
	{
		if(!file_system::local_file_operator_type::is_exists_path(name))
		{
			return false;
		}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			exception::exception::throw_error(e.get_error_code(), e.what());
			remove(name);
		}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			exception::exception::throw_error(e.get_error_code(), e.what());
		}

		return false;
	}

	template<typename Handler>
	static bool prv_create(seg_type& seg, const key_type& name, const Handler& err_fixer, segment_op::create_only_type op, size_type size, u32 mode)
	{
		try
		{
			file_system::local_file_operator_type::create_file_of_binary(name, size);
		}
		catch(const boost::filesystem::filesystem_error&) {}
		catch(const compatibility::stl_exception&){}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			remove(name);
			//err_fixer(e); //
		}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			exception::exception::throw_error(e.get_error_code(), e.what());
		}

		return false;
	}

	template<typename Handler>
	static bool prv_create(seg_type& seg, const key_type& name, const Handler& err_fixer, segment_op::open_only_type op, u32 mode)
	{
		if(!file_system::local_file_operator_type::is_exists_path(name))
		{
			return false;
		}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			remove(name);
			//err_fixer(e);
		}

		try
		{
			seg_type tmp(name.c_str(), boost::interprocess::mode_t(mode));
			seg.swap(tmp);
			return true;
		}
		catch(const boost::interprocess::interprocess_exception& e)
		{
			exception::exception::throw_error(e.get_error_code(), e.what());
		}
		return false;
	}
};

} // segment
} // namespace yggr

#ifdef _MSC_VER
#	pragma warning( pop )
#endif // _MSC_VER

#endif //__YGGR_SEGMENT_FILE_MAPPING_OP_PROXY_HPP__
