#include "Simulation.h"
#include "trace.h"

#include <set>
#include <map>

using namespace std;

namespace
{
   int sign( double x ) { return x < 0 ? -1 : x > 0 ? 1 : 0; }
}

void Simulation::init( std::shared_ptr<TileGraph> tileGraph )
{
   _TileGraph = tileGraph;

   if ( _TileGraph )
   {
      _KeepCloseFars.clear();
      for ( auto kcf : _TileGraph->calcKeepCloseFars() )
         _KeepCloseFars.push_back( { kcf } );

      //_LineVertexConstraints = _Graph->calcLineVertexConstraints();

      _TileGraph->normalizeVertices();
   }

   //for ( const Graph::KeepCloseFar& kcf : _KeepCloseFars )
   //   if ( kcf.keepClose )
   //      qDebug() << graph->idOf( kcf.a ) << "-" << graph->idOf( kcf.b );

}

void Simulation::setRadius( double radius )
{
   _Radius = radius;
   if ( _DualGraph )
   {
      if ( _DualGraph->_GraphShape )
         _DualGraph->_GraphShape->setRadius( radius );
      _DualGraph->normalizeVertices();
      if ( _TileGraph )
         _TileGraph->normalizeVertices();
   }
}

double Simulation::step( int64_t stepIndex, double& paddingError )
{   
   if ( !_TileGraph )
      return -1;
   static bool s_printErrors = false;
   bool printErrors = s_printErrors && _PaddingError == 0;
   double totalError = 0;
   paddingError = 0;
   vector<XYZ> vel( _TileGraph->_Vertices.size() );

   for ( KeepCloseFarConstraint& kcfc : _KeepCloseFars )
   {
      if ( stepIndex % kcfc.checkFrequency != 0 )
         continue;

      const TileGraph::KeepCloseFar& kcf = kcfc.keepCloseFar;
      XYZ a = kcf.a.pos();
      XYZ b = kcf.b.pos();
      double dist = a.dist( b );

      bool wasViolation = false;
      double pad = kcf.keepClose && kcf.keepFar ? 0 : _Padding;
      if ( kcf.keepClose && dist >= 1.-pad )
      {
         vel[kcf.a.index()] += (kcf.a.matrix().inverted() * (b-a)) * (dist-(1-pad)) * .03;
         vel[kcf.b.index()] += (kcf.b.matrix().inverted() * (a-b)) * (dist-(1-pad)) * .03;
         totalError += max(0.,dist-1);
         paddingError += dist-(1-pad);
         if ( printErrors && !kcf.keepFar && dist-1 > 0 ) std::trace << "keep close " << kcf.a.id() << " " << kcf.b.id() << " " << dist-1 << std::endl;
         wasViolation = true;
      }
      if ( kcf.keepFar && dist <= _TileDist+pad )
      {
         vel[kcf.a.index()] += (kcf.a.matrix().inverted() * (a-b).normalized()) * ((_TileDist+pad)-dist) * .03;
         vel[kcf.b.index()] += (kcf.b.matrix().inverted() * (b-a).normalized()) * ((_TileDist+pad)-dist) * .03;
         totalError += max(0.,_TileDist-dist);
         paddingError += (_TileDist+pad)-dist;
         if ( printErrors && !kcf.keepClose && _TileDist-dist > 0 ) std::trace << "keep far " << kcf.a.id() << " " << kcf.b.id() << " " << _TileDist-dist << std::endl;
         wasViolation = true;
      }
      if ( wasViolation )
      {
         kcfc.checkFrequency = 1;
         kcfc.numViolations++;
      }
   }
   ////static bool s_dolvc = true;
   //for ( const Graph::LineVertexConstraint& lvc : _LineVertexConstraints )  if ( !lvc.curveCenter.isValid() )
   //{
   //   double pad = _Padding;

   //   // if the curveCenter is valid, then the curve is a circle with radius=1, centered at lvc.curveCenter
   //   // otherwise,                        the curve is a circle with radius=_Radius, centered at the origin
   //   XYZ center = lvc.curveCenter.isValid() ? _Graph->posOf( lvc.curveCenter ) : XYZ();
   //   XYZ b = _Graph->posOf( lvc.b ) - center;
   //   XYZ a0 = _Graph->posOf( lvc.a0 ) - center;
   //   XYZ a1 = _Graph->posOf( lvc.a1 ) - center;
   //   double R = lvc.curveCenter.isValid() ? 1 : _Radius;
   //   double dot = a0 * a1;
   //   // b = x * a0 + y * a1 + z * (a0^a1)   // solve for x/y/z
   //   double x = (b*a0 * R*R - b*a1 * dot) / (R*R*R*R - dot*dot);
   //   double y = (b*a1 * R*R - b*a0 * dot) / (R*R*R*R - dot*dot);
   //   if ( x < 0 || x > 1 || y < 0 || y > 1 )
   //      continue; // b doesn't lie between a0, a1  (so the distance checks on the endpoints are sufficient)
   //   XYZ q = (a0*x + a1*y).normalized() * R; // project the point to the circle 
   //   double dist = q.dist( b );
   //   if ( dist >= 1+pad ) 
   //      continue;

   //   XYZ qb = (q-b).normalized();
   //   vel[lvc.a0._Index] += (lvc.a0._Mtx.inverted() * qb) * ((1+pad)-dist) *  .005;
   //   vel[lvc.a1._Index] += (lvc.a1._Mtx.inverted() * qb) * ((1+pad)-dist) *  .005;
   //   vel[lvc.b._Index]  += (lvc.b._Mtx.inverted()  * qb) * ((1+pad)-dist) * -.01;
   //   totalError += max(0.,1-dist);
   //   paddingError += (1+pad)-dist;
   //   if ( printErrors && 1-dist > 0 ) qDebug() << "straight line to vertex" << lvc.a0._Index << lvc.a1._Index << lvc.b._Index << 1-dist;
   //}
   //for ( const Graph::LineVertexConstraint& lvc : _LineVertexConstraints )  if ( lvc.curveCenter.isValid() )
   //{
   //   double pad = _Padding;

   //   // if the curveCenter is valid, then the curve is a circle with radius=1, centered at lvc.curveCenter
   //   // otherwise,                        the curve is a circle with radius=_Radius, centered at the origin
   //   XYZ center = lvc.curveCenter.isValid() ? _Graph->posOf( lvc.curveCenter ) : XYZ();
   //   XYZ b = _Graph->posOf( lvc.b ) - center;
   //   if ( b.len2() >= (2+pad)*(2+pad) )
   //      continue;
   //   XYZ a0 = (_Graph->posOf( lvc.a0 ) - center).normalized();
   //   XYZ a1 = (_Graph->posOf( lvc.a1 ) - center).normalized();
   //   double R = lvc.curveCenter.isValid() ? 1 : _Radius;
   //   double dot = a0 * a1;
   //   // b = x * a0 + y * a1 + z * (a0^a1)   // solve for x/y/z
   //   double x = (b*a0 * 1 - b*a1 * dot) / (1 - dot*dot);
   //   double y = (b*a1 * 1 - b*a0 * dot) / (1 - dot*dot);
   //   if ( x < 0 || y < 0 )
   //      continue; // b doesn't lie between a0, a1  (so the distance checks on the endpoints are sufficient)
   //   XYZ q = (a0*x + a1*y).normalized() * R; // project the point to the circle 
   //   double dist = q.dist( b );
   //   if ( dist >= 1+pad ) 
   //      continue;
   //   if ( isnan(dist) )
   //      qDebug() << lvc.curveCenter._Index << lvc.a0._Index << lvc.a1._Index << lvc.b._Index << dist;

   //   XYZ qb = (q-b).normalized();
   //   vel[lvc.curveCenter._Index]  += (lvc.curveCenter._Mtx.inverted() * qb) * ((1+pad)-dist) *  .00003;
   //   vel[lvc.a0._Index]           += (lvc.a0._Mtx.inverted()          * qb) * ((1+pad)-dist) *  .00003;
   //   vel[lvc.a1._Index]           += (lvc.a1._Mtx.inverted()          * qb) * ((1+pad)-dist) *  .00003;
   //   vel[lvc.b._Index]            += (lvc.b._Mtx.inverted()           * qb) * ((1+pad)-dist) * -.00009;
   //   totalError += max(0.,1-dist);
   //   paddingError += (1+pad)-dist;
   //   if ( printErrors && 1-dist > 0 ) qDebug() << "curved line to vertex" << lvc.a0._Index << lvc.a1._Index << lvc.curveCenter._Index << lvc.b._Index << 1-dist;
   //}
      
   // outer perimeter
   for ( const TileGraph::Vertex& vtx : _TileGraph->_Vertices ) if ( vtx._OnPerimeter )
   {
      if ( vtx._Pos.len2() >= _OuterRadius*_OuterRadius )
         continue;

      double d = vtx._Pos.len();
      double distError = _OuterRadius - d;
      vel[vtx._Index] += (vtx._Pos/d) * distError * .03;
      totalError += distError;
   }

   // inner perimeter
   if ( _InnerRadius > 0 )
   for ( const TileGraph::Vertex& vtx : _TileGraph->_Vertices ) if ( vtx._OnInnerPerimeter )
   {
      if ( vtx._Pos.len2() <= _InnerRadius*_InnerRadius )
         continue;

      double d = vtx._Pos.len();
      double distError = d - _InnerRadius;
      vel[vtx._Index] += -(vtx._Pos/d) * distError * .03;
      totalError += distError;
   }


   // strip
   if ( _StripWidth > 0 && _StripHeight > 0 )
   for ( const TileGraph::Vertex& vtx : _TileGraph->_Vertices ) if ( vtx._OnPerimeter )
   {
      double errX = _StripWidth/2 - abs(vtx._Pos.x);
      double errY = _StripHeight/2 - abs(vtx._Pos.y);
      if ( errX < 0 || errY < 0 )
         continue;

      double distError = max( errX, errY );
      XYZ dir = errX < errY ? XYZ(1,0,0) * sign( vtx._Pos.x )
                            : XYZ(0,1,0) * sign( vtx._Pos.y );

      vel[vtx._Index] += dir * distError * .03;
      totalError += distError;
   }

   // apply velocities
   for ( int i = 0; i < (int)vel.size(); i++ ) if ( !_TileGraph->_Vertices[i]._Symmetry->hasSymmetry() ) if ( i != _FixedVertex.index() )
   {
      _TileGraph->_Vertices[i]._Pos += vel[i];
   }
   _TileGraph->normalizeVertices();

   return totalError;
}

double Simulation::step( int numSteps )
{
   for ( KeepCloseFarConstraint& kcfc : _KeepCloseFars )
      kcfc.numViolations = 0;

   double tot = 0;
   double totalPaddingError = 0;
   for ( int i = 0; i < numSteps; i++ )
   {
      double paddingError = 0;
      tot += step( i, paddingError );
      totalPaddingError += paddingError;
   }

   for ( KeepCloseFarConstraint& kcfc : _KeepCloseFars )
      if ( kcfc.numViolations == 0 )
         kcfc.checkFrequency = numSteps / 2;

   _PaddingError = totalPaddingError / numSteps;
   return tot / numSteps;
}

void Simulation::moveDualVerticesToCentroid()
{
   if ( !_TileGraph )
      return;

   for ( const TileGraph::Tile& tile : _TileGraph->_Tiles )
   {
      std::vector<XYZ> v;
      for ( const TileGraph::VertexPtr& vtx : tile._Vertices )
         v.push_back( vtx.pos() );
      _DualGraph->_Vertices[tile._Index].pos = centroid( v );
   }
}