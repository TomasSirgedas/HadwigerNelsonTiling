#include "TileGraph.h"

#include <unordered_set>

std::vector<TileGraph::TilePtr> TileGraph::allTiles() const
{   
   std::vector<TilePtr> ret;
   for ( const Tile& tile : _Tiles )
      for ( const SectorId& sector : tile._Symmetry->uniqueSectors() )
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

TileGraph::VertexPtr TileGraph::vertexWithId( int id ) const
{
   for ( const VertexPtr& a : allVertices() )
      if ( a.id() == id )
         return a;
   return VertexPtr();
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

std::vector<TileGraph::TilePtr> TileGraph::VertexPtr::tiles() const
{
   std::vector<TilePtr> ret;
   for ( const TilePtr& tile : baseVertex()._Tiles )
      ret.push_back( tile.premul( sectorId() ) );
   return ret;
}

bool TileGraph::VertexPtr::hasTile( const TilePtr& tile ) const
{
   std::vector<TilePtr> ret;
   for ( const TilePtr& a : tiles() )
      if ( a == tile )
         return true;
   return false;
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
      ret.push_back( vtx.premul( sectorId() ) );
   return ret;
}

TileGraph::VertexPtr TileGraph::VertexPtr::calcCurve( const VertexPtr& b ) const
{
   std::vector<TilePtr> tiles = _Graph->tilesAt( *this, b );
   if ( tiles.size() != 2 )
      return VertexPtr();

   for ( int tileIdx = 0; tileIdx < 2; tileIdx++ )
   {
      int otherTileColor = tiles[1-tileIdx].color();
      if ( otherTileColor == BLANK_COLOR )
         continue;
      for ( const VertexPtr& vtx : tiles[tileIdx].vertices() )
      {
         if ( vtx == *this ) continue;
         if ( vtx == b ) continue;
         if ( vtx.hasColor( otherTileColor ) )
            return vtx;
      }
   }
   return VertexPtr();
}

std::vector<TileGraph::TilePtr> TileGraph::tilesAt( const VertexPtr& a, const VertexPtr& b ) const
{
   std::vector<TilePtr> ret;
   for ( const TilePtr& tile : a.tiles() )
      if ( b.hasTile( tile ) )
         ret.push_back( tile );
   return ret;
}

namespace
{
   typedef std::unordered_set<TileGraph::VertexPtr, TileGraph::VertexPtrHash> VertexSet;
   VertexSet calcNeighbors( const VertexSet& st )
   {
      VertexSet ret = st;
      for ( const TileGraph::VertexPtr& vtx : st )
      for ( const TileGraph::VertexPtr& neighb : vtx.neighbors() )
         ret.insert( neighb );
      return ret;
   }

   VertexSet calcNeighbors( const TileGraph::VertexPtr& vtx, int depth )
   {
      VertexSet ret = { vtx };
      for ( int i = 0; i < depth; i++ )
         ret = calcNeighbors( ret );
      return ret;
   }
}

std::vector<TileGraph::VertexPtr> TileGraph::VertexPtr::neighbors( int depth ) const
{
   std::unordered_set<VertexPtr, VertexPtrHash> st = calcNeighbors( *this, depth );
   st.erase( *this );
   return std::vector<TileGraph::VertexPtr>( st.begin(), st.end() );
}


std::vector<TileGraph::VertexPtr> TileGraph::TilePtr::vertices() const
{
   std::vector<VertexPtr> ret;
   for ( const VertexPtr& a : baseTile()._Vertices )
      ret.push_back( a.premul( _SectorId ) );
   return ret;
}

bool TileGraph::VertexPtr::hasColor( int color ) const
{
   for ( const TilePtr& a : tiles() )
      if ( a.color() == color )
         return true;
   return false;
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
      if ( tileB.isValid() && tileA != tileB )
         return true;
   }
   return false;
}

bool TileGraph::mustBeClose( const VertexPtr& a, const VertexPtr& b ) const
{
   for ( const TilePtr& tileA : a.tiles() )
   {
      TilePtr tileB = b.tileWithColor( tileA.color() );
      //if ( tileA.color() == 8 )
      //   return false;
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

void TileGraph::normalizeVertices()
{
   for ( Vertex& a : _Vertices )
      a._Pos = _GraphShape->toSurfaceFrom3D( a._Pos );
}