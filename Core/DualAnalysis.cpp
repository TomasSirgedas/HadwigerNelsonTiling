#include "DualAnalysis.h"

namespace
{
   std::string str( const DualGraph::VertexPtr& a ) { return std::to_string( a.id() ); }
   std::string operator+( const DualGraph::VertexPtr& a, const std::string& s ) { return str( a ) + s; }
   std::string operator+( const std::string& s, const DualGraph::VertexPtr& a ) { return s + str( a ); }
}


DualAnalysis::DualAnalysis( const DualGraph& dual )
{
   init( dual );
}

void DualAnalysis::init( const DualGraph& dual )
{
   for ( const DualGraph::VertexPtr& a : dual.rawVertices() )
   for ( const DualGraph::VertexPtr& b : a.neighbors() )
      if ( a.color() == b.color() && a.color() != BLANK_COLOR )
         return setError( "neighbors " + a + " and " + b + " are the same color", { a, b } );

   for ( const DualGraph::VertexPtr& a : dual.rawVertices() )
   for ( const DualGraph::VertexPtr& b : a.neighbors() )
   for ( const DualGraph::VertexPtr& c : a.neighbors() ) if ( b.id() < c.id() )
      if ( c.color() == b.color() && c.color() != BLANK_COLOR )
         return setError( b + " and " + c + " share a neighbor and are the same color", { b, c } );


   for ( const DualGraph::VertexPtr& a : dual.rawVertices() )
   for ( const DualGraph::VertexPtr& b : a.neighbors() ) if ( a.id() < b.id() )
   {
      DualGraph::VertexPtr diag;

      auto diagsA = a.diagonals();
      auto diagsB = b.diagonals();
      int direction = -1;

      for ( int i = 0; i < 2; i++ )
      for ( const DualGraph::VertexPtr& d : (i?b:a).diagonals() ) if ( d.color() == (i?a:b).color() && d.color() != BLANK_COLOR )
      {
         if ( diag.isValid() )
            return setError( "edge " + a + " " + b + " is curved to both " + diag + " and " + d, { a, b, diag, d } );
         diag = d;
         direction = i;
      }
      if ( diag.isValid() )
         _CurveDirectionForEdge[{a.id(),b.id()}] = direction == 1;
   }
   //if ( isError() )
   //   return;


}

bool DualAnalysis::isCurved( const DualGraph::VertexPtr& a, const DualGraph::VertexPtr& b ) const
{
   std::pair<int,int> pr( a.id(), b.id() );
   if ( pr.first > pr.second )
      std::swap( pr.first, pr.second );
   return _CurveDirectionForEdge.count( pr ) > 0;
}

bool DualAnalysis::isCurvedTowardsA( const DualGraph::VertexPtr& a, const DualGraph::VertexPtr& b ) const
{
   std::pair<int,int> pr( a.id(), b.id() );
   bool swappedAB = pr.first > pr.second;
   if ( swappedAB )
      std::swap( pr.first, pr.second );
   auto it = _CurveDirectionForEdge.find( pr );
   if ( it == _CurveDirectionForEdge.end() )
      return false;
   return it->second != swappedAB;
}

void DualAnalysis::setError( const std::string& error, std::vector<DualGraph::VertexPtr> vertices )
{
   _ErrorMessage = error;
   _ErrorVertices = vertices;
}
