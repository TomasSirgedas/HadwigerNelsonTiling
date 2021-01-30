#include "Drawing.h"

#include <QPainter>

#include <Core/DualGraph.h>
#include <Core/TileGraph.h>
#include <Core/Simulation.h>
#include <Core/DualAnalysis.h>

#include <unordered_set>


namespace
{
   void drawTextCentered( QPainter& painter, const QPointF& p, const std::string& str )
   {
      painter.drawText( QRectF( p + QPointF( -1000, -1000 ), QSizeF( 2000, 2000 ) ), QString::fromStdString( str ), QTextOption( Qt::AlignCenter ) );
   }
   uint64_t edgeId( const TileGraph::VertexPtr& a, const TileGraph::VertexPtr& b )
   {
      return ( (uint64_t) std::max( a.id(), b.id() ) << 32 ) + std::min( a.id(), b.id() );
   }
   std::vector<XYZ> calcCurvePlanar( const XYZ& p0_, const XYZ& p1_, const XYZ& center, double maxDistance, bool addP0 )
   {
      XYZ p0 = p0_ - center;
      XYZ p1 = p1_ - center;
      if ( p0.dist2(p1) < 1e-14 )
         return {};
      XYZ axis = XYZ(0,0,-1); // axis

      XYZ v = (axis^p0).normalized();
      XYZ u = v^axis;
      double angle = atan2( p1*v, p1*u );
      int dir = angle > 0 ? 1 : -1;
      angle *= dir;
      if ( angle < 0 )
         angle += PI*2;

      double radius0 = p0 * u;
      double radius1 = sqrt((p1*u)*(p1*u) + (p1*v)*(p1*v));
      double dist = radius0 * angle;
      int numSegments = (int) ceil( dist / maxDistance );

      std::vector<XYZ> ret;
      if ( addP0 )
         ret.push_back( p0 );
      for ( int i = 1; i <= numSegments; i++ )
      {
         double t = (double)i / numSegments;
         double radius = radius0 * (1-t) + radius1 * t;
         double zDist = (p0*axis)*(1-t) + (p1*axis)*t;
         ret.push_back( center + axis*zDist + u*radius*cos(dir*angle*t) + v*radius*sin(dir*angle*t) );
      }
      return ret;
   }
   // dir == 1, dir == -1 --> curve direction
   // dir == 0 --> pick shorter curve direction
   std::vector<XYZ> calcCurve( const XYZ& p0, const XYZ& p1, const XYZ& center, double maxDistance, int dir, bool addP0 )
   {
      if ( p0.dist2(p1) < 1e-14 )
         return {};
      XYZ a = center.len2() < 1e-14 ? (p0^p1).normalized() : center.normalized();
      if ( (a^p0).len2() < 1e-14 )
         return {}; // p is on the axis

      XYZ v = (a^p0).normalized();
      XYZ u = v^a;
      double angle = atan2( p1*v, p1*u );
      if ( dir == 0 ) dir = angle > 0 ? 1 : -1;
      angle *= dir;
      if ( angle < 0 )
         angle += PI*2;

      double radius0 = p0 * u;
      double radius1 = sqrt((p1*u)*(p1*u) + (p1*v)*(p1*v));
      double dist = radius0 * angle;
      int numSegments = (int) ceil( dist / maxDistance );

      std::vector<XYZ> ret;
      if ( addP0 )
         ret.push_back( p0 );
      for ( int i = 1; i <= numSegments; i++ )
      {
         double t = (double)i / numSegments;
         double radius = radius0 * (1-t) + radius1 * t;
         double zDist = (p0*a)*(1-t) + (p1*a)*t;
         ret.push_back( a*zDist + u*radius*cos(dir*angle*t) + v*radius*sin(dir*angle*t) );
      }
      return ret;
   }

