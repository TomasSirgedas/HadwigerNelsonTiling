#pragma once

#include <QWidget>
#include "ui_Drawing.h"

#include <Core/DataTypes.h>

#include "Util.h"

class DualGraph;

class Drawing : public QWidget
{
   Q_OBJECT

public:
   Drawing( QWidget *parent = Q_NULLPTR );
   ~Drawing();

   void updateDrawing( const DualGraph& dual );

   QPointF toBitmap( const XYZ& modelPos ) { return toPointF( ( _ModelToBitmap * modelPos ).toXYZ() ); }
   XYZ toModel( const QPointF& bitmapPos ) { return ( _ModelToBitmap.inverted() * XYZ( bitmapPos.x(), bitmapPos.y(), 0. ) ).toXYZ(); }
   double toModel( double bitmapSize ) const { return bitmapSize / _PixelsPerUnit; }

   bool isVisible( const XYZ& pos ) const;
   QImage makeImage( const QSize& size, const DualGraph& dual );


private:
   void resizeEvent( QResizeEvent *event ) override;

   void mousePressEvent( QMouseEvent * event ) override { emit press( event ); }
   void mouseReleaseEvent( QMouseEvent * event ) override { emit release( event ); }
   void mouseMoveEvent( QMouseEvent * event ) override { emit move( event ); }

signals:
   void resized();

   void press( QMouseEvent * event );
   void release( QMouseEvent * event );
   void move( QMouseEvent * event );

private:
   Ui::Drawing ui;

public:
   Matrix4x4 _ModelToBitmap;
   double _PixelsPerUnit = 100;
};
