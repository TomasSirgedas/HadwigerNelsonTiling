#pragma once

#include <memory>

class TileGraph;
class DualGraph;

std::shared_ptr<TileGraph> makeTileGraph( DualGraph& dual );