   std::vector<XYZ> calcTileOutline( std::shared_ptr<IGraphShape> shape, const TileGraph::TilePtr& tile, double maxSpacing, bool perimeterPopout = false )
   {
      std::vector<XYZ> ret;

      for ( const auto& edge : tile.edges() )
      {
         const TileGraph::VertexPtr& a = edge.first;
         const TileGraph::VertexPtr& b = edge.second;
         if ( maxSpacing >= 1 ) { ret.push_back( b.pos() ); continue; }
         bool isPerimeterEdge = a.isOnPerimeter() && b.isOnPerimeter();
         if ( perimeterPopout && isPerimeterEdge )
         {
            ret.push_back( a.pos() * 2.5 );
            ret.push_back( b.pos() * 2.5 );
            ret.push_back( b.pos() );
            continue;
         }

         TileGraph::VertexPtr c = a.calcCurve( b ); // c = center of curve
         std::vector<XYZ> curve;
         if ( c.isValid() )
         {
            if ( shape->isCurved() )
               curve = calcCurve( a.pos(), b.pos(), c.pos(), maxSpacing, 0, false );
            else
               curve = calcCurvePlanar( a.pos(), b.pos(), c.pos(), maxSpacing, false );
         }
         else
         {
            if ( shape->isCurved() )
               curve = calcCurve( a.pos(), b.pos(), XYZ(), maxSpacing, 1, false );
            else
               curve = { b.pos() };
         }
         ret.insert( ret.end(), curve.begin(), curve.end() );
      }

      return ret;
   }
   void drawMessage( QPainter& painter, const QPoint& pos, const std::string& message_ )
   {
      QFontMetrics fm( painter.font() );
      QString message = QString::fromStdString( message_ );
      int width = fm.horizontalAdvance( message );
      int height = fm.height();
      QRect rect = QRect( pos.x(), pos.y(), width, height );

      painter.setBrush( QColor( 0,0,0,64 ) );
      painter.setPen( Qt::NoPen );
      painter.drawRect( rect.adjusted( -1, -1, 1, 1 ) );
      painter.setPen( QColor( 255,255,255 ) );
      painter.drawText( QRectF( rect ), message, QTextOption( ) );
   }
}


Drawing::Drawing( QWidget *parent )
   : QWidget( parent )
{
   ui.setupUi( this );

   _ModelRotation = Matrix4x4::rotationY( 0. ) * Matrix4x4::rotationX( -.0 );
}

Drawing::~Drawing()
{
}

void Drawing::refresh()
{
   if ( _GraphShape && _GraphShape->modelSize() > 0 )
      _PixelsPerUnit = height() / 2.* .99 / _GraphShape->modelSize();
   else
      _PixelsPerUnit = 100;

   _PixelsPerUnit *= _Zoom;

   _ModelToBitmap = Matrix4x4::translation( XYZ( width()/2, height()/2, 0. ) )
                  * Matrix4x4::scale( XYZ( _PixelsPerUnit, _PixelsPerUnit, 1 ) )
                  * Matrix4x4::scale( XYZ( 1, -1, 1 ) );
}

void Drawing::resizeEvent( QResizeEvent *event )
{
   refresh();
   emit resized();
}

bool Drawing::isVisible( const XYZ& pos ) const
{
   return _GraphShape->isVisible( pos, _ModelRotation );
}

