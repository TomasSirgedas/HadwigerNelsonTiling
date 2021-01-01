#include "DualGraph.h"


const DualGraph::Vertex& DualGraph::VertexPtr::baseVertex() const { return _Graph->_Vertices[_Index]; }
XYZ DualGraph::VertexPtr::pos() const { return ( _Matrix * baseVertex().pos ).toXYZ(); };
int DualGraph::VertexPtr::color() const { return _Graph->_GraphSymmetry->toSector( _SectorId, baseVertex().color ); }
std::string DualGraph::VertexPtr::name() const 
{ 
   std::string sectorName = _Graph->_GraphSymmetry->sectorName( _SectorId );
   return std::to_string( _Index ) + (sectorName.empty() ? "" : "-") + sectorName;
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
      for ( const Matrix4x4& sector : vtx.symmetry->sectors() )
         ret.push_back( VertexPtr( this, vtx.index, sector ) );

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
   _Vertices[vtx._Index].pos = (vtx._Matrix.inverted() * pos).toXYZ();
}

void DualGraph::toggleEdge( const VertexPtr& a, const VertexPtr& b )
{
   if ( !a.isValid() || !b.isValid() || a == b )
      return;

   bool hadEdge = _Vertices[a._Index].hasNeighbor( b.unpremul( a._Matrix ) );
   if ( hadEdge )
   {
      _Vertices[a._Index].removeNeighbor( b.unpremul( a._Matrix ) );
      _Vertices[b._Index].removeNeighbor( a.unpremul( b._Matrix ) );
   }
   else
   {
      _Vertices[a._Index].addNeighbor( b.unpremul( a._Matrix ) );
      _Vertices[b._Index].addNeighbor( a.unpremul( b._Matrix ) );
   }
}