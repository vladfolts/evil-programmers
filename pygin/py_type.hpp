#pragma once
#include "py_object.hpp"

typedef struct _typeobject PyTypeObject;

namespace py
{
	class object;

	class type: public object
	{
	public:
		MOVABLE(type);
		static auto type_name() { return "type"; }

		type() = default;
		type(const object& Object, const char* Name);
		explicit type(cast_guard, const object& Object);

		bool is_same(const type& Object) const;
	};

	type typeof(const object& Object);
}
