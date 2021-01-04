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

std::vector<TileGraph::VertexPtr> TileGraph::rawVertices() const
{
   std::vector<VertexPtr> ret;
   for ( const Vertex& a : _Vertices )
      ret.push_back( a.toVertexPtr( this ) );
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

std::vector<TileGraph::TilePtr> TileGraph::VertexPtr::tiles() const
{
   std::vector<TilePtr> ret;
   for ( const TilePtr& tile : baseVertex()._Tiles )
      ret.push_back( tile.premul( matrix() ) );
   return ret;
}

TileGraph::TilePtr TileGraph::VertexPtr::tileWithColor( int color ) const
{
   std::vector<TilePtr> ret;
   for ( const TilePtr& tile : tiles() )
      if ( tile.color() == color )
         return tile;
   return TileGraph::TilePtr();
}

std::vector<TileGraph::VertexPtr> TileGraph::VertexPtr::neighbors() const
{
   std::vector<VertexPtr> ret;
   for ( const VertexPtr& vtx : baseVertex()._Neighbors )
      ret.push_back( vtx.premul( matrix() ) );
   return ret;
}

namespace
{
   void calcNeighbors( const TileGraph::VertexPtr& vtx, int depth, std::unordered_set<TileGraph::VertexPtr, TileGraph::VertexPtrHash>& st )
   {
      if ( !st.insert( vtx ).second )
         return;

      if ( depth <= 0 )
         return;

      std::vector<TileGraph::VertexPtr> ret;

      for ( const TileGraph::VertexPtr& neighb : vtx.neighbors() )
         calcNeighbors( neighb, depth-1, st );
   }
}

std::vector<TileGraph::VertexPtr> TileGraph::VertexPtr::neighbors( int depth ) const
{
   std::unordered_set<VertexPtr, VertexPtrHash> st;
   calcNeighbors( *this, depth, st );
   st.erase( *this );
   return std::vector<TileGraph::VertexPtr>( st.begin(), st.end() );
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

XYZ TileGraph::TilePtr::avgPos() const
{
   XYZ sum;
   for ( const VertexPtr& a : vertices() )
      sum += a.pos();
   return _Graph->_GraphShape->toSurfaceFrom3D( sum / vertices().size() );
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


bool TileGraph::mustBeFar( const VertexPtr& a, const VertexPtr& b ) const
{   
   for ( const TilePtr& tileA : a.tiles() ) if ( tileA.color() != BLANK_COLOR )
   {
      TilePtr tileB = b.tileWithColor( tileA.color() );
      if ( tileA != tileB )
         return true;
   }
   return false;
}

bool TileGraph::mustBeClose( const VertexPtr& a, const VertexPtr& b ) const
{
   for ( const TilePtr& tileA : a.tiles() )
   {
      TilePtr tileB = b.tileWithColor( tileA.color() );
      if ( tileA == tileB )
         return true;
   }
   return false;
}


std::vector<TileGraph::KeepCloseFar> TileGraph::calcKeepCloseFars() const
{
   std::vector<KeepCloseFar> ret;
   for ( const VertexPtr& vtx : rawVertices() )
   {      
      for ( const VertexPtr& neighb : vtx.neighbors( 5/*search depth*/ ) )
      {
         KeepCloseFar kcf;
         kcf.a = vtx;
         kcf.b = neighb;
         kcf.keepClose = mustBeClose( vtx, neighb );
         kcf.keepFar = mustBeFar( vtx, neighb );
         if ( kcf.keepClose || kcf.keepFar )
            ret.push_back( kcf );
      }
   }
   return ret;
}

CORE_API void TileGraph::normalizeVertices()
{
   for ( Vertex& a : _Vertices )
      a._Pos = _GraphShape->toSurfaceFrom3D( a._Pos );
}