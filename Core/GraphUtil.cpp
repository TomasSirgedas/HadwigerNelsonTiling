#include "GraphUtil.h"
#include "DualGraph.h"
#include "TileGraph.h"

#include <map>
#include <set>



std::shared_ptr<TileGraph> makeGraph( DualGraph& dual, double radius )
{
   dual.sortNeighbors();

   std::shared_ptr<TileGraph> graph( new TileGraph );
   graph->_GraphShape = dual._GraphShape;
   graph->_GraphSymmetry = dual._GraphSymmetry;

   std::map<std::set<int>, int> dualPolygonToTileVertexIndex;

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
               if ( dualPolygonToTileVertexIndex.count( polyAsSet ) )
               {
                  tileVertex = TileGraph::VertexPtr( dualPolygonToTileVertexIndex.at(polyAsSet), sector.inverted() );
                  break;
               }
            }
         }         

         if ( !tileVertex.isValid() ) // create it if needed
         {
            XYZ sum;
            for ( const DualGraph::VertexPtr& c : poly )
               sum += c.pos();

            TileGraph::Vertex v( (int) graph->_Vertices.size() );
            //v._IsSymmetrical = MatrixSymmetryMap::symmetryFor( sum )->hasSymmetry();
            //v._Neighbors;
            v._Pos = graph->_GraphShape->toSurfaceFrom3D( sum / poly.size() ) * radius;
            //v._Tiles;
            graph->_Vertices.push_back( v );
            tileVertex = TileGraph::VertexPtr( v._Index, Matrix4x4() );
            dualPolygonToTileVertexIndex[polyAsSet] = v._Index;
         }

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

