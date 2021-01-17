#include "DualAnalysis.h"

namespace
{
   std::string str( const DualGraph::VertexPtr& a ) { return std::to_string( a.id() ); }
   std::string operator+( const DualGraph::VertexPtr& a, const std::string& s ) { return str( a ) + s; }
   std::string operator+( const std::string& s, const DualGraph::VertexPtr& a ) { return s + str( a ); }
}


DualAnalysis::DualAnalysis( const DualGraph& dual )
{
   for ( const DualGraph::VertexPtr& a : dual.rawVertices() )
      for ( const DualGraph::VertexPtr& b : a.neighbors() )
         if ( a.color() == b.color() )
         {
            setError( "neighbors " + a + " and " + b + " are the same color", { a, b } );
            return;
         }

   for ( const DualGraph::VertexPtr& a : dual.rawVertices() )
      for ( const DualGraph::VertexPtr& b : a.neighbors() )
         for ( const DualGraph::VertexPtr& c : a.neighbors() ) if ( b.id() < c.id() )
            if ( c.color() == b.color() )
            {
               setError( b + " and " + c + " share a neighbor and are the same color", { b, c } );
               return;
            }
}

void DualAnalysis::setError( const std::string& error, std::vector<DualGraph::VertexPtr> vertices )
{
   _ErrorMessage = error;
   _ErrorVertices = vertices;
}
