#pragma once

#include <QPointF>
#include <QColor>

#include <Core/DataTypes.h>


QPointF toPointF( const XYZ& pos );
QColor tileColor( int idx );