// Copyright 2004-5 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_BASIC_GRAPH_HPP
#define BOOST_GRAPH_BASIC_GRAPH_HPP

#include <boost/python.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/python/point2d.hpp>
#include <boost/graph/python/iterator.hpp>
#include <boost/graph/python/resizable_property_map.hpp>

namespace boost { namespace graph { namespace python {

template<typename DirectedS>
class basic_graph;

} } } 

namespace boost { namespace python {

template<typename DirectedS> 
class has_back_reference<graph::python::basic_graph<DirectedS> > { 
public:
  typedef mpl::true_ type;
};

} } 

namespace boost { namespace graph { namespace python {

template<typename T, typename DirectedS>
struct basic_descriptor
{
  typedef basic_graph<DirectedS> graph_type;

  basic_descriptor() : graph(0) {}
  basic_descriptor(T base, const basic_graph<DirectedS>* graph) 
    : base(base), graph(graph) { }

  operator T() const { return base; }

  struct create 
  {
    typedef basic_descriptor result_type;
    
    create() : graph(0) { }

    explicit create(const basic_graph<DirectedS>* graph) : graph(graph) { }

    basic_descriptor operator()(T base) const 
    { 
      return basic_descriptor<T, DirectedS>(base, graph); 
    }

    const basic_graph<DirectedS>* graph;
  };

  T base;
  const basic_graph<DirectedS>* graph;
};

template<typename T, typename DirectedS>
inline bool 
operator==(const basic_descriptor<T, DirectedS>& u,
           const basic_descriptor<T, DirectedS>& v)
{ return u.base == v.base; }

template<typename T, typename DirectedS>
inline bool 
operator!=(const basic_descriptor<T, DirectedS>& u,
           const basic_descriptor<T, DirectedS>& v)
{ return u.base != v.base; }

template<typename Key, typename IndexMap>
struct basic_index_map
{
  typedef Key                                                  key_type;
  typedef typename property_traits<IndexMap>::value_type value_type;
  typedef typename property_traits<IndexMap>::reference  reference;
  typedef typename property_traits<IndexMap>::category   category;

  basic_index_map(const IndexMap& id = IndexMap())
    : id(id) { }

  value_type operator[](const key_type& key) const
  {
    return get(id, key.base);
  }

  IndexMap id;
};

template<typename Key, typename IndexMap>
inline typename basic_index_map<Key, IndexMap>::value_type
get(const basic_index_map<Key, IndexMap>& pm, 
    typename basic_index_map<Key, IndexMap>::key_type const& key)
{ return get(pm.id, key.base); }

struct stored_minstd_rand
{
  stored_minstd_rand(int seed = 1) : gen(seed) { }

