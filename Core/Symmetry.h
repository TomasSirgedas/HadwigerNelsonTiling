#pragma once

#include "CoreMacros.h"
#include "DataTypes.h"
#include "Util.h"

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class IGraphSymmetry;


//struct Sector
//{
//   Sector with0AtIndex( int index ) const { if ( index < 0 ) return *this; Sector ret = *this; ret[index] = 0; return ret; }
//   std::string name() const { std::string ret; for ( int x : v ) ret += std::to_string( x ); return ret; }
//   int& operator[]( int index ) { return v[index]; }
//   int operator[]( int index ) const { return v[index]; }
//   bool operator==( const Sector& rhs ) const { return v == rhs.v; }
//   bool operator!=( const Sector& rhs ) const { return !(*this == rhs); }
//   bool operator<( const Sector& rhs ) const { return v < rhs.v; }
//
//   std::vector<int> v;
//};


class SectorSymmetryForVertex
{
public:
   SectorSymmetryForVertex( const IGraphSymmetry* _GraphSymmetry, const XYZ& pos );

   std::vector<Matrix4x4> uniqueSectors() const { return _UniqueSectors; }
   std::vector<Matrix4x4> sectorEquivalents( int sectorId ) const { return _EquivalentSectors[sectorId]; }
   Matrix4x4 canonicalizedSector( const Matrix4x4& sector ) const;
   bool hasSymmetry() const { return _EquivalentSectors[0].size() > 1; }

private:
   const IGraphSymmetry* _GraphSymmetry;
   std::vector<std::vector<int>> _EquivalentSectorIds;
   std::vector<std::vector<Matrix4x4>> _EquivalentSectors;
   std::vector<Matrix4x4> _UniqueSectors;
};

class IGraphSymmetry
{
public:
   virtual int numSectors() const = 0;
   virtual int sectorId( const Matrix4x4& sector ) const = 0;
   virtual int toSector( int sectorId, int color ) const = 0;
   virtual int fromSector( int sectorId, int color ) const = 0;
   virtual std::string sectorName( int sectorId ) const = 0;
   //virtual std::vector<Matrix4x4> allVisibleSectors() const = 0;
   virtual std::vector<Matrix4x4> allSectors() const = 0;

   std::shared_ptr<SectorSymmetryForVertex> calcSectorSymmetry( const XYZ& pos ) const { return std::shared_ptr<SectorSymmetryForVertex>( new SectorSymmetryForVertex( this, pos ) ); }
};

class SymmetryGroup
{
public:
   CORE_API SymmetryGroup( const Matrix4x4& matrix, const Perm& colorPerm ) : _Matrix(matrix), _ColorPerm(colorPerm) { init( 0, 0 ); }
   CORE_API SymmetryGroup( const Matrix4x4& matrix, const Perm& colorPerm, int visibleLoIndex, int visibleHiIndex ) : _Matrix(matrix), _ColorPerm(colorPerm) { init( visibleLoIndex, visibleHiIndex ); }
   CORE_API int visibleLoIndex() const { return _VisibleLoIndex; }
   CORE_API int visibleHiIndex() const { return _VisibleHiIndex; }
   CORE_API int loIndex() const { return _LoIndex; }
   CORE_API int hiIndex() const { return _HiIndex; }
   