QImage Drawing::makeTransparentImage( const QSize& size, std::shared_ptr<const Simulation> simulation, std::shared_ptr<const DualAnalysis> dualAnalysis )
{   
   const DualGraph& dual = *simulation->_DualGraph;
   const TileGraph& graph = *simulation->_TileGraph;
   QImage image( size, QImage::Format_ARGB32_Premultiplied );

   image.fill( Qt::darkGray );
   QPainter painter( &image );
   painter.setRenderHint( QPainter::Antialiasing, true );


   if ( _ShowDualGraph )
   {
      // draw edges
      painter.setPen( QColor( 0, 0, 0, 192 ) );
      for ( const DualGraph::VertexPtr& a : dual.allVisibleVertices() )
      for ( const DualGraph::VertexPtr& b : a.neighbors() ) if ( isVisible( a.pos() ) || isVisible( b.pos() ) )
      {  
         if ( b.isVisible() && a < b )
            painter.drawLine( toBitmap( a.pos() ), toBitmap( b.pos() ) );
         if ( !b.isVisible() )
            painter.drawLine( toBitmap( a.pos() ), toBitmap( (a.pos()*.8 + b.pos()*.2) ) );


         if ( dualAnalysis && dualAnalysis->isCurved( a, b ) )
         {
            QPointF p0 = toBitmap( a.pos() );
            QPointF p1 = toBitmap( b.pos() );
            if ( dualAnalysis->isCurvedTowardsA( a, b ) )
               std::swap( p0, p1 );
            QPointF v = p1-p0;
            QPointF u = QPointF( -v.y(), v.x() );
            // draw arrow
            painter.drawLine( p0 + v*.4 + u*.05, p0 + v*.5 );
            painter.drawLine( p0 + v*.4 - u*.05, p0 + v*.5 );
            painter.drawLine( p0 + v*.3        , p0 + v*.5 );
         }
      }

      // draw vertices
      painter.setPen( Qt::black );
      for ( const DualGraph::VertexPtr& a : dual.allVisibleVertices() ) if ( isVisible( a.pos() ) )
      {
         painter.setBrush( tileColor( a.color() ) );
         painter.drawEllipse( toBitmap( a.pos() ), 4, 4 );
      }

      // draw error vertices
      if ( dualAnalysis )
      {
         painter.setPen( QPen( QColor( 255, 0, 0, 128 ), 3. ) );
         painter.setBrush( Qt::NoBrush );
         for ( const DualGraph::VertexPtr& a : dualAnalysis->errorVertices() ) if ( a.isValid() )
         {
            painter.drawEllipse( toBitmap( a.pos() ), 6, 6 );
         }
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

   if ( _ShowTileGraph && &graph != nullptr )
   {
      // draw tiles
      painter.setPen( Qt::NoPen );
      //painter.setPen( QColor( 0, 0, 0, 32 ) );
      for ( const TileGraph::TilePtr& tile : graph.allTiles() ) // if ( isVisible( a.pos() ) )
      {
         std::vector<XYZ> outline = calcTileOutline( _GraphShape, tile, 10/_PixelsPerUnit/*max curve spacing*/, _DiskMode && simulation->_PerimeterRadius > 0 );
         QPolygonF poly;
         for ( const XYZ& a : outline )
            poly.append( toBitmap( a ) );

         //for ( const TileGraph::VertexPtr& a : tile.vertices() )
         //   poly.append( toBitmap( a.pos() ) );

         if ( signedArea( poly ) < 0 )
            continue;

         painter.setBrush( withAlpha( tileColor( tile.color() ), .2 ) );
         painter.drawPolygon( poly );
      }


      if ( _ShowRigids )
      {
         painter.setPen( QPen( QColor(0,0,0,96), 2.5 ) );
         painter.setBrush( Qt::NoBrush );
         std::unordered_set<uint64_t> usedEdges;
         for ( const SectorId& sectorId : simulation->_TileGraph->_GraphSymmetry->allVisibleSectors() )
         for ( const auto& pr : simulation->_KeepCloseFars )
         {  
            TileGraph::VertexPtr aa = pr.a.premul( sectorId );
            TileGraph::VertexPtr bb = pr.b.premul( sectorId );
            if ( !usedEdges.insert( edgeId( aa, bb ) ).second )
               continue;
            XYZ a = sectorId.matrix() * pr.a.pos();
            XYZ b = sectorId.matrix() * pr.b.pos();

            {
               if ( pr.keepClose && pr.keepFar && isVisible( a ) && isVisible( b )  )
                  painter.drawLine( toBitmap( a ), toBitmap( b ) );
            }
         }
      }    

      // draw labels
      if ( _ShowLabels )
      {
         painter.setPen( Qt::black );
         for ( const TileGraph::VertexPtr& a : graph.allVertices() ) if ( isVisible( a.pos() ) )
         {
            drawTextCentered( painter, toBitmap( a.pos() ) + QPointF( 0, -7 ), a.name() );
         }


         //// tile label(?) -- same as dual graph vertex label
         //painter.setPen( Qt::black );
         //for ( const TileGraph::TilePtr& tile : graph.allTiles() ) if ( isVisible( tile.avgPos() ) )
         //{
         //   XYZ p = tile.avgPos();
         //   drawTextCentered( painter, toBitmap( tile.avgPos() ), "[" + tile.name() + "]" );
         //}
      }

   }

   if ( _ShowTileGraph && simulation->_TileGraph )
   {
      TileGraph::VertexPtr a = simulation->_TileGraph->vertexWithId( simulation->_ShowDistanceVertices.first );
      TileGraph::VertexPtr b = simulation->_TileGraph->vertexWithId( simulation->_ShowDistanceVertices.second );
      if ( a.isValid() && b.isValid() )
      {
         XYZ posA = a.pos();
         XYZ posB = b.pos();
         double dist = posA.dist( posB );
         QPen distPen( QColor(255,255,255,128), 3. );
         distPen.setStyle( Qt::DotLine );
         painter.setPen( distPen );
         painter.drawLine( toBitmap( posA ), toBitmap( posB ) );
         painter.setPen( Qt::black );
         drawMessage( painter, QPoint( 4, 4 ), "distance = " + std::to_string( dist ) );
      }
   }

   if ( dualAnalysis )
   {
      painter.setPen( QPen( Qt::black ) );
      drawMessage( painter, QPoint( 4, size.height()-12-4 ), dualAnalysis->errorMessage() );
   }

   if ( simulation->_PerimeterRadius > 0 && !_DiskMode )
   {
      painter.setPen( QColor( 0, 0, 0, 64 ) );
      painter.setBrush( Qt::NoBrush );
      QPointF center = toBitmap( XYZ( 0, 0, 0 ) );
      double rx = QLineF( center, toBitmap( XYZ( simulation->_PerimeterRadius, 0, 0 ) ) ).length();
      double ry = QLineF( center, toBitmap( XYZ( 0, simulation->_PerimeterRadius, 0 ) ) ).length();
      painter.drawEllipse( center, rx, ry );

      painter.setFont( QFont( "Arial", 24 ) );
      painter.setPen( QColor( 0, 0, 0, 255 ) );
      painter.drawText( toBitmap( XYZ( -simulation->_PerimeterRadius, -simulation->_PerimeterRadius, 0 ) ), QString( "r = %1" ).arg( simulation->_PerimeterRadius ) );
   }

   return image;
}

QImage Drawing::makeImage( const QSize& size, std::shared_ptr<const Simulation> simulation, std::shared_ptr<const DualAnalysis> dualAnalysis )
{
   QImage image = makeTransparentImage( size, simulation, dualAnalysis );

   QImage finalImage( size, QImage::Format_RGB32 );   

   // crop to disk
   if ( simulation->_PerimeterRadius > 0 && _DiskMode )
   {
      QPointF center = toBitmap( XYZ( 0, 0, 0 ) );
      double r = QLineF( center, toBitmap( XYZ( simulation->_PerimeterRadius, 0, 0 ) ) ).length();

      int sx = image.width();
      int sy = image.height();
      uint8_t* p = (uint8_t*) image.bits();
      for ( int y = 0; y < sy; y++ )
      for ( int x = 0; x < sx; x++, p += 4 )
      {
         double d2 = (x+.5 - center.x())*(x+.5 - center.x()) + (y+.5 - center.y())*(y+.5 - center.y());
         double d = sqrt( d2 );
         double alpha = r - d + .5;
         alpha = alpha < 0 ? 0 : alpha > 1 ? 1 : alpha; // clamp to [0,1]
         if ( alpha == 0 ) { p[0] = 0; p[1] = 0; p[2] = 0; p[3] = 0; }
         else if ( alpha < 1 )
         {
            p[0] = (uint8_t) lround( p[0]*alpha );
            p[1] = (uint8_t) lround( p[1]*alpha );
            p[2] = (uint8_t) lround( p[2]*alpha );
            p[3] = (uint8_t) lround( p[3]*alpha );
         }
      }
   }

   QPainter painter( &finalImage );
   painter.fillRect( finalImage.rect(), _DiskMode ? Qt::white : Qt::darkGray );
   painter.drawImage( QPoint( 0, 0 ), image );

   //if ( simulation->_PerimeterRadius > 0 && _DiskMode )
   //{
   //   painter.setFont( QFont( "Arial", 24 ) );
   //   painter.setPen( QColor( 0, 0, 0, 255 ) );
   //   painter.drawText( toBitmap( XYZ( -simulation->_PerimeterRadius, -simulation->_PerimeterRadius, 0 ) ), QString( "r = %1" ).arg( simulation->_PerimeterRadius ) );
   //}

   return finalImage;
}

void Drawing::updateDrawing( std::shared_ptr<const Simulation> simulation, std::shared_ptr<const DualAnalysis> dualAnalysis )
{  
   ui.label->setPixmap( QPixmap::fromImage( makeImage( size(), simulation, dualAnalysis ) ) );
}

bool Drawing::getModelPos( const QPointF& bitmapPos, XYZ& modelPos ) const 
{ 
   XYZ surfacePos;
   if ( !_GraphShape->toSurfaceFrom2D( _ModelToBitmap.inverted() * XYZ( bitmapPos.x(), bitmapPos.y(), 0. ), surfacePos ) )
      return false; 
   modelPos = _ModelRotation.inverted() * surfacePos;
   return true;
}