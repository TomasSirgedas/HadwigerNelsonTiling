#include "Util.h"

#include <vector>

QPointF toPointF( const XYZ& pos ) { return QPointF( pos.x, pos.y ); }

const std::vector<uint32_t> COLORS = { 0xFFED1D26, 0xFF3F47CB, 0xFFFFF200, 0xFFA349A4, 0xFF22B14C, 0xFFFF7F27, 0xFF00EEEE, 0xFFFFFFFF, 0xFF000000, 0xFF909090 };
QColor tileColor( int idx )
{
   return QColor::fromRgb( COLORS[idx] );
}

QColor withAlpha( const QColor& color, double alpha ) { return QColor( color.red(), color.green(), color.blue(), lround( alpha*255 ) ); }

double signedArea( const QPolygonF& poly )
{
   if ( poly.size() <= 2 )
      return 0;

   double area2 = poly[0].x() * poly.back().y() - poly.back().x() * poly[0].y();
      
   for ( int i = 0; i+1 < poly.size(); i++ )
      area2 += poly[i+1].x() * poly[i].y() - poly[i].x() * poly[i+1].y();

   return area2 / 2;
}