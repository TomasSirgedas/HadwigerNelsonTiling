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
            for ( const Matrix4x4& sector : dual._GraphSymmetry->allVisibleSectors() )
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
         }

         if ( tileVertex.isValid() )
            tile._Vertices.push_back( tileVertex );         
      }
      graph->_Tiles.push_back( tile );
      //for ( const TileGraph::VertexPtr& vtx : tile._Vertices )
      //   if ( tile._SymmetryMap->isReal( vtx._Mtx ) ) // only add one copy
      //      graph->_Vertices[vtx._Index]._Tiles.push_back( TileGraph::TilePtr( tile._Index, vtx._Mtx.inverted() ) );
      //for ( int i = 0; i < (int) tile._Vertices.size(); i++ )
      //   graph->addNeighbor( tile._Vertices[i], tile._Vertices[(i+1)%tile._Vertices.size()] );
   }


   //   for ( const Graph::VertexPtr& a : graph->allVertices() )
   //      for ( const Graph::VertexPtr& b : graph->neighbors( a ) )
   //         qDebug() << graph->idOf( a ) << "-" << graph->idOf( b );

   return graph;
}

std::ostream& operator<<( std::ostream& os, const DualGraph::VertexPtr& a ) { return os << a.name(); }