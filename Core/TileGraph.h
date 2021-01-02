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
   class Tile;
   class Vertex;

   class VertexPtr
   {
   public:
      VertexPtr() {}
      VertexPtr( const TileGraph* graph, int idx, const Matrix4x4& mtx ) : _Graph(graph), _Index(idx), _Matrix(mtx) {}
      CORE_API bool isValid() const { return _Graph != nullptr; }
      CORE_API VertexPtr premul( const Matrix4x4& mtx ) const { return VertexPtr( _Graph, _Index, mtx * _Matrix ); }
      CORE_API bool operator==( const VertexPtr& rhs ) const;
      //uint64_t id() const;

      CORE_API const Vertex& baseVertex() const { return _Graph->_Vertices[_Index]; }
      CORE_API VertexPtr toVertexPtr( const TileGraph* graph ) const { return VertexPtr( graph, _Index, Matrix4x4() ); }
      CORE_API XYZ pos() const { return _Matrix * baseVertex()._Pos; }

   private:
      const TileGraph* _Graph = nullptr;
      int _Index = -1;
      Matrix4x4 _Matrix;
   };
   class TilePtr
   {
   public:
      TilePtr() {}
      TilePtr( const TileGraph* graph, int idx, const Matrix4x4& mtx ) : _Graph(graph), _Index(idx), _Matrix(mtx) {}
      CORE_API TilePtr premul( const Matrix4x4& mtx ) const { return TilePtr( _Graph, _Index, mtx * _Matrix ); }
      CORE_API bool isValid() const { return _Graph != nullptr; }
      //bool operator==( const TilePtr& rhs ) const;

      CORE_API const Tile& baseTile() const { return _Graph->_Tiles[_Index]; }
      CORE_API int color() const { return 0; }
      CORE_API std::vector<VertexPtr> vertices() const;

   private:
      const TileGraph* _Graph = nullptr;
      int _Index = -1;
      Matrix4x4 _Matrix;
   };
   class Vertex
   {
   public:
      VertexPtr toVertexPtr( const TileGraph* graph ) const { return VertexPtr( graph, _Index, Matrix4x4() ); }

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
      TilePtr toTilePtr( const TileGraph* graph ) const { return TilePtr( graph, _Index, Matrix4x4() ); }

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

   CORE_API std::vector<TilePtr> rawTiles() const;
   CORE_API std::vector<TilePtr> allTiles() const;

public:
   std::vector<Vertex> _Vertices;
   std::vector<Tile> _Tiles;
   std::shared_ptr<IGraphSymmetry> _GraphSymmetry;
   std::shared_ptr<IGraphShape> _GraphShape;
};