  minstd_rand gen;
};

template<typename DirectedS>
class basic_graph
  : private noncopyable, 
    public stored_minstd_rand,
    public adjacency_list<listS, listS, DirectedS,
                          property<vertex_index_t, std::size_t>,
                          property<edge_index_t, std::size_t> >
{
  typedef adjacency_list<listS, listS, DirectedS,
                         property<vertex_index_t, std::size_t>,
                         property<edge_index_t, std::size_t> > inherited;
  typedef graph_traits<inherited> traits;

  typedef typename traits::vertex_iterator           base_vertex_iterator;
  typedef typename traits::edge_iterator             base_edge_iterator;
  typedef typename traits::out_edge_iterator         base_out_edge_iterator;
  typedef typename traits::in_edge_iterator          base_in_edge_iterator;
  typedef typename traits::adjacency_iterator        base_adjacency_iterator;
  typedef typename property_map<inherited, vertex_index_t>::const_type
                                                     base_vertex_index_map;
  typedef typename property_map<inherited, edge_index_t>::const_type
                                                     base_edge_index_map;

 public:
  typedef typename traits::vertex_descriptor         base_vertex_descriptor;
  typedef typename traits::edge_descriptor           base_edge_descriptor;

  typedef basic_descriptor<base_vertex_descriptor, DirectedS>
                                                     Vertex;
  typedef Vertex                                     vertex_descriptor;
  typedef basic_descriptor<base_edge_descriptor, DirectedS>
                                                     Edge;
  typedef Edge                                       edge_descriptor;
  typedef basic_index_map<Vertex, base_vertex_index_map>
                                                     VertexIndexMap;
  typedef basic_index_map<Edge, base_edge_index_map> EdgeIndexMap;
  typedef std::size_t                                vertices_size_type;
  typedef std::size_t                                edges_size_type;
  typedef std::size_t                                degree_size_type;
  typedef typename traits::directed_category         directed_category;
  typedef typename traits::edge_parallel_category    edge_parallel_category;
  typedef typename traits::traversal_category        traversal_category; 
  typedef transform_iterator<typename Vertex::create, base_vertex_iterator>
                                                     vertex_iterator;
  typedef transform_iterator<typename Edge::create, base_edge_iterator>
                                                     edge_iterator;
  typedef transform_iterator<typename Edge::create, base_out_edge_iterator>
                                                     out_edge_iterator;
  typedef transform_iterator<typename Edge::create, base_in_edge_iterator>
                                                     in_edge_iterator;
  typedef transform_iterator<typename Vertex::create, base_adjacency_iterator>
                                                     adjacency_iterator;

  static Vertex null_vertex()
  {
    return Vertex(traits::null_vertex(), 0);
  }

  basic_graph(PyObject* self);

  basic_graph(PyObject* self, 
              boost::python::object, 
              const std::string& name_map = std::string());

  ~basic_graph()
  {
    for (std::list<resizable_property_map*>::iterator v = vertex_maps.begin();
         v != vertex_maps.end(); 
 	 ++v) {
      delete *v;
    }
    for (std::list<resizable_property_map*>::iterator e = edge_maps.begin();
         e != edge_maps.end(); 
         ++e) {
      delete *e;
    }
  }

  bool is_directed() const
  { return is_convertible<directed_category, directed_tag>::value; }

  Vertex add_vertex();
  void clear_vertex(Vertex vertex);
  void remove_vertex(Vertex vertex);
  
  Edge add_edge(Vertex u, Vertex v);
  void remove_edge(Edge edge);

  std::pair<Edge, bool> edge(Vertex u, Vertex v) const;
  
  // Property map handling
  void register_vertex_map(std::auto_ptr<resizable_property_map> map)
  { 
    vertex_maps.push_back(map.get()); 
    map.release();
  }

  void register_edge_map(std::auto_ptr<resizable_property_map> map)
  { 
    edge_maps.push_back(map.get()); 
    map.release();
  }

  inherited&       base()       { return *this; }
  const inherited& base() const { return *this; }

  boost::python::dict& vertex_properties() { return vertex_properties_; }
  boost::python::dict& edge_properties() { return edge_properties_; }
  
  const boost::python::dict& vertex_properties() const 
  { return vertex_properties_; }

  const boost::python::dict& edge_properties() const 
  { return edge_properties_; }

  boost::python::object vertex_type_;
  boost::python::object edge_type_;

  static boost::python::handle<> py_null_vertex;
  vector_property_map<boost::python::object, VertexIndexMap> vertex_objects;
  vector_property_map<boost::python::object, EdgeIndexMap>   edge_objects;

  static boost::python::object pyconstruct() { return graph_type(); }

  template<typename InputIterator>
  static boost::python::object 
  pyconstruct(InputIterator first, InputIterator last, vertices_size_type n)
  {
    typedef basic_graph<DirectedS> self_type;

    // Build through Python, so that self-references work
    boost::python::object result = pyconstruct();
    self_type& g = boost::python::extract<self_type&>(result)();

    // Initialize vertices
    std::vector<Vertex> vertices;
    for (vertices_size_type i = 0; i < n; ++i)
      vertices.push_back(g.add_vertex());

    // Add edges
    for (; first != last; ++first)
      g.add_edge(vertices[first->first], vertices[first->second]);

    return result;
  }

  // The Python type corresponding to this instance of basic_graph<>
  static boost::python::object graph_type;

protected:
  void initialize();
  
private:
  // The Python object instance corresponding to this graph instance.
  PyObject* self;

  /* Mapping from indices to descriptors, which allows us to provide
     edge and vertex removal while retaining O(1) lookup for external
     property maps. */
  std::vector<vertex_descriptor> index_to_vertex;
  std::vector<edge_descriptor>   index_to_edge;

  /* The lists of property maps "attached" to the graph, which will be
     updated to deal with vertex and edge removals. */
  std::list<resizable_property_map*> vertex_maps;
  std::list<resizable_property_map*> edge_maps;

  boost::python::dict vertex_properties_;
  boost::python::dict edge_properties_;
};

// Vertex List Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::vertex_iterator, 
                 typename basic_graph<DirectedS>::vertex_iterator>
vertices(const basic_graph<DirectedS>& g)
{ 
  typedef typename graph_traits<basic_graph<DirectedS> >::vertex_iterator
    vertex_iterator;
  typedef typename graph_traits<basic_graph<DirectedS> >::vertex_descriptor
    Vertex;

  return std::make_pair(vertex_iterator(vertices(g.base()).first, 
                                        typename Vertex::create(&g)),
                        vertex_iterator(vertices(g.base()).second, 
                                        typename Vertex::create(&g)));
}

template<typename DirectedS>
inline std::size_t num_vertices(const basic_graph<DirectedS>& g) 
{ return num_vertices(g.base()); }

// Edge List Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::edge_iterator, 
                 typename basic_graph<DirectedS>::edge_iterator>
edges(const basic_graph<DirectedS>& g)
{
  typedef typename graph_traits<basic_graph<DirectedS> >::edge_iterator 
    edge_iterator;
  typedef typename graph_traits<basic_graph<DirectedS> >::edge_descriptor Edge;

  return std::make_pair(edge_iterator(edges(g.base()).first, 
                                        typename Edge::create(&g)),
                        edge_iterator(edges(g.base()).second, 
                                        typename Edge::create(&g)));  
}

template<typename DirectedS>
inline std::size_t num_edges(const basic_graph<DirectedS>& g) 
{ return num_edges(g.base()); }

// Incidence Graph concept
template<typename DirectedS>
inline typename basic_graph<DirectedS>::vertex_descriptor 
source(typename basic_graph<DirectedS>::edge_descriptor e, 
       const basic_graph<DirectedS>& g)
{ 
  typedef typename graph_traits<basic_graph<DirectedS> >::vertex_descriptor
    Vertex;
  return Vertex(source(e.base, g.base()), &g);
}

template<typename DirectedS>
inline typename basic_graph<DirectedS>::vertex_descriptor 
target(typename basic_graph<DirectedS>::edge_descriptor e, 
       const basic_graph<DirectedS>& g)
{ 
  typedef typename graph_traits<basic_graph<DirectedS> >::vertex_descriptor
    Vertex;
  return Vertex(target(e.base, g.base()), &g);
}

template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::out_edge_iterator, 
                 typename basic_graph<DirectedS>::out_edge_iterator>
out_edges(typename basic_graph<DirectedS>::vertex_descriptor u, 
          const basic_graph<DirectedS>& g)
{ 
  typedef typename graph_traits<basic_graph<DirectedS> >::out_edge_iterator
    out_edge_iterator;
  typedef typename graph_traits<basic_graph<DirectedS> >::edge_descriptor Edge;
  return std::make_pair(out_edge_iterator(out_edges(u.base, g.base()).first, 
                                          typename Edge::create(&g)),
                        out_edge_iterator(out_edges(u.base, g.base()).second,
                                          typename Edge::create(&g)));
}

template<typename DirectedS>
inline std::size_t 
out_degree(typename basic_graph<DirectedS>::vertex_descriptor u, 
           const basic_graph<DirectedS>& g)
{ return out_degree(u.base, g.base()); }

// Bidirectional Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::in_edge_iterator, 
                 typename basic_graph<DirectedS>::in_edge_iterator>
in_edges(typename basic_graph<DirectedS>::vertex_descriptor u, 
         const basic_graph<DirectedS>& g)
{ 
  typedef typename graph_traits<basic_graph<DirectedS> >::in_edge_iterator
    in_edge_iterator;
  typedef typename graph_traits<basic_graph<DirectedS> >::edge_descriptor Edge;
  return std::make_pair(in_edge_iterator(in_edges(u.base, g.base()).first, 
                                         typename Edge::create(&g)),
                        in_edge_iterator(in_edges(u.base, g.base()).second,
                                         typename Edge::create(&g)));
}

template<typename DirectedS>
inline std::size_t 
in_degree(typename basic_graph<DirectedS>::vertex_descriptor u, 
          const basic_graph<DirectedS>& g)
{ return in_degree(u.base, g.base()); }

// Adjacency Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::adjacency_iterator, 
                 typename basic_graph<DirectedS>::adjacency_iterator>
adjacent_vertices(typename basic_graph<DirectedS>::vertex_descriptor u, 
                  const basic_graph<DirectedS>& g)
{ 
  typedef typename graph_traits<basic_graph<DirectedS> >::adjacency_iterator
    adjacency_iterator;
  typedef typename graph_traits<basic_graph<DirectedS> >::vertex_descriptor
    Vertex;

  return std::make_pair
           (adjacency_iterator(adjacent_vertices(u.base, g.base()).first, 
                               typename Vertex::create(&g)),
            adjacency_iterator(adjacent_vertices(u.base, g.base()).second, 
                               typename Vertex::create(&g)));
}

// Mutable basic_graph<DirectedS> concept
template<typename DirectedS>
inline typename basic_graph<DirectedS>::vertex_descriptor 
add_vertex(basic_graph<DirectedS>& g)
{ return g.add_vertex(); }

template<typename DirectedS>
inline void
clear_vertex(typename basic_graph<DirectedS>::vertex_descriptor v, 
             basic_graph<DirectedS>& g)
{ return g.clear_vertex(v); }

template<typename DirectedS>
inline void
remove_vertex(typename basic_graph<DirectedS>::vertex_descriptor v, 
              basic_graph<DirectedS>& g)
{ return g.remove_vertex(v); }

template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::edge_descriptor, bool>
add_edge(typename basic_graph<DirectedS>::vertex_descriptor u, 
         typename basic_graph<DirectedS>::vertex_descriptor v, 
         basic_graph<DirectedS>& g)
{ return std::make_pair(g.add_edge(u, v), true); }

// Adjacency list extras
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::edge_descriptor, bool>
edge(typename basic_graph<DirectedS>::vertex_descriptor u, 
     typename basic_graph<DirectedS>::vertex_descriptor v, 
     const basic_graph<DirectedS>& g)
{ 
  return g.edge(u, v);
}

template<typename DirectedS>
inline void
remove_edge(typename basic_graph<DirectedS>::edge_descriptor e, 
            basic_graph<DirectedS>& g)
{ return g.remove_edge(e); }

template<typename DirectedS, typename Predicate>
struct adapt_edge_predicate
{
  explicit adapt_edge_predicate(const Predicate& pred,
                                basic_graph<DirectedS>* graph) 
    : pred(pred), graph(graph) { }

