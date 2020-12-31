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
      VertexPtr( const DualGraph* graph, int index, const Sector& sector ) : graph(graph), index(index), sector(sector)
      {
      }

      CORE_API bool operator==( const VertexPtr& rhs ) const { return graph == rhs.graph && index == rhs.index && sector == rhs.sector; }
      CORE_API bool operator<( const VertexPtr& rhs ) const { return sector != rhs.sector ? sector < rhs.sector : index < rhs.index; }
      CORE_API bool operator>( const VertexPtr& rhs ) const { return rhs < *this; }

      CORE_API bool isValid() const { return graph != nullptr; }
      CORE_API XYZ  pos() const;
      CORE_API int  color() const;
      CORE_API std::string name() const;

      CORE_API VertexPtr              withSectorId( const Sector& sector ) const { VertexPtr ret = *this; ret.sector = baseVertex().symmetry->canonicalizedSector( sector ); return ret; }
      CORE_API VertexPtr              premul( const Sector& sector ) const;
      CORE_API VertexPtr              unpremul( const Sector& sector ) const;
      CORE_API std::vector<VertexPtr> neighbors() const;

   private:
      const Vertex& baseVertex() const;

   public:
      const DualGraph* graph = nullptr;
      int index = -1;
      Sector sector;
   };

   class Vertex
   {
   public:
      Vertex( int index, int color, const XYZ& pos ) : index(index), color(color), pos(pos) {}

      VertexPtr canonicalizedNeighbor( const VertexPtr& a ) const { return a.withSectorId( symmetry->canonicalizedSector( a.sector ) ); }
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
      std::shared_ptr<SectorSymmetryForVertex> symmetry;
      std::vector<VertexPtr> neighbors;
   };

   CORE_API DualGraph( std::shared_ptr<IGraphSymmetry> symmetry, std::shared_ptr<IGraphShape> shape );

   CORE_API std::vector<VertexPtr> allVertices() const;

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

