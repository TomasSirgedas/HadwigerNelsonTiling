#include "TileGraph.h"

#include <unordered_set>

std::vector<TileGraph::TilePtr> TileGraph::allTiles() const
{   
   std::vector<TilePtr> ret;
   for ( const Tile& tile : _Tiles )
      for ( const Matrix4x4& sector : tile._Symmetry->uniqueSectors() )
         ret.push_back( tile.toTilePtr( this ).premul( sector ) );
   return ret;
}

std::vector<TileGraph::VertexPtr> TileGraph::allVertices() const
{   
   std::unordered_set<int> usedIds;
   std::vector<VertexPtr> ret;
   for ( const TilePtr& tile : allTiles() )
      for ( const VertexPtr& vtx : tile.vertices() )
      {
         if ( !usedIds.insert( vtx.id() ).second ) continue; // already used
         ret.push_back( vtx );
      }
   return ret;
}

std::vector<TileGraph::TilePtr> TileGraph::rawTiles() const
{
   std::vector<TilePtr> ret;
   for ( const Tile& tile : _Tiles )
      ret.push_back( tile.toTilePtr( this ) );
   return ret;
}

void TileGraph::VertexPtr::updateCache()
{
   _SectorId = _Graph->_GraphSymmetry->sectorId( _Matrix );
}



std::vector<TileGraph::VertexPtr> TileGraph::TilePtr::vertices() const
{
   std::vector<VertexPtr> ret;
   for ( const VertexPtr& a : baseTile()._Vertices )
      ret.push_back( a.premul( _Matrix ) );
   return ret;
}

void TileGraph::TilePtr::updateCache()
{
   _SectorId = _Graph->_GraphSymmetry->sectorId( _Matrix );
}



TileGraph::Vertex& TileGraph::addVertex( const XYZ& pos )
{
   _Vertices.push_back( Vertex( (int) _Vertices.size(), pos ) );
   _Vertices.back()._Symmetry = _GraphSymmetry->calcSectorSymmetry( pos );
   return _Vertices.back();
}


TileGraph::VertexPtr TileGraph::vertexAt( const XYZ& pos, double maxDist ) const
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

void TileGraph::setVertexPos( const VertexPtr& vtx, const XYZ& pos )
{
   _Vertices[vtx.index()]._Pos = vtx.matrix().inverted() * pos;
}