  bool operator()(typename basic_graph<DirectedS>::base_edge_descriptor e)
  {
    typedef typename basic_graph<DirectedS>::edge_descriptor Edge;
    return pred(Edge(e, graph));
  }

  Predicate pred;
  basic_graph<DirectedS>* graph;
};

template<typename DirectedS, typename Predicate>
void 
remove_out_edge_if(typename basic_graph<DirectedS>::vertex_descriptor u,
                   Predicate pred, basic_graph<DirectedS>& g)
{
  remove_out_edge_if(u.base, 
                     adapt_edge_predicate<DirectedS, Predicate>(pred, &g),
                     g.base());
}

template<typename DirectedS>
void export_basic_graph(const char* name);

template<typename DirectedS>
typename basic_graph<DirectedS>::VertexIndexMap
get(vertex_index_t, const basic_graph<DirectedS>& g)
{ return get(vertex_index, g.base()); }

template<typename DirectedS>
typename basic_graph<DirectedS>::VertexIndexMap
get(vertex_index_t, basic_graph<DirectedS>& g)
{ return get(vertex_index, const_cast<const basic_graph<DirectedS>&>(g).base()); }

template<typename DirectedS>
typename basic_graph<DirectedS>::EdgeIndexMap
get(edge_index_t, const basic_graph<DirectedS>& g)
{ return get(edge_index, g.base()); }

template<typename DirectedS>
typename basic_graph<DirectedS>::EdgeIndexMap
get(edge_index_t, basic_graph<DirectedS>& g)
{ return get(edge_index, const_cast<const basic_graph<DirectedS>&>(g).base()); }

template<typename DirectedS>
struct graph_pickle_suite : boost::python::pickle_suite
{
  typedef basic_graph<DirectedS> Graph;
  typedef typename Graph::vertex_descriptor Vertex;
  typedef typename Graph::edge_descriptor Edge;