   CORE_API int canonicalizedIndex( int index ) const { return _IsFinite ? mod( index - _LoIndex, size() ) + _LoIndex : index; }
   //CORE_API int visibleSize() const { return _VisibleHiIndex - _VisibleLoIndex; }
   CORE_API int size() const { return _HiIndex - _LoIndex; }
   CORE_API Matrix4x4 matrix( int index ) const { return _Matrix.pow( canonicalizedIndex( index ) ); }
   CORE_API Perm colorPerm( int index ) const { return _ColorPerm.pow( canonicalizedIndex( index ) ); }
   //CORE_API int combine( int indexA, int indexB ) const { return indexOf( matrix( indexA ) * matrix( indexB ) ); }
   CORE_API int combine( int indexA, int indexB ) const { return canonicalizedIndex( indexA + indexB ); }
   CORE_API int invert( int index ) const { return canonicalizedIndex( -index ); }
   //CORE_API int indexOf( const Matrix4x4& rhs ) const
   //{
   //   for ( int i = _LoIndex; i < _HiIndex; i++ )
   //      if ( rhs.eq( matrix( i ) ) )
   //         return i;
   //   return 999999;
   //}

private:
   void init( int visibleLoIndex, int visibleHiIndex );

private:
   Matrix4x4 _Matrix;
   Perm      _ColorPerm;
   int       _LoIndex = 0; // inclusive
   int       _HiIndex = 0; // exclusive
   int       _VisibleLoIndex = 0; // inclusive
   int       _VisibleHiIndex = 0; // exclusive
   bool      _IsFinite;
};

class GraphSymmetry_Groups : public IGraphSymmetry
{
public:

   CORE_API GraphSymmetry_Groups( const std::vector<SymmetryGroup>& groups );
   CORE_API int numSectors() const override { int ret = 1; for ( const SymmetryGroup& g : _Groups ) ret *= g.size(); return ret; }
   CORE_API int sectorId( const Matrix4x4& sector ) const override { return _SectorHashToId.at( matrixHash( sector ) ); }
   CORE_API int toSector( int sectorId, int color ) const override { return _SectorIdToColorPerm[sectorId][color]; }
   CORE_API int fromSector( int sectorId, int color ) const override { return _SectorIdToColorPerm[sectorId].inverted()[color]; }
   CORE_API std::string sectorName( int sectorId ) const { return _SectorIdToName[sectorId]; }
   //CORE_API std::vector<Matrix4x4> allVisibleSectors() const override { return _AllVisibleSectors; }
   CORE_API std::vector<Matrix4x4> allSectors() const override { return _SectorIdToMatrix; }

private:
   void initAllSectorGroupIndexes( int i, std::vector<int>& groupIndexes );

public:
   std::vector<SymmetryGroup> _Groups;
   std::unordered_map<uint64_t, int> _SectorHashToId;
   //std::vector<Matrix4x4> _AllVisibleSectors;
   std::vector<std::vector<int>> _AllSectorGroupIndexes;
   std::vector<Matrix4x4> _SectorIdToMatrix;
   std::vector<Perm> _SectorIdToColorPerm;
   std::vector<std::string> _SectorIdToName;
};



class IGraphShape
{
public:
   virtual bool toSurfaceFrom2D( const XYZ& p, XYZ& surfacePoint ) const = 0;
   virtual XYZ toSurfaceFrom3D( const XYZ& p ) const = 0;
   virtual double modelSize() const = 0;
   virtual XYZ normalAt( const XYZ& p ) const = 0;
};

class GraphShapeSphere : public IGraphShape
{
public:
   GraphShapeSphere( double radius ) : _Radius( radius ) {}
   bool toSurfaceFrom2D( const XYZ& p, XYZ& surfacePoint ) const override
   {
      double z2 = _Radius*_Radius - p.x*p.x - p.y*p.y;
      if ( z2 <= 0 )
         return false;
      surfacePoint = XYZ( p.x, p.y, -sqrt( z2 ) );
      return true;
   }
   XYZ toSurfaceFrom3D( const XYZ& p ) const override { return p.normalized() * _Radius; }
   double modelSize() const override { return _Radius; }
   XYZ normalAt( const XYZ& p ) const override { return p.normalized(); }

private:
   double _Radius;
};


class GraphShapePlane : public IGraphShape
{
public:
   GraphShapePlane() {}
   bool toSurfaceFrom2D( const XYZ& p, XYZ& surfacePoint ) const override
   {
      surfacePoint = XYZ( p.x, p.y, 0. );
      return true;
   }
   XYZ toSurfaceFrom3D( const XYZ& p ) const override { return XYZ( p.x, p.y, 0 ); }
   double modelSize() const override { return 0; }
   XYZ normalAt( const XYZ& p ) const override { return XYZ( 0, 0, -1 ); }
};