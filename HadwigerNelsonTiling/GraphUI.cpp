#include "GraphUI.h"
#include "PlatformSpecific.h"

#include <Core/DataTypes.h>
#include <QShortcut>
#include <QMouseEvent>
#include <QDebug>


GraphUI::GraphUI( QWidget *parent )
   : QWidget( parent )
{
   ui.setupUi( this );


   std::shared_ptr<IGraphShape> shape( new GraphShapeSphere( 1. ) );

   //SymmetryGroup g5( Matrix4x4::rotation( XYZ(0,0,1), 2*PI/5 ), Perm( { 1,2,3,4,0,5,6,7,8,9 } ) );
   //SymmetryGroup g3( Matrix4x4::rotation( XYZ(0,0,1), 2*PI/3 ) * Matrix4x4::translation( XYZ(3,0,0) ), Perm( { 1,2,0,3,4,5,6,7,8,9 } ) );   
   //std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( { g3, g5 } ) );

   SymmetryGroup symA( Icosahedron().map( {0,1,2}, { 5,4,8} ), Perm( { 5,4,2,3,1,0,6,7,8,9 } ) );
   SymmetryGroup symB( Icosahedron().map( {0,1,2}, {11,7,3} ), Perm( { 5,1,3,2,4,0,6,7,8,9 } ) );
   SymmetryGroup symC( Icosahedron().map( {0,1,2}, { 1,2,0} ), Perm( { 1,2,0,5,3,4,6,7,8,9 } ) );
   SymmetryGroup symD( Icosahedron().map( {0,1,2}, { 0,2,3} ), Perm( { 0,2,3,4,5,1,6,7,8,9 } ) );
   std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( { symA, symB, symC, symD } ) );


   //_DualGraph.reset( new DualGraph( std::shared_ptr<IGraphSymmetry>( new GraphSymmetry_PlanarRotation( 5 ) ) ) );
   _DualGraph.reset( new DualGraph( sym, shape ) );
   _DualGraph->addVertex( 0, Icosahedron()[0] );
   _DualGraph->addVertex( 2, (Icosahedron()[0]*3 + Icosahedron()[1] + Icosahedron()[2]*.7).normalized() );
   //for ( double t = .02; t < 1; t += .02 )
   //   _DualGraph->addVertex( 2, (Icosahedron()[0]*(1-t) + Icosahedron()[1]*t + Icosahedron()[2]*.01 ).normalized() );
   //_DualGraph->addVertex( 5, XYZ(0,0,0) );
   //_DualGraph->addVertex( 1, XYZ(1,0,0) );

   ui.drawing->_GraphShape = _DualGraph->shape();

   connect( ui.drawing, &Drawing::resized, [&](){ updateDrawing(); } );


   connect( ui.drawing, &Drawing::press, [this]( QMouseEvent* event ) {
      if ( event->buttons().testFlag( Qt::LeftButton ) )
         handleMouse( event->pos(), true, true, false );
      //if ( event->buttons().testFlag( Qt::RightButton ) )
      //   handleRightButton( event->pos(), true, true, false );
   } );

   connect( ui.drawing, &Drawing::move, [this]( QMouseEvent* event ) {
      handleMouse( event->pos(), true, false, false );
      //if ( event->buttons().testFlag( Qt::RightButton ) )
      //   handleRightButton( event->pos(), true, false, false );
   } );

   connect( ui.drawing, &Drawing::release, [this]( QMouseEvent* event ) {
      //if ( event->buttons().testFlag( Qt::LeftButton ) )
      handleMouse( event->pos(), false, false, true );
      ////if ( event->buttons().testFlag( Qt::RightButton ) )
      //handleRightButton( event->pos(), false, false, true );
   } );


   QObject::connect( new QShortcut(QKeySequence(Qt::Key_0), this ), &QShortcut::activated, [this]() { addVertex( 0 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_R), this ), &QShortcut::activated, [this]() { addVertex( 0 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_1), this ), &QShortcut::activated, [this]() { addVertex( 1 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_B), this ), &QShortcut::activated, [this]() { addVertex( 1 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_2), this ), &QShortcut::activated, [this]() { addVertex( 2 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_Y), this ), &QShortcut::activated, [this]() { addVertex( 2 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_3), this ), &QShortcut::activated, [this]() { addVertex( 3 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_P), this ), &QShortcut::activated, [this]() { addVertex( 3 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_4), this ), &QShortcut::activated, [this]() { addVertex( 4 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_G), this ), &QShortcut::activated, [this]() { addVertex( 4 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_5), this ), &QShortcut::activated, [this]() { addVertex( 5 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_O), this ), &QShortcut::activated, [this]() { addVertex( 5 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_6), this ), &QShortcut::activated, [this]() { addVertex( 6 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_C), this ), &QShortcut::activated, [this]() { addVertex( 6 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_7), this ), &QShortcut::activated, [this]() { addVertex( 7 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_W), this ), &QShortcut::activated, [this]() { addVertex( 7 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_8), this ), &QShortcut::activated, [this]() { addVertex( 8 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_K), this ), &QShortcut::activated, [this]() { addVertex( 8 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_9), this ), &QShortcut::activated, [this]() { addVertex( 9 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_N), this ), &QShortcut::activated, [this]() { addVertex( 9 ); } );

}

GraphUI::~GraphUI()
{
}


bool GraphUI::getMousePos( XYZ& mousePos ) const
{
   return ui.drawing->getModelPos( ui.drawing->mapFromGlobal( QCursor::pos() ), mousePos );
}

DualGraph::VertexPtr GraphUI::dualVertexAtMouse( double maxPixelDist ) const
{
   XYZ mousePos;
   if ( !getMousePos( mousePos ) )
      return DualGraph::VertexPtr();

   return _DualGraph->vertexAt( mousePos, ui.drawing->toModel( maxPixelDist ) );
}

void GraphUI::addVertex( int color )
{
   DualGraph::VertexPtr vtx = dualVertexAtMouse( 6 );
   if ( vtx.isValid() ) // change vtx color
   {
      _DualGraph->setVertexColor( vtx, color );
   }
   else                 // add vtx
   {
      XYZ mousePos;
      if ( getMousePos( mousePos ) )
         _DualGraph->addVertex( color, mousePos );
   }
   updateDrawing();
}

void GraphUI::updateDrawing()
{
   ui.drawing->updateDrawing( *_DualGraph );
}

void GraphUI::handleMouse( const QPoint& mouseBitmapPos, bool isMove, bool isClick, bool isUnclick )
{
   XYZ mousePos;
   if ( !ui.drawing->getModelPos( mouseBitmapPos, mousePos ) )
      return;

   if ( isUnclick )
   {
      if ( isKeyDown( 'E' ) )
      {
         _DualGraph->toggleEdge( _DragDualEdgeStartVtx, dualVertexAtMouse( 30. ) );
         updateDrawing();
      }

      _DragDualVtx = DualGraph::VertexPtr();
      _DragDualEdgeStartVtx = DualGraph::VertexPtr();
   }

   if ( isKeyDown( 'M' ) && isClick )
   {
      _DragDualVtx = dualVertexAtMouse( 8. );
   }

   if ( isKeyDown( 'E' ) && isClick )
   {
      _DragDualEdgeStartVtx = dualVertexAtMouse( 12. );
   }

   if ( isKeyDown( 'M' ) && isMove )
   {
      XYZ modelPos;
      if ( _DragDualVtx.isValid() )
      {
         _DualGraph->setVertexPos( _DragDualVtx, mousePos );
         updateDrawing();
      }
   }   
}