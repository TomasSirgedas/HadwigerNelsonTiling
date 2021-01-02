#pragma once

#include "CoreMacros.h"
#include "DualGraph.h"

#include <memory>
#include <iostream>

class TileGraph;
class DualGraph;

CORE_API std::shared_ptr<TileGraph> makeTileGraph( DualGraph& dual, double radius );

CORE_API std::ostream& operator<<( std::ostream& os, const DualGraph::VertexPtr& a );