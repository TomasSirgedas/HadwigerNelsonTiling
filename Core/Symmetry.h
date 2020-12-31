#pragma once

#include "CoreMacros.h"
#include "DataTypes.h"
#include "Util.h"

#include <vector>
#include <string>
#include <memory>

class IGraphSymmetry;


struct Sector
{
   Sector with0AtIndex( int index ) const { if ( index < 0 ) return *this; Sector ret = *this; ret[index] = 0; return ret; }
   std::string name() const { std::string ret; for ( int x : v ) ret += std::to_string( x ); return ret; }
   int& operator[]( int index ) { return v[index]; }
   int operator[]( int index ) const { return v[index]; }
   bool operator==( const Sector& rhs ) const { return v == rhs.v; }
   bool operator!=( const Sector& rhs ) const { return !(*this == rhs); }
   bool operator<( const Sector& rhs ) const { return v < rhs.v; }

   std::vector<int> v;
};

class SectorSymmetryForVertex
{
public:
   SectorSymmetryForVertex( const IGraphSymmetry* _GraphSymmetry, const XYZ& pos );

   std::vector<Sector> sectors() const { return _Sectors; }
   std::vector<Sector> sector0Equivalents() const { return _Sector0Equivalents; }
   Sector canonicalizedSector( const Sector& sector ) const;
   bool hasSymmetry() const { return _Sector0Equivalents.size() > 1; }

private:
   const IGraphSymmetry* _GraphSymmetry;
   std::vector<Sector> _Sector0Equivalents;
   std::vector<Sector> _Sectors;
   
};

class IGraphSymmetry
{
public:
   virtual int numSectors() const = 0;
   //virtual int numVisibleSectors() const = 0;
   virtual XYZ toSector( const Sector& sector, const XYZ& pos ) const = 0;
   virtual XYZ fromSector( const Sector& sector, const XYZ& pos ) const = 0;
   virtual int toSector( const Sector& sector, int color ) const = 0;
   virtual int fromSector( const Sector& sector, int color ) const = 0;
   virtual Sector combineSectors( const Sector& a, const Sector& b ) const = 0;
   virtual Sector invertSector( const Sector& sector ) const = 0;
   virtual std::string sectorName( const Sector& sector ) const { return sector == sector0() ? std::string() : sector.name(); }
   virtual Sector sector0() const = 0;
   virtual std::vector<Sector> allSectors() const = 0;

   std::shared_ptr<SectorSymmetryForVertex> calcSectorSymmetry( const XYZ& pos ) const { return std::shared_ptr<SectorSymmetryForVertex>( new SectorSymmetryForVertex( this, pos ) ); }
};

class SymmetryGroup
{
public:
   CORE_API SymmetryGroup( const Matrix4x4& matrix, const Perm& colorPerm ) : _Matrix(matrix), _ColorPerm(colorPerm) { init( 0, 0 ); }
   CORE_API SymmetryGroup( const Matrix4x4& matrix, const Perm& colorPerm, int loIndex, int hiIndex ) : _Matrix(matrix), _ColorPerm(colorPerm) { init( loIndex, hiIndex ); }
   CORE_API int loIndex() const { return _LoIndex; }
   CORE_API int hiIndex() const { return _HiIndex; }
   
   CORE_API int canonicalizedIndex( int index ) const { return _IsFinite ? mod( index - _LoIndex, size() ) + _LoIndex : index; }
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
   void init( int loIndexHint, int hiIndexHint );

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
   CORE_API int numSectors() const override { int ret = 1; for ( const SymmetryGroup& g : _Groups ) ret *= g.size(); return ret; }
   CORE_API XYZ toSector( const Sector& sector, const XYZ& pos ) const override { XYZ p = pos; for ( int i = (int) _Groups.size()-1; i >= 0; i-- ) p = ( _Groups[i].matrix( sector[i] ) * p ).toXYZ(); return p; }
   CORE_API XYZ fromSector( const Sector& sector, const XYZ& pos ) const override { return toSector( invertSector( sector ), pos ); }
   CORE_API int toSector( const Sector& sector, int color ) const override { int c = color; for ( int i = (int) _Groups.size()-1; i >= 0; i-- ) c = _Groups[i].colorPerm( sector[i] )[c]; return c; }
   CORE_API int fromSector( const Sector& sector, int color ) const override { return toSector( invertSector( sector ), color ); }
   CORE_API Sector combineSectors( const Sector& a, const Sector& b ) const override { Sector ret = sector0(); for ( int i = 0; i < (int) _Groups.size(); i++ ) ret[i] = _Groups[i].combine( a[i], b[i] ); return ret; }
   CORE_API Sector invertSector( const Sector& sector ) const override { Sector ret = sector0(); for ( int i = 0; i < (int) _Groups.size(); i++ ) ret[i] = _Groups[i].invert( sector[i] ); return ret; }
   CORE_API Sector sector0() const override { return Sector { std::vector<int>( _Groups.size(), 0 ) }; }
   CORE_API std::vector<Sector> allSectors() const override { return _AllSectors; }

private:
   void initAllSectors( int i, Sector& sector );

public:
   std::vector<SymmetryGroup> _Groups;
   std::vector<Sector> _AllSectors;
};
