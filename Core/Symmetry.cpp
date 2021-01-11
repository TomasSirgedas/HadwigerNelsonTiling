#include "Symmetry.h"

#include <cassert>
#include <set>
#include <algorithm>

void SymmetryGroup::init( int visibleLoIndexHint, int visibleHiIndexHint )
{
   const int MAX_SIZE = 100;
   Matrix4x4 m = _Matrix;
   int groupSize;
   for ( groupSize = 1; !m.eq( Matrix4x4() ) && groupSize <= MAX_SIZE; groupSize++ )
      m = _Matrix * m;

   if ( groupSize > MAX_SIZE )
   {
      _IsFinite = false;
      _LoIndex = _VisibleLoIndex - 8;
      _HiIndex = _VisibleHiIndex + 8;
      _VisibleLoIndex = visibleLoIndexHint;
      _VisibleHiIndex = visibleHiIndexHint;
      assert( _VisibleHiIndex > _VisibleLoIndex );
   }
   else
   {
      assert( visibleLoIndexHint == 0 && visibleHiIndexHint == 0 ); // only use hints for infinite groups
      _IsFinite = true;
      _VisibleLoIndex = _LoIndex = 0;
      _VisibleHiIndex = _HiIndex = groupSize;
      assert( _ColorPerm.pow( groupSize ).isIdentity() ); // make sure color group permutation is valid
   }
}

Json SymmetryGroup::toJson() const
{
   Json ret;
   ret["matrix"] = _Matrix.toJson();
   ret["colorPerm"] = _ColorPerm.toJson();
   ret["loIndex"] = _LoIndex;
   ret["hiIndex"] = _HiIndex;
   ret["visibleLoIndex"] = _VisibleLoIndex;
   ret["visibleHiIndex"] = _VisibleHiIndex;
   ret["isFinite"] = _IsFinite;
   return ret;
}

SymmetryGroup::SymmetryGroup( const Json& json )
{
   _Matrix           = Matrix4x4( json["matrix"] );
   _ColorPerm        = Perm( json["colorPerm"] );
   _LoIndex          = json["loIndex"].toInt();
   _HiIndex          = json["hiIndex"].toInt();
   _VisibleLoIndex   = json["visibleLoIndex"].toInt();
   _VisibleHiIndex   = json["visibleHiIndex"].toInt();
   _IsFinite         = json["isFinite"].toBool();
}

GraphSymmetry_Groups::GraphSymmetry_Groups( const std::vector<SymmetryGroup>& groups ) : _Groups( groups )
{
   int N = (int) _Groups.size();
   {
      std::vector<int> groupIndexes = { std::vector<int>( N, -1 ) };
      initAllSectorGroupIndexes( 0, groupIndexes );
   }

   for ( int sectorId = 0; sectorId < (int)_AllSectorGroupIndexes.size(); sectorId++ )
   {
      Matrix4x4 m;
      for ( int i = 0; i < N; i++ )
         m = m * _Groups[i].matrix( _AllSectorGroupIndexes[sectorId][i] );

      uint64_t hash = matrixHash( m );
      assert( !_SectorHashToId.count( hash ) );
      _SectorHashToId[hash] = sectorId;

      _SectorIdToMatrix.push_back( m );
      _AllSectors.push_back( SectorId( sectorId, this ) );
   }

   for ( int sectorId = 0; sectorId < (int)_AllSectorGroupIndexes.size(); sectorId++ )
   {
      Perm perm;
      for ( int i = 0; i < N; i++ )
         perm = _Groups[i].colorPerm( _AllSectorGroupIndexes[sectorId][i] ) * perm;

      _SectorIdToColorPerm.push_back( perm );
   }

   for ( int sectorId = 0; sectorId < (int)_AllSectorGroupIndexes.size(); sectorId++ )
   {
      std::string name;
      for ( int i = 0; i < N; i++ )
         name += std::to_string( _AllSectorGroupIndexes[sectorId][i] );

      bool allZeros = std::count( name.begin(), name.end(), '0' ) == name.size();
      if ( allZeros )
         name = "";

      _SectorIdToName.push_back( name );
   }   

   for ( int sectorId = 0; sectorId < (int)_AllSectorGroupIndexes.size(); sectorId++ )
   {
      bool isVisible = true;
      for ( int i = 0; i < N; i++ )
         isVisible = isVisible && _Groups[i].isVisible( _AllSectorGroupIndexes[sectorId][i] );
      if ( isVisible )
         _AllVisibleSectors.push_back( SectorId( sectorId, this ) );
      _SectorIdIsVisible.push_back( isVisible );
   }

   for ( int sectorId = 0; sectorId < (int)_AllSectorGroupIndexes.size(); sectorId++ )
   {
      _Invert.push_back( this->sectorId( matrix( sectorId ).inverted() ) );
   }

   _Mul.resize( (int)_AllSectorGroupIndexes.size() );
   for ( int a = 0; a < (int)_AllSectorGroupIndexes.size(); a++ )
   for ( int b = 0; b < (int)_AllSectorGroupIndexes.size(); b++ )
   {
      _Mul[a].push_back( sectorId( matrix( a ) * matrix( b ) ) );
   }
}

