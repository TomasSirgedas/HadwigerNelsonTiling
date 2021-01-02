#pragma once

#include "CoreMacros.h"
#include "DataTypes.h"
#include "Symmetry.h"

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <cassert>

class TileGraph
{
public:
   class VertexPtr
   {
   public:
      VertexPtr() {}
      VertexPtr( int idx, const Matrix4x4& mtx ) : _Index(idx), _Mtx(mtx) {}
      bool isValid() const { return _Index >= 0; }
      VertexPtr premul( const Matrix4x4& mtx ) const { return VertexPtr( _Index, mtx * _Mtx ); }
      bool operator==( const VertexPtr& rhs ) const;
      uint64_t id() const;

      int _Index = -1;
      Matrix4x4 _Mtx;
   };
   class TilePtr
   {
   public:
      TilePtr() {}
      TilePtr( int idx, const Matrix4x4& mtx ) : _Index(idx), _Mtx(mtx) {}
      TilePtr premul( const Matrix4x4& mtx ) const { return TilePtr( _Index, mtx * _Mtx ); }
      bool isValid() const { return _Index >= 0; }
      //bool operator==( const TilePtr& rhs ) const;
      int _Index = -1;
      Matrix4x4 _Mtx;
   };
   class Vertex
   {
   public:
      Vertex( int idx ) : _Index(idx) {}
      int _Index;
      //bool _IsSymmetrical;
      XYZ _Pos;
      //std::vector<VertexPtr> _Neighbors;
      //std::vector<TilePtr> _Tiles;
   };
   class Tile
   {
   public:
      int _Index;
      int _Color;
      std::vector<VertexPtr> _Vertices;
      std::shared_ptr<SectorSymmetryForVertex> _Symmetry;
      //bool _IsSymmetrical = false;
      //std::shared_ptr<MatrixSymmetryMap> _SymmetryMap;

      //bool hasVertex( const VertexPtr& a ) const { 
      //   for ( const VertexPtr& b : _Vertices )
      //      if ( a._Index == b._Index && fuzzyCompare( a._Mtx, b._Mtx ) )
      //         return true;
      //   return false;
      //}
   };
   struct KeepCloseFar
   {
      VertexPtr a;
      VertexPtr b;
      bool keepClose;
      bool keepFar;
   };
   // these two shouldn't get too close together
   // - line[a0,a1] curves centered on curveCenter
   // - vertex b
   struct LineVertexConstraint
   {
      VertexPtr a0;  
      VertexPtr a1;
      VertexPtr curveCenter;
      VertexPtr b;
   };



public:
   std::vector<Vertex> _Vertices;
   std::vector<Tile> _Tiles;
   std::shared_ptr<IGraphSymmetry> _GraphSymmetry;
   std::shared_ptr<IGraphShape> _GraphShape;
};

