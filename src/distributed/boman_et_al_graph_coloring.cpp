// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "config.hpp"
#include "graph_types.hpp"
#include <boost/parallel/property_map.hpp>
#include <boost/graph/python/distributed/boman_et_al_graph_coloring.hpp>

namespace boost { namespace graph { namespace distributed { namespace python {

void export_boman_et_al_graph_coloring()
{
  using boost::python::arg;
  using boost::python::def;

  typedef property_map<Graph, vertex_index_t>::type VertexIndexMap;
  typedef vector_property_map<int, VertexIndexMap>
    VertexColorMap;

  def("boman_et_al_graph_coloring",
      &boost::graph::python::distributed::boman_et_al_graph_coloring<Graph>,
      (arg("graph"), 
       arg("color_map") = static_cast<VertexColorMap*>(0),
       arg("chunk_size") = 100));
}

} } } } // end namespace boost::graph::distributed::python
