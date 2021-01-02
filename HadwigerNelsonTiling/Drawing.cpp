#include "Drawing.h"

#include <QPainter>

#include <Core/DualGraph.h>
#include <Core/TileGraph.h>


namespace
{
   void drawTextCentered( QPainter& painter, const QPointF& p, const std::string& str )
   {
      painter.drawText( QRectF( p + QPointF( -1000, -1000 ), QSizeF( 2000, 2000 ) ), QString::fromStdString( str ), QTextOption( Qt::AlignCenter ) );
   }
}


Drawing::Drawing( QWidget *parent )
   : QWidget( parent )
{
   ui.setupUi( this );

   _ModelRotation = Matrix4x4::rotationY( 0. ) * Matrix4x4::rotationX( -.3 );
}

Drawing::~Drawing()
{
}

void Drawing::resizeEvent( QResizeEvent *event )
{
   _ModelToBitmap = Matrix4x4::translation( XYZ( width()/2, height()/2, 0. ) )
                  * Matrix4x4::scale( XYZ( _PixelsPerUnit, _PixelsPerUnit, 1 ) )
                  * Matrix4x4::scale( XYZ( 1, -1, 1 ) );

   if ( _GraphShape && _GraphShape->modelSize() > 0 )
      _PixelsPerUnit = height() / 2.* .99 / _GraphShape->modelSize();

   emit resized();
}

bool Drawing::isVisible( const XYZ& pos ) const
{
   return (_ModelRotation * pos).z <= 0;
}

QImage Drawing::makeImage( const QSize& size, const DualGraph& dual, const TileGraph& graph )
{
   QImage image( size, QImage::Format_RGB888 );

   QPainter painter( &image );
   painter.fillRect( image.rect(), Qt::darkGray );
   painter.setRenderHint( QPainter::Antialiasing, true );


   if ( _ShowDualGraph )
   {
      // draw edges
      painter.setPen( QColor( 0, 0, 0, 192 ) );
      for ( const DualGraph::VertexPtr& a : dual.allVisibleVertices() )
      for ( const DualGraph::VertexPtr& b : a.neighbors() ) if ( a < b ) if ( isVisible( a.pos() ) || isVisible( b.pos() ) )
      {
         painter.drawLine( toBitmap( a.pos() ), toBitmap( b.pos() ) );
         //painter.drawLine( toBitmap( a.pos() ), toBitmap( (a.pos() + b.pos()) / 2 ) );
      }

      // draw vertices
      painter.setPen( Qt::black );
      for ( const DualGraph::VertexPtr& a : dual.allVisibleVertices() ) if ( isVisible( a.pos() ) )
      {
         painter.setBrush( tileColor( a.color() ) );
         painter.drawEllipse( toBitmap( a.pos() ), 4, 4 );
      }

      // draw labels
      if ( _ShowLabels )
      {
         painter.setPen( Qt::black );
         for ( const DualGraph::VertexPtr& a : dual.allVisibleVertices() ) if ( isVisible( a.pos() ) )
         {
            drawTextCentered( painter, toBitmap( a.pos() ) + QPointF( 0, -11 ), a.name() );
         }
      }
   }

   if ( /*_ShowTileGraph &&*/ &graph != nullptr )
   {
      for ( const TileGraph::TilePtr& tile : graph.allTiles() ) // if ( isVisible( a.pos() ) )
      {
         QPolygonF poly;
         for ( const TileGraph::VertexPtr& a : tile.vertices() )
            poly.append( toBitmap( a.pos() ) );

         painter.setBrush( withAlpha( tileColor( tile.color() ), .5 ) );
         painter.drawPolygon( poly );
      }

   }

   return image;
}

void Drawing::updateDrawing( const DualGraph& dual, const TileGraph& tileGraph )
{  
   ui.label->setPixmap( QPixmap::fromImage( makeImage( size(), dual, tileGraph ) ) );
}

bool Drawing::getModelPos( const QPointF& bitmapPos, XYZ& modelPos ) const 
{ 
   XYZ surfacePos;
   if ( !_GraphShape->toSurfaceFrom2D( _ModelToBitmap.inverted() * XYZ( bitmapPos.x(), bitmapPos.y(), 0. ), surfacePos ) )
      return false; 
   modelPos = _ModelRotation.inverted() * surfacePos;
   return true;
}