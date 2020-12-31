#include "Symmetry.h"

#include <cassert>
#include <set>
#include <algorithm>

void SymmetryGroup::init( int loIndexHint, int hiIndexHint )
{
   if ( size() == 0 )
   {
      const int MAX_SIZE = 100;
      Matrix4x4 m = _Matrix;
      for ( _HiIndex = 1; !m.eq( Matrix4x4() ) && _HiIndex <= MAX_SIZE; _HiIndex++ )
         m = _Matrix * m;

      if ( _HiIndex > MAX_SIZE )
      {
         _LoIndex = loIndexHint;
         _HiIndex = hiIndexHint;
         _IsFinite = false;
         assert( size() > 0 );
      }
      else
      {
         _IsFinite = true;
         assert( _ColorPerm.pow( _HiIndex ).isIdentity() ); // make sure color group permutation is valid
      }
   }
}

GraphSymmetry_Groups::GraphSymmetry_Groups( const std::vector<SymmetryGroup>& groups ) : _Groups( groups )
{
   int N = (int) _Groups.size();
   {
      Sector sector = { std::vector<int>( N, -1 ) };
      initAllSectors( 0, sector );
   }
}

void GraphSymmetry_Groups::initAllSectors( int i, Sector& sector )
{
   if ( i >= (int) _Groups.size() )
   { 
      _AllSectors.push_back( sector );
      return;
   }
   for ( int k = _Groups[i].loIndex(); k < _Groups[i].hiIndex(); k++ )
   {
      sector[i] = k;
      initAllSectors( i+1, sector );
      sector[i] = -1;
   }
}

SectorSymmetryForVertex::SectorSymmetryForVertex( const IGraphSymmetry* graphSymmetry, const XYZ& pos ) : _GraphSymmetry( graphSymmetry )
{
   int n = _GraphSymmetry->numSectors();
   
   Sector sector0 = _GraphSymmetry->sector0();
   XYZ sector0Pos = _GraphSymmetry->toSector( sector0, pos );
   assert( pos == sector0Pos );

   for ( const Sector& a : _GraphSymmetry->allSectors() )
      if ( _GraphSymmetry->toSector( a, pos ).dist2( sector0Pos ) < 1e-12 )
         _Sector0Equivalents.push_back( a );

   std::set<Sector> usedSectors;
   for ( const Sector& a : _GraphSymmetry->allSectors() ) if ( !usedSectors.count( a ) )
   {
      _Sectors.push_back( a );
      for ( const Sector& b : _Sector0Equivalents )
         usedSectors.insert( _GraphSymmetry->combineSectors( a, b ) );
   }
}


Sector SectorSymmetryForVertex::canonicalizedSector( const Sector& sector ) const
{
   if ( !hasSymmetry() )
      return sector;

   std::vector<Sector> v;
   for ( const Sector& b : _Sector0Equivalents )
      v.push_back( _GraphSymmetry->combineSectors( sector, b ) );
   Sector ret = *std::min_element( v.begin(), v.end() );
   return ret;
}

