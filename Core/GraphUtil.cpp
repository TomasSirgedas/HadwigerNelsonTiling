#include "GraphUtil.h"
#include "DualGraph.h"
#include "TileGraph.h"

#include <map>
#include <set>


std::shared_ptr<TileGraph> makeTileGraph( DualGraph& dual, double radius )
{
   dual.sortNeighbors();

   std::shared_ptr<TileGraph> graph( new TileGraph );
   graph->_GraphShape = dual._GraphShape;
   graph->_GraphSymmetry = dual._GraphSymmetry;

   std::map<std::set<int>, TileGraph::VertexPtr> dualPolygonToTileVertex;
   std::map<int, std::vector<DualGraph::VertexPtr>> polyMap;

   for ( const DualGraph::Vertex& aa : dual._Vertices )
   {
      DualGraph::VertexPtr a = aa.toVertexPtr( &dual );

      TileGraph::Tile tile;
      tile._Index = (int) graph->_Tiles.size();
      tile._Color = a.color();
      tile._Symmetry = a.baseVertex().symmetry;

      for ( const DualGraph::VertexPtr& b : a.neighbors() )
      {
         std::vector<DualGraph::VertexPtr> poly = a.polygon( b );

         TileGraph::VertexPtr tileVertex;
         std::set<int> polyAsSet;
         for ( const DualGraph::VertexPtr& c : poly )
            polyAsSet.insert( c.id() );

         // find TileGraph vertex if it was already created
         {            
            for ( const Matrix4x4& sector : dual._GraphSymmetry->allSectors() )
            {
               std::set<int> polyAsSet;
               for ( const DualGraph::VertexPtr& c : poly )
                  polyAsSet.insert( c.premul( sector ).id() );
               if ( dualPolygonToTileVertex.count( polyAsSet ) )
               {
                  tileVertex = dualPolygonToTileVertex.at(polyAsSet).premul( sector.inverted() );
                  break;
               }
            }
         }         

         if ( !tileVertex.isValid() && !poly.empty() ) // create it if needed
         {
            XYZ sum;
            for ( const DualGraph::VertexPtr& c : poly )
               sum += c.pos();

            TileGraph::Vertex& v = graph->addVertex( graph->_GraphShape->toSurfaceFrom3D( sum / poly.size() ) * radius );
            tileVertex = v.toVertexPtr( graph.get() );
            dualPolygonToTileVertex[polyAsSet] = tileVertex;
            polyMap[v._Index] = poly; // used to populate tile neighbors later
         }

         if ( tileVertex.isValid() )
         {
            tile._Vertices.push_back( tileVertex );
         }
      }
      graph->_Tiles.push_back( tile );
   }

   // populate vertex `_Tiles`
   for ( auto& pr : polyMap )
   for ( const DualGraph::VertexPtr& c : pr.second )
      graph->_Vertices[pr.first]._Tiles.push_back( TileGraph::TilePtr( graph.get(), c.index(), c.matrix() ) );

   // populate vertex `_Neighbors`
   for ( const TileGraph::VertexPtr& a : graph->rawVertices() )
   {
      std::vector<TileGraph::TilePtr> tiles = a.tiles();
      for ( int i = 0; i < (int) tiles.size(); i++ )
      {
         const TileGraph::TilePtr& tile = tiles[i];
         const TileGraph::TilePtr& nextTile = tiles[(i+1)%tiles.size()];
         const TileGraph::VertexPtr v0 = tile.next( a );
         const TileGraph::VertexPtr v1 = nextTile.prev( a );

         graph->_Vertices[a.index()]._Neighbors.push_back( v0 );
         if ( v0 != v1 )
            graph->_Vertices[a.index()]._Neighbors.push_back( v1 );
      }
   }

   //   for ( const Graph::VertexPtr& a : graph->allVertices() )
   //      for ( const Graph::VertexPtr& b : graph->neighbors( a ) )
   //         qDebug() << graph->idOf( a ) << "-" << graph->idOf( b );

   return graph;
}

std::ostream& operator<<( std::ostream& os, const DualGraph::VertexPtr& a ) { return os << a.name(); }
std::ostream& operator<<( std::ostream& os, const TileGraph::VertexPtr& a ) { return os << a.name(); }
std::ostream& operator<<( std::ostream& os, const TileGraph::TilePtr& a ) { return os << a.name(); }