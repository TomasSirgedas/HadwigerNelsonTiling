#include "DualGraph.h"


const DualGraph::Vertex& DualGraph::VertexPtr::baseVertex() const { return graph->_Vertices[index]; }
XYZ DualGraph::VertexPtr::pos() const { return graph->_GraphSymmetry->toSector( sector, baseVertex().pos ); };
int DualGraph::VertexPtr::color() const { return graph->_GraphSymmetry->toSector( sector, baseVertex().color ); }
std::string DualGraph::VertexPtr::name() const 
{ 
   std::string sectorName = graph->_GraphSymmetry->sectorName( sector );
   return std::to_string( index ) + (sectorName.empty() ? "" : "-") + sectorName;
}

std::vector<DualGraph::VertexPtr> DualGraph::VertexPtr::neighbors() const
{
   std::vector<DualGraph::VertexPtr> ret;
   for ( const DualGraph::VertexPtr& baseNeighb : baseVertex().neighbors )
   {
      DualGraph::VertexPtr neighb = baseNeighb.premul( sector );
      for ( const Sector& otherSector : baseVertex().symmetry->sector0Equivalents() )
         ret.push_back( neighb.premul( otherSector ) );
   }
   return ret;
}

DualGraph::VertexPtr DualGraph::VertexPtr::premul( const Sector&  otherSector ) const
{
   return withSectorId( graph->_GraphSymmetry->combineSectors( otherSector, sector ) );
}

DualGraph::VertexPtr DualGraph::VertexPtr::unpremul( const Sector&  otherSector ) const
{
   return withSectorId( graph->_GraphSymmetry->combineSectors( graph->_GraphSymmetry->invertSector( otherSector ), sector ) );
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


std::vector<DualGraph::VertexPtr> DualGraph::allVertices() const
{
   std::vector<VertexPtr> ret;

   for ( const Vertex& vtx : _Vertices )
      for ( const Sector& sector : vtx.symmetry->sectors() )
         ret.push_back( VertexPtr( this, vtx.index, sector ) );

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
   _Vertices[vtx.index].color = _GraphSymmetry->fromSector( vtx.sector, color );
}

void DualGraph::setVertexPos( const VertexPtr& vtx, const XYZ& pos )
{
   _Vertices[vtx.index].pos = _GraphSymmetry->fromSector( vtx.sector, pos );
}

void DualGraph::toggleEdge( const VertexPtr& a, const VertexPtr& b )
{
   if ( !a.isValid() || !b.isValid() || a == b )
      return;

   bool hadEdge = _Vertices[a.index].hasNeighbor( b.unpremul( a.sector ) );
   if ( hadEdge )
   {
      _Vertices[a.index].removeNeighbor( b.unpremul( a.sector ) );
      _Vertices[b.index].removeNeighbor( a.unpremul( b.sector ) );
   }
   else
   {
      _Vertices[a.index].addNeighbor( b.unpremul( a.sector ) );
      _Vertices[b.index].addNeighbor( a.unpremul( b.sector ) );
   }
}