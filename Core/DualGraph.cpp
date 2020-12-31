#include "DualGraph.h"


const DualGraph::Vertex& DualGraph::VertexPtr::baseVertex() const { return graph->_Vertices[index]; }
XYZ DualGraph::VertexPtr::pos() const { return graph->_GraphSymmetry->toSector( sectorId, baseVertex().pos ); };
int DualGraph::VertexPtr::color() const { return graph->_GraphSymmetry->toSector( sectorId, baseVertex().color ); }
std::string DualGraph::VertexPtr::name() const 
{ 
   return std::to_string( index ) + graph->_GraphSymmetry->sectorName( sectorId );
}

std::vector<DualGraph::VertexPtr> DualGraph::VertexPtr::neighbors() const
{
   std::vector<DualGraph::VertexPtr> ret;
   for ( const DualGraph::VertexPtr& baseNeighb : baseVertex().neighbors )
   {
      DualGraph::VertexPtr neighb = baseNeighb.premul( sectorId );
      for ( int otherSectorId : baseVertex().symmetry->sectorsEquivalentTo( 0 ) )
         ret.push_back( neighb.premul( otherSectorId ) );
   }
   return ret;
}

DualGraph::VertexPtr DualGraph::VertexPtr::premul( int otherSectorId ) const
{
   return withSectorId( graph->_GraphSymmetry->combineSectors( otherSectorId, sectorId ) );
}

DualGraph::VertexPtr DualGraph::VertexPtr::unpremul( int otherSectorId ) const
{
   return withSectorId( graph->_GraphSymmetry->combineSectors( graph->_GraphSymmetry->invertSector( otherSectorId ), sectorId ) );
}


DualGraph::DualGraph( std::shared_ptr<IGraphSymmetry> graphSymmetry ) : _GraphSymmetry( graphSymmetry )
{   
}

void DualGraph::addVertex( int color, const XYZ& pos )
{
   _Vertices.push_back( Vertex( (int) _Vertices.size(), color, pos ) );
   _Vertices.back().symmetry = _GraphSymmetry->calcSectorSymmetry( pos );
}


std::vector<DualGraph::VertexPtr> DualGraph::allVertices() const
{
   std::vector<VertexPtr> ret;

   for ( const Vertex& vtx : _Vertices )
      for ( int sectorId : vtx.symmetry->sectors() )
         ret.push_back( VertexPtr( this, vtx.index, sectorId ) );

   return ret;
}

DualGraph::VertexPtr DualGraph::vertexAt( const XYZ& pos, double maxDist ) const
{
   VertexPtr ret;
   double bestDist2 = maxDist * maxDist;


   for ( const VertexPtr& a : allVertices() )
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
   _Vertices[vtx.index].color = _GraphSymmetry->fromSector( vtx.sectorId, color );
}

void DualGraph::setVertexPos( const VertexPtr& vtx, const XYZ& pos )
{
   _Vertices[vtx.index].pos = _GraphSymmetry->fromSector( vtx.sectorId, pos );
}

void DualGraph::toggleEdge( const VertexPtr& a, const VertexPtr& b )
{
   if ( !a.isValid() || !b.isValid() || a == b )
      return;

   bool hadEdge = _Vertices[a.index].hasNeighbor( b.unpremul( a.sectorId ) );
   if ( hadEdge )
   {
      _Vertices[a.index].removeNeighbor( b.unpremul( a.sectorId ) );
      _Vertices[b.index].removeNeighbor( a.unpremul( b.sectorId ) );
   }
   else
   {
      _Vertices[a.index].addNeighbor( b.unpremul( a.sectorId ) );
      _Vertices[b.index].addNeighbor( a.unpremul( b.sectorId ) );
   }
}