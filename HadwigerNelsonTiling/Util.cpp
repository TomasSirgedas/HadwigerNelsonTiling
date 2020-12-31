#include "Util.h"

#include <vector>

QPointF toPointF( const XYZ& pos ) { return QPointF( pos.x, pos.y ); }

const std::vector<uint32_t> COLORS = { 0xFFED1D26, 0xFF3F47CB, 0xFFFFF200, 0xFFA349A4, 0xFF22B14C, 0xFFFF7F27, 0xFF00EEEE, 0xFFFFFFFF, 0xFF000000, 0xFF808080 };
QColor tileColor( int idx )
{
   return QColor::fromRgb( COLORS[idx] );
}