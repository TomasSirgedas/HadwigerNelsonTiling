#include "DualGraph.h"
#include "trace.h"

class SwapNum
{
public:
   SwapNum( int a, int b ) : _a(a), _b(b) {}
   int operator[]( int x ) const { return x == _a ? _b : x == _b ? _a : x; }
private:
   int _a, _b;
};

const DualGraph::Vertex& DualGraph::VertexPtr::baseVertex() const { return _Graph->_Vertices[_Index]; }
XYZ DualGraph::VertexPtr::pos() const { return _SectorId.matrix() * baseVertex().pos; };
int DualGraph::VertexPtr::color() const { return _SectorId.mapColor( baseVertex().color ); }
std::string DualGraph::VertexPtr::name() const 
{ 
   //std::string sectorName = _Graph->_GraphSymmetry->sectorName( _SectorId );
   //return std::to_string( _Index ) + (sectorName.empty() ? "" : "-") + sectorName;
   //return std::to_string( _Index ) + "-" + std::to_string( _SectorId );
   return std::to_string( id() );
}

std::vector<DualGraph::VertexPtr> DualGraph::VertexPtr::neighbors() const
{
   std::vector<DualGraph::VertexPtr> ret;
   for ( const DualGraph::VertexPtr& baseNeighb : baseVertex().neighbors )
   {
      ret.push_back( baseNeighb.premul( _SectorId ) );
      //DualGraph::VertexPtr neighb = baseNeighb.premul( _Matrix );
      //for ( const Matrix4x4& otherSector : baseVertex().symmetry->sector0Equivalents() )
      //   ret.push_back( neighb.premul( otherSector ) );
   }
   return ret;
}

DualGraph::VertexPtr DualGraph::VertexPtr::premul( const SectorId& mtx ) const
{
   return withSectorId( mtx * _SectorId );
}

DualGraph::VertexPtr DualGraph::VertexPtr::unpremul( const SectorId& mtx ) const
{
   return withSectorId( mtx.inverted() * _SectorId );
}


DualGraph::DualGraph( std::shared_ptr<IGraphSymmetry> graphSymmetry, std::shared_ptr<IGraphShape> shape ) 
   : _GraphSymmetry( graphSymmetry )
   , _GraphShape( shape )
{   
}

void DualGraph::addVertex( int color, const XYZ& pos )
{
   _Vertices.push_back( Vertex( (int) _Vertices.size(), color, _GraphShape->toSurfaceFrom3D( pos ) ) );
   _Vertices.back().symmetry = _GraphSymmetry->calcSectorSymmetry( pos );
}

void DualGraph::swapVertexIndexes( int a, int b )
{
   std::swap( _Vertices[a], _Vertices[b] );
   std::swap( _Vertices[a].index, _Vertices[b].index );
   SwapNum swapper( a, b );
   for ( Vertex& vtx : _Vertices )
      for ( VertexPtr& neighb : _Vertices[vtx.index].neighbors )
         neighb._Index = swapper[neighb._Index];
}

void DualGraph::deleteVertex( const VertexPtr& vtx )
{
   if ( !vtx.isValid() )
      return;
   int k = (int)_Vertices.size()-1;
   swapVertexIndexes( vtx.index(), k );

   for ( VertexPtr& neighb : _Vertices[k].neighbors )
      _Vertices[neighb.index()].eraseEdgesTo( k );

   _Vertices.pop_back();
}

std::vector<DualGraph::VertexPtr> DualGraph::rawVertices() const
{
   std::vector<VertexPtr> ret;
   for ( const Vertex& vtx : _Vertices )
      ret.push_back( vtx.toVertexPtr( this ) );
   return ret;
}

std::vector<DualGraph::VertexPtr> DualGraph::allVisibleVertices() const
{
   std::vector<VertexPtr> ret;

   for ( const Vertex& vtx : _Vertices )
      for ( const SectorId& sector : vtx.symmetry->uniqueSectors() )
         ret.push_back( vtx.toVertexPtr( this ).withSectorId( sector ) );

   return ret;
}

DualGraph::VertexPtr DualGraph::vertexAt( const XYZ& pos, double maxDist ) const
{
   VertexPtr ret;
   double bestDist2 = maxDist * maxDist;


   for ( const VertexPtr& a : allVisibleVertices() )
   {
      double dist2 = a.pos().dist2( pos );
      if ( dist2 < bestDist2 )
      {
         bestDist2 = dist2;
         ret = a;
      }
   }

   return ret;
}

void DualGraph::setVertexColor( const VertexPtr& vtx, int color )
{
   _Vertices[vtx._Index].color = vtx._SectorId.unmapColor( color );;
}

void DualGraph::setVertexPos( const VertexPtr& vtx, const XYZ& pos )
{
   _Vertices[vtx._Index].pos = vtx._SectorId.inverted().matrix() * pos;
}

void DualGraph::toggleEdge( int idA, int idB )
{
   toggleEdge( vertexWithId( idA ), vertexWithId( idB ) );
}

