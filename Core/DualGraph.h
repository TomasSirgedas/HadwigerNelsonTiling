#pragma once

#include "CoreMacros.h"
#include "DataTypes.h"
#include "Symmetry.h"

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <cassert>


class DualGraph
{
public:
   class Vertex;

   class VertexPtr
   {
      friend DualGraph;
   public:
      VertexPtr() {}
      VertexPtr( const DualGraph* graph, int index, const Matrix4x4& matrix ) 
         : _Graph(graph)
         , _Index(index)
         , _Matrix(matrix)
      {
         _Matrix = baseVertex().symmetry->canonicalizedSector( matrix );
         updateCache();
      }

      CORE_API bool operator==( const VertexPtr& rhs ) const { return _Graph == rhs._Graph && _Index == rhs._Index && _SectorId == rhs._SectorId; }
      CORE_API bool operator<( const VertexPtr& rhs ) const { return _SectorId != rhs._SectorId ? _SectorId < rhs._SectorId : _Index < rhs._Index; }
      CORE_API bool operator>( const VertexPtr& rhs ) const { return rhs < *this; }

      CORE_API bool isValid() const { return _Graph != nullptr; }
      CORE_API XYZ  pos() const;
      CORE_API int  color() const;
      CORE_API std::string name() const;

      CORE_API VertexPtr              withMatrix( const Matrix4x4& mtx ) const { return VertexPtr( _Graph, _Index, mtx ); }
      CORE_API VertexPtr              premul( const Matrix4x4& mtx ) const;
      CORE_API VertexPtr              unpremul( const Matrix4x4& mtx ) const;
      CORE_API std::vector<VertexPtr> neighbors() const;

   public:
      void updateCache();

   private:
      const Vertex& baseVertex() const;

   private:
      const DualGraph* _Graph = nullptr;
      int _Index = -1;
      Matrix4x4 _Matrix;

   private: // cached
      int _SectorId = -1;
   };

   class Vertex
   {
   public:
      Vertex( int index, int color, const XYZ& pos ) : index(index), color(color), pos(pos) {}

      VertexPtr toVertexPtr( const DualGraph* graph ) const { return VertexPtr( graph, index, Matrix4x4() ); }

      //VertexPtr canonicalizedNeighbor( const VertexPtr& a ) const { return a; /*return a.withMatrix( symmetry->canonicalizedSector( a._Matrix ) );*/ }
      bool hasNeighbor( VertexPtr a ) const { for ( const VertexPtr& neighb : neighbors ) if ( neighb == a ) return true; return false; }
      void addNeighbor( VertexPtr a ) 
      { 
         for ( const Matrix4x4& premul : symmetry->sectorEquivalents( 0 ) ) 
            neighbors.push_back( a.premul( premul ) ); 
      }
      void removeNeighbor( VertexPtr a ) 
      { 
         for ( const Matrix4x4& premul : symmetry->sectorEquivalents( 0 ) ) 
         {
            size_t prevCt = neighbors.size();
            neighbors.erase( std::remove_if( neighbors.begin(), neighbors.end(), [&]( const VertexPtr& b ) { return a.premul( premul ) == b; } ), neighbors.end() ); 
            assert( neighbors.size() == prevCt - 1 );
         }
      }

   public:
      int index;
      int color;
      XYZ pos;
      std::shared_ptr<SectorSymmetryForVertex> symmetry;
      std::vector<VertexPtr> neighbors;
   };

   CORE_API DualGraph( std::shared_ptr<IGraphSymmetry> symmetry, std::shared_ptr<IGraphShape> shape );

   CORE_API std::vector<VertexPtr> allVisibleVertices() const;

   CORE_API void addVertex( int color, const XYZ& pos );
   CORE_API VertexPtr vertexAt( const XYZ& pos, double maxDist ) const;
   CORE_API void setVertexColor( const VertexPtr& vtx, int color );
   CORE_API void setVertexPos( const VertexPtr& vtx, const XYZ& pos );
            
   CORE_API void toggleEdge( const VertexPtr& a, const VertexPtr& b );   


   CORE_API std::shared_ptr<IGraphShape> shape() { return _GraphShape; }

private:
   std::vector<Vertex> _Vertices;
   std::shared_ptr<IGraphSymmetry> _GraphSymmetry;
   std::shared_ptr<IGraphShape> _GraphShape;
};

