#include "DualAnalysis.h"

namespace
{
   std::string str( const DualGraph::VertexPtr& a ) { return std::to_string( a.id() ); }
   std::string operator+( const DualGraph::VertexPtr& a, const std::string& s ) { return str( a ) + s; }
   std::string operator+( const std::string& s, const DualGraph::VertexPtr& a ) { return s + str( a ); }   
   bool intersects( int a0, int a1, int b0, int b1 )
   {
      if ( ( ( a0 > b0 && a1 > b0 ) || (a0 < b0 && a1 < b0) )
        && ( ( a0 > b1 && a1 > b1 ) || (a0 < b1 && a1 < b1) ) )
         return false;
   
      if ( ( ( b0 > a0 && b1 > a0 ) || (b0 < a0 && b1 < a0) )
        && ( ( b0 > a1 && b1 > a1 ) || (b0 < a1 && b1 < a1) ) )
         return false;

      return true;
   }
}

class PolyRigids
{
public:
   bool addRigid( std::pair<int,int> r ) 
   { 
      if ( !_IsValid )
         return false;

      if ( r.first > r.second )
         std::swap( r.first, r.second );

      for ( const auto& e : _Rigids )
      {
         if ( e == r )
            return true;
         if ( !intersects( r.first, r.second, e.first, e.second ) )
         {
            _Conflict[0] = e;
            _Conflict[1] = r;
            _IsValid = false;
            return _IsValid;
         }
      }
         
      _Rigids.push_back( r );
      return true;
   }

public:
   std::vector<std::pair<int,int>> _Rigids;
   bool _IsValid = true;
   std::pair<int,int> _Conflict[2];
};


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
      
   for ( const DualGraph::VertexPtr& a : dual.rawVertices() )
   {
      std::vector<std::vector<DualGraph::VertexPtr>> ed = a.edgesAndDiagonals();
      std::vector<int> colorPositions[MAX_COLORS+1];
      int colorNeighborPositions[MAX_COLORS+1];
      memset( colorNeighborPositions, -1, sizeof(colorNeighborPositions) );
      for ( int i = 0; i < (int)ed.size(); i++ )
      {
         colorNeighborPositions[ed[i][0].color()] = i;
         colorPositions[ed[i][0].color()].push_back( i==0?(int)ed.size()-1:i-1 );
         for ( int j = 0; j < (int)ed[i].size(); j++ )
            colorPositions[ed[i][j].color()].push_back( i );
      }

      PolyRigids rigids;
      for ( int color = 0; color < MAX_COLORS; color++ )
      {
         int falseCP0 = colorNeighborPositions[color];
         int falseCP1 = falseCP0==0 ? (int)ed.size()-1 : falseCP0-1;
         std::vector<int>& cp = colorPositions[color];
         for ( int i = 0; i < (int)cp.size(); i++ )
            for ( int j = i+1; j < (int)cp.size(); j++ ) if ( !( (cp[i] == falseCP0 && cp[j] == falseCP1) || (cp[i] == falseCP1 && cp[j] == falseCP0) ) )
               rigids.addRigid( { cp[i], cp[j] } );
      }

      if ( !rigids._IsValid )
      {
         auto verticesAt = [&]( int i ) { auto ret = ed[i]; ret.push_back( ed[i==0?(int)ed.size()-1:i-1][0] ); return ret; };
         DualGraph::VertexPtr bb[2];
         DualGraph::VertexPtr cc[2];
         for ( int i = 0; i < 2; i++ )
            for ( const DualGraph::VertexPtr& b : verticesAt( rigids._Conflict[i].first ) )
               for ( const DualGraph::VertexPtr& c : verticesAt( rigids._Conflict[i].second ) )
                  if ( b != c && b.color() == c.color() )
                     { bb[i] = b; cc[i] = c; }
         return setError( "non-intersecting rigids for tile " + a, { a, bb[0], cc[0], bb[1], cc[1] } );
      }
   }


}

bool DualAnalysis::isCurved( const DualGraph::VertexPtr& a_, const DualGraph::VertexPtr& b_ ) const
{
   DualGraph::VertexPtr a = a_.premul( a_.sectorId().inverted() );
   DualGraph::VertexPtr b = b_.premul( a_.sectorId().inverted() );

   std::pair<int,int> pr( a.id(), b.id() );
   if ( pr.first > pr.second )
      std::swap( pr.first, pr.second );
   return _CurveDirectionForEdge.count( pr ) > 0;
}

bool DualAnalysis::isCurvedTowardsA( const DualGraph::VertexPtr& a_, const DualGraph::VertexPtr& b_ ) const
{
   DualGraph::VertexPtr a = a_.premul( a_.sectorId().inverted() );
   DualGraph::VertexPtr b = b_.premul( a_.sectorId().inverted() );

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
