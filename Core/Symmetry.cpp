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
         _AllVisibleSectors.push_back( _SectorIdToMatrix[sectorId] );
      _SectorIdIsVisible.push_back( isVisible );
   }
}

void GraphSymmetry_Groups::initAllSectorGroupIndexes( int i, std::vector<int>& groupIndexes )
{
   if ( i >= (int) _Groups.size() )
   { 
      _AllSectorGroupIndexes.push_back( groupIndexes );
      return;
   }
   for ( int k = _Groups[i].loIndex(); k < _Groups[i].hiIndex(); k++ )
   {
      groupIndexes[i] = k;
      initAllSectorGroupIndexes( i+1, groupIndexes );
      groupIndexes[i] = -1;
   }
}







SectorSymmetryForVertex::SectorSymmetryForVertex( const IGraphSymmetry* graphSymmetry, const XYZ& pos ) : _GraphSymmetry( graphSymmetry )
{   
   std::vector<Matrix4x4> allSectors = _GraphSymmetry->allSectors();

   _EquivalentSectorIds.resize( allSectors.size() );
   _EquivalentSectors.resize( allSectors.size() );

   for ( int a = 0; a < (int)allSectors.size(); a++ )
   for ( int b = 0; b < (int)allSectors.size(); b++ )
   {
      XYZ posA = allSectors[a] * pos;
      XYZ posB = allSectors[b] * pos;
      if ( posA.dist2( posB ) < 1e-12 )
      {
         _EquivalentSectorIds[a].push_back( b );
         _EquivalentSectors[a].push_back( allSectors[b] );
      }
   }

   for ( int a = 0; a < (int)allSectors.size(); a++ ) if ( _GraphSymmetry->isSectorIdVisible( a ) )
      if ( _EquivalentSectorIds[a][0] == a )
         _UniqueSectors.push_back( allSectors[a] );

   _IdentitySectorId = graphSymmetry->sectorId( Matrix4x4() );
}

Matrix4x4 SectorSymmetryForVertex::canonicalizedSector( const Matrix4x4& sector ) const
{
   if ( !hasSymmetry() )
      return sector;

   return _EquivalentSectors[_GraphSymmetry->sectorId( sector )][0];
}


