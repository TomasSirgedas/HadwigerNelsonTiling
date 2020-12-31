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
   public:
      VertexPtr() {}
      VertexPtr( const DualGraph* graph, int index, int sectorId ) : graph(graph), index(index), sectorId(sectorId) 
      {
      }

      CORE_API bool operator==( const VertexPtr& rhs ) const { return graph == rhs.graph && index == rhs.index && sectorId == rhs.sectorId; }
      CORE_API bool operator<( const VertexPtr& rhs ) const { return sectorId != rhs.sectorId ? sectorId < rhs.sectorId : index < rhs.index; }
      CORE_API bool operator>( const VertexPtr& rhs ) const { return rhs < *this; }

      CORE_API bool isValid() const { return graph != nullptr; }
      CORE_API XYZ  pos() const;
      CORE_API int  color() const;
      CORE_API std::string name() const;

      CORE_API VertexPtr              withSectorId( int sectorId ) const { VertexPtr ret = *this; ret.sectorId = baseVertex().symmetry->canonicalizedSector( sectorId ); return ret; }
      CORE_API VertexPtr              premul( int sectorId ) const;
      CORE_API VertexPtr              unpremul( int sectorId ) const;
      CORE_API std::vector<VertexPtr> neighbors() const;

   private:
      const Vertex& baseVertex() const;

   public:
      const DualGraph* graph = nullptr;
      int index = -1;
      int sectorId = -1;
   };

   class Vertex
   {
   public:
      Vertex( int index, int color, const XYZ& pos ) : index(index), color(color), pos(pos) {}

      VertexPtr canonicalizedNeighbor( const VertexPtr& a ) const { return a.withSectorId( symmetry->canonicalizedSector( a.sectorId ) ); }
      bool hasNeighbor( VertexPtr a ) const { a = canonicalizedNeighbor( a ); for ( const VertexPtr& neighb : neighbors ) if ( neighb == a ) return true; return false; }
      void addNeighbor( VertexPtr a ) { a = canonicalizedNeighbor( a ); neighbors.push_back( a ); }
      void removeNeighbor( VertexPtr a ) 
      { 
         a = canonicalizedNeighbor( a ); 
         size_t prevCt = neighbors.size();
         neighbors.erase( std::remove_if( neighbors.begin(), neighbors.end(), [&]( const VertexPtr& b ) { return a == b; } ), neighbors.end() ); 
         assert( neighbors.size() == prevCt - 1 );
      }

   public:
      int index;
      int color;
      XYZ pos;
      std::shared_ptr<SectorSymmetry> symmetry;
      std::vector<VertexPtr> neighbors;
   };

   CORE_API DualGraph( std::shared_ptr<IGraphSymmetry> symmetry );

   CORE_API std::vector<VertexPtr> allVertices() const;

   CORE_API void addVertex( int color, const XYZ& pos );
   CORE_API VertexPtr vertexAt( const XYZ& pos, double maxDist ) const;
   CORE_API void setVertexColor( const VertexPtr& vtx, int color );
   CORE_API void setVertexPos( const VertexPtr& vtx, const XYZ& pos );
            
   CORE_API void toggleEdge( const VertexPtr& a, const VertexPtr& b );   

private:
   std::vector<Vertex> _Vertices;
   std::shared_ptr<IGraphSymmetry> _GraphSymmetry;
};

