#include "headers.hpp"

#include "py_type.hpp"

#include "python.hpp"

namespace py
{
	type::type(cast_guard, const object& Object):
		object(PyType_Check(Object.get())? Object : object(invoke(PyObject_Type, Object.get())))
	{
	}

	type::type(const object& Object, const char* Name):
		type(cast_guard{}, Object.get_attribute(Name))
	{
	}

	bool type::is_same(const type& Object) const
	{
		return get()->ob_type == Object.get()->ob_type;
	}

	type typeof(const object& Object)
	{
		return type(cast_guard{}, Object);
	}
}
