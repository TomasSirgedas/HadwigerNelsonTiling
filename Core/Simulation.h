#pragma once

#include "TileGraph.h"
#include "DualGraph.h"

class Simulation
{
public:
   CORE_API void init( std::shared_ptr<TileGraph> graph );
   CORE_API void normalizeVertices();
   CORE_API double step( double& paddingError );
   CORE_API double step( int numSteps );
   CORE_API void setRadius( double radius );   
   CORE_API bool showDistanceVerticesExist() const { return _ShowDistanceVertices.first.exists() && _ShowDistanceVertices.second.exists();  }

public:
   double _Radius = 1;
   double _Padding = .0000;
   double _PaddingError = 0;
   TileGraph::VertexPtr _FixedVertex;
   std::shared_ptr<TileGraph> _TileGraph;
   std::shared_ptr<DualGraph> _DualGraph;
   std::vector<TileGraph::KeepCloseFar> _KeepCloseFars;
   std::vector<TileGraph::LineVertexConstraint> _LineVertexConstraints;
   std::pair<TileGraph::VertexPtr, TileGraph::VertexPtr> _ShowDistanceVertices;
};

