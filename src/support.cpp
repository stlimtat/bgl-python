// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/python.hpp>
#include <boost/graph/python/point2d.hpp>
#include <boost/graph/python/point3d.hpp>
#include <boost/graph/properties.hpp>

namespace boost { namespace graph { namespace python {

template<typename T, typename Key, typename Value>
Value get_subscript_key_value(const T& t, const Key& key)
{
  return t[key];
}

template<typename T, typename Key, typename Value>
void set_subscript_key_value(T& t, const Key& key, const Value& value)
{
  t[key] = value;
}

BOOST_PYTHON_MODULE(_support)
{
  using boost::python::class_;
  using boost::python::enum_;
  using boost::python::init;

  class_<point2d>("Point2D")
    .def(init<float, float>())
    .def("__getitem__", &get_subscript_key_value<point2d, std::size_t, float>)
    .def("__setitem__", &set_subscript_key_value<point2d, std::size_t, float>)
    .def("__len__", &point_traits<point2d>::dimensions)
    ;
  class_<point3d>("Point3D")
    .def(init<float, float, float>())
    .def("__getitem__", &get_subscript_key_value<point3d, std::size_t, float>)
    .def("__setitem__", &set_subscript_key_value<point3d, std::size_t, float>)
    .def("__len__", &point_traits<point2d>::dimensions)
    ;

  enum_<default_color_type>("Color")
    .value("white", color_traits<default_color_type>::white())
    .value("gray", color_traits<default_color_type>::gray())
    .value("black", color_traits<default_color_type>::black())
    ;
}

} } } // end namespace boost::graph::python
