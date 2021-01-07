#pragma once

#include "CoreMacros.h"
#include "DataTypes.h"
#include "Symmetry.h"
#include "Defs.h"

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
         if ( _SectorId < 0 )
            *this = VertexPtr();
      }

      CORE_API bool operator==( const VertexPtr& rhs ) const { return _Graph == rhs._Graph && _Index == rhs._Index && _SectorId == rhs._SectorId; }
      CORE_API bool operator!=( const VertexPtr& rhs ) const { return !(*this == rhs); }
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
      CORE_API bool                   isVisible() const { return _Graph->_GraphSymmetry->isSectorIdVisible( _SectorId ); }


      CORE_API VertexPtr next( const VertexPtr& a ) const { return baseVertex().next( a.unpremul( _Matrix ) ).premul( _Matrix ); }
      CORE_API std::vector<VertexPtr> polygon( const VertexPtr& a ) const
      {
         std::vector<VertexPtr> ret = { a, *this };
         while ( true )
         {
            VertexPtr c = ret.back().next( ret[ret.size()-2] );
            if ( !c.isValid() )
               return {};
            if ( c == ret[0] )
               return ret;
            ret.push_back( c );
         }
      }
      CORE_API int id() const { return isValid() ? MAX_VERTICES * _SectorId + _Index : -1; }
      CORE_API int index() const { return isValid() ? _Index : -1; }
      CORE_API Matrix4x4 matrix() const { return _Matrix; }
      CORE_API int sectorId() const { return _SectorId; }

      CORE_API Json toJson() const;

   public:
      void updateCache();

   public:
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
         for ( const Matrix4x4& premul : symmetry->sectorEquivalentsToIdentity() ) 
            neighbors.push_back( a.premul( premul ) ); 
      }
      void removeNeighbor( VertexPtr a ) 
      { 
         for ( const Matrix4x4& premul : symmetry->sectorEquivalentsToIdentity() ) 
         {
            size_t prevCt = neighbors.size();
            neighbors.erase( std::remove_if( neighbors.begin(), neighbors.end(), [&]( const VertexPtr& b ) { return a.premul( premul ) == b; } ), neighbors.end() ); 
            assert( neighbors.size() == prevCt - 1 );
         }
      }
      int neighborIndexOf( const VertexPtr& a ) const
      {
         for ( int i = 0; i < (int)neighbors.size(); i++ )
            if ( a == neighbors[i] )
               return i;
         return -1;
      }
      VertexPtr next( const VertexPtr& a ) const // next neighbor counter-clockwise
      {
         int indexOfA = neighborIndexOf( a );
         return indexOfA == -1 ? VertexPtr() : neighbors[(indexOfA+1)%(int)neighbors.size()];
      }
      Json toJson() const;

   public:
      int index;
      int color;
      XYZ pos;
      std::shared_ptr<SectorSymmetryForVertex> symmetry;
      std::vector<VertexPtr> neighbors;
   };

   CORE_API DualGraph( std::shared_ptr<IGraphSymmetry> symmetry, std::shared_ptr<IGraphShape> shape );

   CORE_API VertexPtr operator[]( int index ) const { return _Vertices[index].toVertexPtr( this ); }
   CORE_API VertexPtr vertexWithName( const std::string& name ) const { for ( const VertexPtr& a : allVisibleVertices() ) if ( a.name() == name ) return a; return VertexPtr(); }
   CORE_API VertexPtr vertexWithId( int id ) const { for ( const VertexPtr& a : allVisibleVertices() ) if ( a.id() == id ) return a; return VertexPtr(); }

   CORE_API std::vector<VertexPtr> allVisibleVertices() const;
   CORE_API std::vector<VertexPtr> rawVertices() const;

   CORE_API void addVertex( int color, const XYZ& pos );
   CORE_API VertexPtr vertexAt( const XYZ& pos, double maxDist ) const;
   CORE_API void setVertexColor( const VertexPtr& vtx, int color );
   CORE_API void setVertexPos( const VertexPtr& vtx, const XYZ& pos );

   CORE_API void toggleEdge( int idA, int idB );   
   CORE_API void toggleEdge( const VertexPtr& a, const VertexPtr& b );   
   CORE_API void sortNeighbors();

   CORE_API void normalizeVertices();

   CORE_API std::shared_ptr<IGraphShape> shape() { return _GraphShape; }

   CORE_API Json toJson() const;

public:
   std::vector<Vertex> _Vertices;
   std::shared_ptr<IGraphSymmetry> _GraphSymmetry;
   std::shared_ptr<IGraphShape> _GraphShape;
};