  static
  boost::python::tuple
  getstate(boost::python::object g_obj);
  
  static
  void
  setstate(boost::python::object g_obj, boost::python::tuple state);
};

extern const char* graph_init_doc;
extern const char* add_vertex_property_doc;
extern const char* add_edge_property_doc;

} } } // end namespace boost::graph::python

namespace boost {
  // Doing the right thing here (by unifying with vertex_index_t and
  // edge_index_t) breaks GCC.
  template<typename DirectedS, typename Tag>
  struct property_map<graph::python::basic_graph<DirectedS>, Tag>
  {
  private:
    typedef typename graph::python::basic_graph<DirectedS>::VertexIndexMap
      vertex_index_type;
    typedef typename graph::python::basic_graph<DirectedS>::EdgeIndexMap
      edge_index_type;

    typedef typename mpl::if_<is_same<Tag, edge_index_t>,
                              edge_index_type,
                              detail::error_property_not_found>::type
      edge_or_none;

  public:
    typedef typename mpl::if_<is_same<Tag, vertex_index_t>,
                              vertex_index_type,
                              edge_or_none>::type type;

    typedef type const_type;
  };

  template<typename DirectedS, typename Tag>
  struct property_map<const graph::python::basic_graph<DirectedS>, Tag>
  {
  private:
    typedef typename graph::python::basic_graph<DirectedS>::VertexIndexMap
      vertex_index_type;
    typedef typename graph::python::basic_graph<DirectedS>::EdgeIndexMap
      edge_index_type;

    typedef typename mpl::if_<is_same<Tag, edge_index_t>,
                              edge_index_type,
                              detail::error_property_not_found>::type
      edge_or_none;

  public:
    typedef typename mpl::if_<is_same<Tag, vertex_index_t>,
                              vertex_index_type,
                              edge_or_none>::type type;

    typedef type const_type;
  };

}

#endif // BOOST_GRAPH_BASIC_GRAPH_HPP
