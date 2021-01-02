#pragma once

#include "ui_Drawing.h"
#include "Util.h"

#include <QWidget>
#include <Core/DataTypes.h>
#include <Core/Util.h>
#include <memory>

class DualGraph;
class IGraphShape;

class Drawing : public QWidget
{
   Q_OBJECT

public:
   Drawing( QWidget *parent = Q_NULLPTR );
   ~Drawing();

   void updateDrawing( const DualGraph& dual );

   QPointF toBitmap( const XYZ& modelPos ) { return toPointF( _ModelToBitmap * _ModelRotation * modelPos ); }
   //XYZ toModelZ0( const QPointF& bitmapPos ) { return ( (_ModelToBitmap * _ModelRotation).inverted() * XYZ( bitmapPos.x(), bitmapPos.y(), 0. ) ).toXYZ(); }
   bool getModelPos( const QPointF& bitmapPos, XYZ& modelPos ) const;
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
   Matrix4x4 _ModelRotation;
   double _PixelsPerUnit = 100;
   std::shared_ptr<IGraphShape> _GraphShape;
};