void DualGraph::toggleEdge( const VertexPtr& a, const VertexPtr& b )
{
   if ( !a.isValid() || !b.isValid() || a == b )
      return;

   VertexPtr a0 = a.unpremul( b._SectorId );
   VertexPtr b0 = b.unpremul( a._SectorId );

   bool hadEdge = _Vertices[a._Index].hasNeighbor( b0 );
   if ( hadEdge )
   {
      _Vertices[a._Index].removeNeighbor( b0 );
      if ( a0 != b0 )
         _Vertices[b._Index].removeNeighbor( a0 );
   }
   else
   {
      _Vertices[a._Index].addNeighbor( b0 );
      if ( a0 != b0 )
         _Vertices[b._Index].addNeighbor( a0 );
   }

   //for ( auto a : (*this)[0].neighbors() )
   //   std::trace << a.name() << " ";
   //std::trace << std::endl;
}

void DualGraph::sortNeighbors()
{
   for ( Vertex& vtx : _Vertices )
   {
      XYZ n = _GraphShape->normalAt( vtx.pos );
      Matrix4x4 m = matrixRotateToZAxis( n ) * Matrix4x4::translation( -vtx.pos );
      auto angleOf = [&]( const XYZ& p ) { XYZ q = m*p; return ::atan2( q.y, q.x ); };
      sort( vtx.neighbors.begin(), vtx.neighbors.end(), [&]( const VertexPtr& a, const VertexPtr& b ) { return angleOf( a.pos() ) < angleOf( b.pos() ); } );
   }
}

void DualGraph::normalizeVertices()
{
   for ( Vertex& a : _Vertices )
      a.pos = _GraphShape->toSurfaceFrom3D( a.pos );
}

Json DualGraph::toJson() const
{
   JsonArray vertices;
   for ( const auto& a : _Vertices )
      vertices.push_back( a.toJson() );

   return JsonObj { { "symmetry", _GraphSymmetry->toJson() }, { "shape", _GraphShape->toJson() }, { "vertices", vertices } };
}

Json DualGraph::Vertex::toJson() const
{
   JsonArray neighborsJson;
   for ( const auto& a : neighbors )
      neighborsJson.push_back( a.toJson() );

   return JsonObj { {"index", index}, {"color", color}, {"pos", pos.toJson()}, {"neighbors", neighborsJson} };
}

DualGraph::Vertex::Vertex( const Json& json, const DualGraph* graph )
{
   index = json["index"].toInt();
   color = json["color"].toInt();
   pos = XYZ( json["pos"] );

   for ( const Json& j : json["neighbors"].toArray() )
      neighbors.push_back( VertexPtr( j, graph ) );
}

Json DualGraph::VertexPtr::toJson() const
{
   return JsonObj { {"index", index()}, {"sectorId", _SectorId.id()} };
}

DualGraph::VertexPtr::VertexPtr( const Json& json, const DualGraph* graph )
{
   _Graph = graph;
   _Index = json["index"].toInt();
   _SectorId = SectorId( json["sectorId"].toInt(), graph->_GraphSymmetry.get() );
}



DualGraph::DualGraph( const Json& json )
{
   if ( json.hasMember("edges") ) { initFromIcoJson( json ); return; }

   _GraphShape = IGraphShape::fromJson( json["shape"] );
   _GraphSymmetry = IGraphSymmetry::fromJson( json["symmetry"] );

   for ( const Json& j : json["vertices"].toArray() )
   {
      _Vertices.push_back( Vertex( j, this ) );
      _Vertices.back().symmetry = _GraphSymmetry->calcSectorSymmetry( _Vertices.back().pos );
   }
}

void DualGraph::initFromIcoJson( const Json& json )
{
   double radius = 0;
   {
      const Json& j = json["vertices"].toArray()[0];
      radius = XYZ( j["x"].toDouble(), j["y"].toDouble(), j["z"].toDouble() ).len();
   }

   _GraphShape.reset( new GraphShapeSphere( radius ) );

   SymmetryGroup symA( Icosahedron().map( {0,1,2}, { 5,4,8} ), Perm( { 5,4,2,3,1,0,6,7,8,9 } ) );
   SymmetryGroup symB( Icosahedron().map( {0,1,2}, {11,7,3} ), Perm( { 5,1,3,2,4,0,6,7,8,9 } ) );
   SymmetryGroup symC( Icosahedron().map( {0,1,2}, { 1,2,0} ), Perm( { 1,2,0,5,3,4,6,7,8,9 } ) );
   SymmetryGroup symD( Icosahedron().map( {0,1,2}, { 0,2,3} ), Perm( { 0,2,3,4,5,1,6,7,8,9 } ) );
   _GraphSymmetry.reset( new GraphSymmetry_Groups( { symA, symB, symC, symD } ) );

   for ( const Json& j : json["vertices"].toArray() )
   {
      int color = j["color"].toInt();
      XYZ pos( j["x"].toDouble(), j["y"].toDouble(), j["z"].toDouble() );
      addVertex( color, pos );      
   }

   for ( const Json& j : json["edges"].toArray() )
   {
      int a = j[0].toInt();
      int b = j[1].toInt();
      int rawSectorId = j[2].toInt();
      int sectorId = 0;
      if ( rawSectorId == 0 )
         sectorId = 0;
      else if ( rawSectorId == 10 )
         sectorId = 6;
      else if ( rawSectorId == 20 )
         sectorId = 5;
      else if ( rawSectorId == 30 )
         sectorId = 4;
      else
         continue;

      toggleEdge( VertexPtr( this, a, SectorId::identity( _GraphSymmetry.get() ) ), VertexPtr( this, b, SectorId( sectorId, _GraphSymmetry.get() ) ) );
   }
}