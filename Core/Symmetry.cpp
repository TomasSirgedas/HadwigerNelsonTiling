#include "Symmetry.h"

#include <cassert>

void SymmetryGroup::init()
{
   if ( size() == 0 )
   {
      Matrix4x4 m = _Matrix;
      for ( _HiIndex = 1; !m.eq( Matrix4x4() ) && _HiIndex < 100; _HiIndex++ )
         m = _Matrix * m;
      Perm p = _ColorPerm.pow( _HiIndex );
      assert( _ColorPerm.pow( _HiIndex ).isIdentity() );
   }
}

GraphSymmetry_Groups::GraphSymmetry_Groups( const std::vector<SymmetryGroup>& groups ) : _Groups( groups )
{
   int N = (int) _Groups.size();
   {
      std::vector<int> indexes( N, -1 );
      initSectorIndexes( 0, indexes );
   }

   for ( const std::vector<int>& indexes : _SectorIndexes )
   {
      Matrix4x4 m;
      for ( int i = 0; i < N; i++ )
         m = m * _Groups[i].matrix( indexes[i] );
      _SectorMatrices.push_back( m );
   }

   for ( const std::vector<int>& indexes : _SectorIndexes )
   {
      Perm perm;
      for ( int i = 0; i < N; i++ )
         perm = perm * _Groups[i].colorPerm( indexes[i] );
      _ColorPerms.push_back( perm );
      _ColorPermsInv.push_back( perm.inverted() );
   }

   for ( const std::vector<int>& indexesA : _SectorIndexes )
   {
      std::vector<int> combine;
      for ( const std::vector<int>& indexesB : _SectorIndexes )
      {
         std::vector<int> v;
         for ( int i = 0; i < N; i++ )
            v.push_back( _Groups[i].combine( indexesA[i], indexesB[i] ) );
         combine.push_back( sectorIdOf( v ) );
      }
      _Combine.push_back( combine );
   }

   for ( const std::vector<int>& indexes : _SectorIndexes )
   {
      std::vector<int> v;
      for ( int i = 0; i < N; i++ )
         v.push_back( _Groups[i].invert( indexes[i] ) );
      _Invert.push_back( sectorIdOf( v ) );
   }

}

int GraphSymmetry_Groups::sectorIdOf( const std::vector<int>& indexes )
{
   int sectorId = int( find( _SectorIndexes.begin(), _SectorIndexes.end(), indexes ) - _SectorIndexes.begin() );
   return sectorId == (int) _SectorIndexes.size() ? -1 : sectorId;
}

void GraphSymmetry_Groups::initSectorIndexes( int i, std::vector<int>& indexes )
{
   if ( i >= (int) _Groups.size() )
   { 
      _SectorIndexes.push_back( indexes );
      return;
   }
   for ( int k = _Groups[i].loIndex(); k < _Groups[i].hiIndex(); k++ )
   {
      indexes[i] = k;
      initSectorIndexes( i+1, indexes );
      indexes[i] = -1;
   }
}

SectorSymmetry::SectorSymmetry( const IGraphSymmetry* graphSymmetry, const XYZ& pos ) : _GraphSymmetry( graphSymmetry )
{
   int n = _GraphSymmetry->numSectors();
   
   _SectorsEquivalentTo.resize( n );
   for ( int i = 0; i < n; i++ )
      for ( int k = 0; k < n; k++ )
         if ( _GraphSymmetry->toSector( i, pos ).dist2( _GraphSymmetry->toSector( k, pos ) ) < 1e-12 )
            _SectorsEquivalentTo[i].push_back( k );

   for ( int i = 0; i < n; i++ )
      if ( _SectorsEquivalentTo[i][0] == i )
         _Sectors.push_back( i );   
}



GraphSymmetry_PlanarRotation::GraphSymmetry_PlanarRotation( int N ) : N(N) 
{
   for ( int i = 0; i < N; i++ )
   {
      _SectorMatrices.push_back( Matrix4x4::rotation( XYZ(0,0,1), 2*PI/N * i ) );
   }
}