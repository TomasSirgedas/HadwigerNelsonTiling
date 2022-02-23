#pragma once

#include "TileGraph.h"
#include "DualGraph.h"

class Simulation
{
public:
   struct KeepCloseFarConstraint
   {
      TileGraph::KeepCloseFar keepCloseFar;
      int checkFrequency = 1;
      int numViolations = 0;
   };

public:
   CORE_API void init( std::shared_ptr<TileGraph> graph );
   CORE_API void normalizeVertices();
   CORE_API double step( int64_t stepIndex, double& paddingError );
   CORE_API double step( int numSteps );
   CORE_API void setRadius( double radius );   
   CORE_API void moveDualVerticesToCentroid();

public:
   double _Radius = 1;
   double _Padding = .0000;
   double _PaddingError = 0;
   double _OuterRadius = 0;
   double _InnerRadius = 0;
   double _StripWidth = 0;
   double _StripHeight = 0;
   double _TileDist = 1.;
   TileGraph::VertexPtr _FixedVertex;
   std::shared_ptr<TileGraph> _TileGraph;
   std::shared_ptr<DualGraph> _DualGraph;
   std::vector<KeepCloseFarConstraint> _KeepCloseFars;
   std::vector<TileGraph::LineVertexConstraint> _LineVertexConstraints;
   std::pair<int, int> _ShowDistanceVertices = {-1,-1};
};

