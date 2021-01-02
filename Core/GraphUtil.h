#pragma once

#include "CoreMacros.h"

#include <memory>

class TileGraph;
class DualGraph;

CORE_API std::shared_ptr<TileGraph> makeTileGraph( DualGraph& dual, double radius );