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
   class TilePtr;
   class Vertex;

   class VertexPtr
   {
   public:
      VertexPtr() {}
      VertexPtr( const TileGraph* graph, int idx, const Matrix4x4& mtx ) : _Graph(graph), _Index(idx), _Matrix(mtx) 
      {
         _Matrix = baseVertex()._Symmetry->canonicalizedSector( mtx );
         updateCache();
      }
      CORE_API bool isValid() const { return _Graph != nullptr; }
      CORE_API VertexPtr premul( const Matrix4x4& mtx ) const { return VertexPtr( _Graph, _Index, mtx * _Matrix ); }
      CORE_API bool operator==( const VertexPtr& rhs ) const { return _Graph == rhs._Graph && _Index == rhs._Index && _SectorId == rhs._SectorId; }
      CORE_API bool operator!=( const VertexPtr& rhs ) const { return !(*this == rhs); }
      //uint64_t id() const;

      CORE_API const Vertex& baseVertex() const { return _Graph->_Vertices[_Index]; }
      CORE_API VertexPtr toVertexPtr( const TileGraph* graph ) const { return VertexPtr( graph, _Index, Matrix4x4() ); }
      CORE_API XYZ pos() const { return _Matrix * baseVertex()._Pos; }
      CORE_API int id() const { return isValid() ? _Graph->_Vertices.size() * _SectorId + _Index : -1; }
      //CORE_API std::string name() const { return std::to_string( id() ); }
      CORE_API std::string name() const { return std::to_string( _Index ) + "-" + std::to_string( _SectorId ); }
      CORE_API const Matrix4x4& matrix() const { return _Matrix; }
      CORE_API int index() const { return _Index; }
      CORE_API std::vector<TilePtr> tiles() const;
      CORE_API std::vector<VertexPtr> neighbors() const;

   public:
      void updateCache();

   private:
      const TileGraph* _Graph = nullptr;
      int _Index = -1;
      Matrix4x4 _Matrix;

   private:
      int _SectorId = -1;
   };
   class TilePtr
   {
   public:
      TilePtr() {}
      TilePtr( const TileGraph* graph, int idx, const Matrix4x4& mtx ) : _Graph(graph), _Index(idx), _Matrix(mtx) 
      {
         _Matrix = baseTile()._Symmetry->canonicalizedSector( mtx );
         updateCache();
      }
      CORE_API TilePtr premul( const Matrix4x4& mtx ) const { return TilePtr( _Graph, _Index, mtx * _Matrix ); }
      CORE_API bool isValid() const { return _Graph != nullptr; }
      //bool operator==( const TilePtr& rhs ) const;

      CORE_API const Tile& baseTile() const { return _Graph->_Tiles[_Index]; }
      CORE_API int color() const { return _Graph->_GraphSymmetry->toSector( _SectorId, baseTile()._Color ); }
      CORE_API std::vector<VertexPtr> vertices() const;
      CORE_API XYZ avgPos() const;
      CORE_API int id() const { return isValid() ? _Graph->_Tiles.size() * _SectorId + _Index : -1; }
      //CORE_API std::string name() const { return std::to_string( id() ); }
      CORE_API std::string name() const { return std::to_string( _Index ) + "-" + std::to_string( _SectorId ); }
      CORE_API VertexPtr next( const VertexPtr& a ) const { return baseTile().next( a.premul( _Matrix.inverted() ) ).premul( _Matrix ); }
      CORE_API VertexPtr prev( const VertexPtr& a ) const { return baseTile().prev( a.premul( _Matrix.inverted() ) ).premul( _Matrix ); }

   public:
      void updateCache();

   private:
      const TileGraph* _Graph = nullptr;
      int _Index = -1;
      Matrix4x4 _Matrix;

   private: // cached
      int _SectorId = -1;
   };
   class Vertex
   {
   public:
      VertexPtr toVertexPtr( const TileGraph* graph ) const { return VertexPtr( graph, _Index, Matrix4x4() ); }

   public:
      Vertex( int idx, const XYZ& pos ) : _Index(idx), _Pos(pos) {}
      int _Index;
      XYZ _Pos;
      std::vector<VertexPtr> _Neighbors;
      std::vector<TilePtr> _Tiles;
      std::shared_ptr<SectorSymmetryForVertex> _Symmetry;
   };
   class Tile
   {
   public:
      TilePtr toTilePtr( const TileGraph* graph ) const { return TilePtr( graph, _Index, Matrix4x4() ); }

      VertexPtr next( const VertexPtr& a ) const { return _Vertices[mod(verticesIndexOf(a)+1, (int)_Vertices.size())]; }
      VertexPtr prev( const VertexPtr& a ) const { return _Vertices[mod(verticesIndexOf(a)-1, (int)_Vertices.size())]; }

   private:
      int verticesIndexOf( const VertexPtr& a ) const { for ( int i = 0; i < (int) _Vertices.size(); i++ ) if ( _Vertices[i] == a ) return i; throw 777; return -1; }

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

   CORE_API Vertex& addVertex( const XYZ& pos );

   CORE_API std::vector<TilePtr> rawTiles() const;
   CORE_API std::vector<VertexPtr> rawVertices() const;
   CORE_API std::vector<TilePtr> allTiles() const;
   CORE_API std::vector<VertexPtr> allVertices() const;

   CORE_API VertexPtr vertexAt( const XYZ& pos, double maxDist ) const;
   CORE_API void setVertexPos( const VertexPtr& vtx, const XYZ& pos );

public:
   std::vector<Vertex> _Vertices;
   std::vector<Tile> _Tiles;
   std::shared_ptr<IGraphSymmetry> _GraphSymmetry;
   std::shared_ptr<IGraphShape> _GraphShape;
};

