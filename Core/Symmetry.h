#pragma once

#include "CoreMacros.h"
#include "DataTypes.h"
#include "Util.h"

#include <vector>
#include <string>
#include <memory>

class IGraphSymmetry;



class SectorSymmetry
{
public:
   SectorSymmetry( const IGraphSymmetry* _GraphSymmetry, const XYZ& pos );

   std::vector<int> sectors() const { return _Sectors; }
   std::vector<int> sectorsEquivalentTo( int sectorId ) const { return _SectorsEquivalentTo[sectorId]; }
   int canonicalizedSector( int sectorId ) const { return _SectorsEquivalentTo[sectorId][0]; }

private:
   const IGraphSymmetry* _GraphSymmetry;
   std::vector<int> _Sectors;
   std::vector<std::vector<int>> _SectorsEquivalentTo;
};

class IGraphSymmetry
{
public:
   virtual int numSectors() const = 0;
   virtual XYZ toSector( int sectorId, const XYZ& pos ) const = 0;
   virtual XYZ fromSector( int sectorId, const XYZ& pos ) const = 0;
   virtual int toSector( int sectorId, int color ) const = 0;
   virtual int fromSector( int sectorId, int color ) const = 0;
   virtual int combineSectors( int a, int b ) const = 0;
   virtual int invertSector( int a ) const = 0;
   virtual std::string sectorName( int sectorId ) const { return ""; }

   std::shared_ptr<SectorSymmetry> calcSectorSymmetry( const XYZ& pos ) const { return std::shared_ptr<SectorSymmetry>( new SectorSymmetry( this, pos ) ); }
};

class SymmetryGroup
{
public:
   CORE_API SymmetryGroup( const Matrix4x4& matrix, const Perm& colorPerm ) : _Matrix(matrix), _ColorPerm(colorPerm) { init(); }
   CORE_API SymmetryGroup( const Matrix4x4& matrix, const Perm& colorPerm, int loIndex, int hiIndex ) : _Matrix(matrix), _ColorPerm(colorPerm), _LoIndex(loIndex), _HiIndex(hiIndex) { init(); }
   CORE_API int loIndex() const { return _LoIndex; }
   CORE_API int hiIndex() const { return _HiIndex; }

   CORE_API int size() const { return _HiIndex - _LoIndex; }
   CORE_API Matrix4x4 matrix( int index ) const { return _Matrix.pow( index ); }
   CORE_API Perm colorPerm( int index ) const { return _ColorPerm.pow( index ); }
   CORE_API int combine( int indexA, int indexB ) const { return indexOf( matrix( indexA ) * matrix( indexB ) ); }
   CORE_API int invert( int index ) const { return indexOf( matrix( index ).inverted() ); }
   CORE_API int indexOf( const Matrix4x4& rhs ) const
   {
      for ( int i = _LoIndex; i < _HiIndex; i++ )
         if ( rhs.eq( matrix( i ) ) )
            return i;
      return 999999;
   }

private:
   void init();

private:
   Matrix4x4 _Matrix;
   Perm      _ColorPerm;
   int       _LoIndex = 0; // inclusive
   int       _HiIndex = 0; // exclusive
   bool      _IsFinite;
};

class GraphSymmetry_Groups : public IGraphSymmetry
{
public:
   CORE_API GraphSymmetry_Groups( const std::vector<SymmetryGroup>& groups );
   CORE_API int numSectors() const override { return (int) _SectorMatrices.size(); }
   CORE_API XYZ toSector( int sectorId, const XYZ& pos ) const override { return ( _SectorMatrices[sectorId] * pos ).toXYZ(); }
   CORE_API XYZ fromSector( int sectorId, const XYZ& pos ) const override { return ( _SectorMatrices[sectorId].inverted() * pos ).toXYZ(); }
   CORE_API int toSector( int sectorId, int color ) const override { return _ColorPerms[sectorId][color]; }
   CORE_API int fromSector( int sectorId, int color ) const override { return _ColorPermsInv[sectorId][color]; }
   CORE_API int combineSectors( int a, int b ) const override { return _Combine[a][b]; }
   CORE_API int invertSector( int a ) const override { return _Invert[a]; }
   CORE_API std::string sectorName( int sectorId ) const { return sectorId > 0 ? "-" + std::to_string( sectorId ) : ""; }

private:
   void initSectorIndexes( int i, std::vector<int>& indexes );
   int sectorIdOf( const std::vector<int>& indexes );

public:
   std::vector<SymmetryGroup> _Groups;
   std::vector<std::vector<int>> _SectorIndexes;
   std::vector<Matrix4x4> _SectorMatrices;
   std::vector<Perm> _ColorPerms;
   std::vector<Perm> _ColorPermsInv;
   std::vector<std::vector<int>> _Combine;
   std::vector<int> _Invert;
};

class GraphSymmetry_PlanarRotation : public IGraphSymmetry
{
public:
   CORE_API GraphSymmetry_PlanarRotation( int N );

   int numSectors() const override { return N; }
   XYZ toSector( int sectorId, const XYZ& pos ) const override { return ( _SectorMatrices[sectorId] * pos ).toXYZ(); }
   XYZ fromSector( int sectorId, const XYZ& pos ) const override { return ( _SectorMatrices[sectorId].inverted() * pos ).toXYZ(); }
   int toSector( int sectorId, int color ) const override { return color < N ? mod( color + sectorId, N ) : color; }
   int fromSector( int sectorId, int color ) const override { return color < N ? mod( color - sectorId, N ) : color; }
   int combineSectors( int a, int b ) const override { return (a + b) % N; }
   int invertSector( int a ) const override { return mod( -a, N ); }
   std::string sectorName( int sectorId ) const override { return "-" + std::to_string( sectorId ); }

public:
   int N;
   std::vector<Matrix4x4> _SectorMatrices;
   //std::vector<std::shared_ptr<ISector>> sectors;
};