void GraphSymmetry_Groups::initAllSectorGroupIndexes( int i, std::vector<int>& groupIndexes )
{
   if ( i >= (int) _Groups.size() )
   { 
      _AllSectorGroupIndexes.push_back( groupIndexes );
      return;
   }
   std::vector<int> indexes;
   for ( int k = _Groups[i].loIndex(); k < _Groups[i].hiIndex(); k++ )
      indexes.push_back( k );
   std::sort( indexes.begin(), indexes.end(), []( int a, int b ) { return (uint32_t) a < (uint32_t) b; } );
   for ( int k : indexes )
   {
      groupIndexes[i] = k;
      initAllSectorGroupIndexes( i+1, groupIndexes );
      groupIndexes[i] = -1;
   }
}

Json GraphSymmetry_Groups::toJson() const
{
   Json ret;
   for ( const SymmetryGroup& g : _Groups )
      ret.push_back( g.toJson() );
   return ret;
}

std::shared_ptr<IGraphSymmetry> IGraphSymmetry::fromJson( const Json& json )
{
   std::vector<SymmetryGroup> symmetryGroups;
   for ( const Json& e : json.toArray() )
      symmetryGroups.push_back( SymmetryGroup( e ) );
   return std::shared_ptr<IGraphSymmetry>( new GraphSymmetry_Groups( symmetryGroups ) );
}





SectorSymmetryForVertex::SectorSymmetryForVertex( const IGraphSymmetry* graphSymmetry, const XYZ& pos ) : _GraphSymmetry( graphSymmetry )
{   
   std::vector<SectorId> allSectors = _GraphSymmetry->allSectors();

   _EquivalentSectorIds.resize( allSectors.size() );
   //_EquivalentSectors.resize( allSectors.size() );

   for ( int a = 0; a < (int)allSectors.size(); a++ )
   for ( int b = 0; b < (int)allSectors.size(); b++ )
   {
      XYZ posA = allSectors[a].matrix() * pos;
      XYZ posB = allSectors[b].matrix() * pos;
      if ( posA.dist2( posB ) < 1e-12 )
      {
         _EquivalentSectorIds[a].push_back( SectorId( b, graphSymmetry ) );
         //_EquivalentSectors[a].push_back( allSectors[b] );
      }
   }

   for ( int a = 0; a < (int)allSectors.size(); a++ ) if ( _GraphSymmetry->isSectorIdVisible( a ) )
      if ( _EquivalentSectorIds[a][0].id() == a )
         _UniqueSectors.push_back( allSectors[a] );

   _IdentitySectorId = graphSymmetry->sectorId( Matrix4x4() );
}

//Matrix4x4 SectorSymmetryForVertex::canonicalizedSector( const Matrix4x4& sector ) const
//{
//   if ( !hasSymmetry() )
//      return sector;
//
//   return _EquivalentSectors[_GraphSymmetry->sectorId( sector )][0];
//}

SectorId SectorSymmetryForVertex::canonicalizedSectorId( const SectorId& sectorId ) const
{
   if ( !hasSymmetry() )
      return sectorId;

   return _EquivalentSectorIds[sectorId.id()][0];
}

std::shared_ptr<IGraphShape> IGraphShape::fromJson( const Json& json )
{
   if ( json["type"] == "plane" )
      return std::shared_ptr<IGraphShape>( new GraphShapePlane() );
   if ( json["type"] == "sphere" )
      return std::shared_ptr<IGraphShape>( new GraphShapeSphere( json["radius"].toDouble() ) );
   throw 777;
   return nullptr;
}



SectorId SectorId::operator*( const SectorId& rhs ) const { return SectorId( _GraphSymmetry->mul( _Id, rhs._Id ), _GraphSymmetry ); }
SectorId SectorId::inverted() const { return SectorId( _GraphSymmetry->inverted( _Id ), _GraphSymmetry ); }
Matrix4x4 SectorId::matrix() const { return _GraphSymmetry->matrix( _Id ); }

int SectorId::mapColor( int color ) const { return _GraphSymmetry->toSector( _Id, color ); }
int SectorId::unmapColor( int color ) const { return _GraphSymmetry->fromSector( _Id, color ); }