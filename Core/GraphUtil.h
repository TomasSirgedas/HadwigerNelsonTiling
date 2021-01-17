#pragma once

#include "CoreMacros.h"
#include "DualGraph.h"
#include "TileGraph.h"

#include <memory>
#include <iostream>

class TileGraph;
class DualGraph;

CORE_API std::shared_ptr<TileGraph> makeTileGraph( DualGraph& dual, double radius );

CORE_API std::ostream& operator<<( std::ostream& os, const DualGraph::VertexPtr& a );
CORE_API std::ostream& operator<<( std::ostream& os, const TileGraph::VertexPtr& a );
CORE_API std::ostream& operator<<( std::ostream& os, const TileGraph::TilePtr& a );

CORE_API std::vector<XYZ> positionsOf( const std::vector<TileGraph::VertexPtr>& v );
CORE_API std::vector<XYZ> positionsOf( const std::vector<DualGraph::VertexPtr>& v );
CORE_API double signedArea( const std::vector<TileGraph::VertexPtr>& v );

CORE_API bool isValidPolygon( const std::shared_ptr<IGraphShape> shape, const std::vector<DualGraph::VertexPtr>& v );