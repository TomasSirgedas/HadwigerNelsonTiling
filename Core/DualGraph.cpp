#include "DualGraph.h"
#include "trace.h"

const DualGraph::Vertex& DualGraph::VertexPtr::baseVertex() const { return _Graph->_Vertices[_Index]; }
XYZ DualGraph::VertexPtr::pos() const { return _Matrix * baseVertex().pos; };
int DualGraph::VertexPtr::color() const { return _Graph->_GraphSymmetry->toSector( _SectorId, baseVertex().color ); }
std::string DualGraph::VertexPtr::name() const 
{ 
   //std::string sectorName = _Graph->_GraphSymmetry->sectorName( _SectorId );
   //return std::to_string( _Index ) + (sectorName.empty() ? "" : "-") + sectorName;
   return std::to_string( _Index ) + "-" + std::to_string( _SectorId );
   //return std::to_string( id() );
}

std::vector<DualGraph::VertexPtr> DualGraph::VertexPtr::neighbors() const
{
   std::vector<DualGraph::VertexPtr> ret;
   for ( const DualGraph::VertexPtr& baseNeighb : baseVertex().neighbors )
   {
      ret.push_back( baseNeighb.premul( _Matrix ) );
      //DualGraph::VertexPtr neighb = baseNeighb.premul( _Matrix );
      //for ( const Matrix4x4& otherSector : baseVertex().symmetry->sector0Equivalents() )
      //   ret.push_back( neighb.premul( otherSector ) );
   }
   return ret;
}

void DualGraph::VertexPtr::updateCache()
{
   _SectorId = _Graph->_GraphSymmetry->sectorId( _Matrix );
}

DualGraph::VertexPtr DualGraph::VertexPtr::premul( const Matrix4x4& mtx ) const
{
   return withMatrix( mtx * _Matrix );
}

DualGraph::VertexPtr DualGraph::VertexPtr::unpremul( const Matrix4x4& mtx ) const
{
   return withMatrix( mtx.inverted() * _Matrix );
}


DualGraph::DualGraph( std::shared_ptr<IGraphSymmetry> graphSymmetry, std::shared_ptr<IGraphShape> shape ) 
   : _GraphSymmetry( graphSymmetry )
   , _GraphShape( shape )
{   
}

void DualGraph::addVertex( int color, const XYZ& pos )
{
   _Vertices.push_back( Vertex( (int) _Vertices.size(), color, pos ) );
   _Vertices.back().symmetry = _GraphSymmetry->calcSectorSymmetry( pos );
}


std::vector<DualGraph::VertexPtr> DualGraph::allVisibleVertices() const
{
   std::vector<VertexPtr> ret;

   for ( const Vertex& vtx : _Vertices )
      for ( const Matrix4x4& sector : vtx.symmetry->uniqueSectors() )
         ret.push_back( vtx.toVertexPtr( this ).withMatrix( sector ) );

   return ret;
}

DualGraph::VertexPtr DualGraph::vertexAt( const XYZ& pos, double maxDist ) const
{
   VertexPtr ret;
   double bestDist2 = maxDist * maxDist;


   for ( const VertexPtr& a : allVisibleVertices() )
   {
      double dist2 = a.pos().dist2( pos );
      if ( dist2 < bestDist2 )
      {
         bestDist2 = dist2;
         ret = a;
      }
   }

   return ret;
}

void DualGraph::setVertexColor( const VertexPtr& vtx, int color )
{
   _Vertices[vtx._Index].color = _GraphSymmetry->fromSector( vtx._SectorId, color );
}

void DualGraph::setVertexPos( const VertexPtr& vtx, const XYZ& pos )
{
   _Vertices[vtx._Index].pos = vtx._Matrix.inverted() * pos;
}

void DualGraph::toggleEdge( const VertexPtr& a, const VertexPtr& b )
{
   if ( !a.isValid() || !b.isValid() || a == b )
      return;

   VertexPtr a0 = a.unpremul( b._Matrix );
   VertexPtr b0 = b.unpremul( a._Matrix );

   bool hadEdge = _Vertices[a._Index].hasNeighbor( b0 );
   if ( hadEdge )
   {
      _Vertices[a._Index].removeNeighbor( b0 );
      if ( a0 != b0 )
         _Vertices[b._Index].removeNeighbor( a0 );
   }
   else
   {
      _Vertices[a._Index].addNeighbor( b0 );
      if ( a0 != b0 )
         _Vertices[b._Index].addNeighbor( a0 );
   }

   //for ( auto a : (*this)[0].neighbors() )
   //   std::trace << a.name() << " ";
   //std::trace << std::endl;
}

void DualGraph::sortNeighbors()
{
   for ( Vertex& vtx : _Vertices )
   {
      XYZ n = _GraphShape->normalAt( vtx.pos );
      Matrix4x4 m = matrixRotateToZAxis( n ) * Matrix4x4::translation( -vtx.pos );
      auto angleOf = [&]( const XYZ& p ) { XYZ q = m*p; return ::atan2( q.y, q.x ); };
      sort( vtx.neighbors.begin(), vtx.neighbors.end(), [&]( const VertexPtr& a, const VertexPtr& b ) { return angleOf( a.pos() ) < angleOf( b.pos() ); } );
   }
}