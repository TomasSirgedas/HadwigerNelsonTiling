#include "GraphUI.h"
#include "PlatformSpecific.h"

#include <Core/DataTypes.h>
#include <Core/GraphUtil.h>
#include <Core/Util.h>
#include <Core/Simulation.h>

#include <QShortcut>
#include <QMouseEvent>
#include <QDebug>
#include <QValidator>


namespace
{
   std::shared_ptr<DualGraph> hardcodedDualGraph( int index )
   {
      std::shared_ptr<DualGraph> dual;

      // ico-symmetry sphere
      if ( index == 1 )
      {
         std::shared_ptr<IGraphShape> shape( new GraphShapeSphere( 2.5 ) );

         SymmetryGroup symA( Icosahedron().map( {0,1,2}, { 5,4,8} ), Perm( { 5,4,2,3,1,0,6,7,8,9 } ) );
         SymmetryGroup symB( Icosahedron().map( {0,1,2}, {11,7,3} ), Perm( { 5,1,3,2,4,0,6,7,8,9 } ) );
         SymmetryGroup symC( Icosahedron().map( {0,1,2}, { 1,2,0} ), Perm( { 1,2,0,5,3,4,6,7,8,9 } ) );
         SymmetryGroup symD( Icosahedron().map( {0,1,2}, { 0,2,3} ), Perm( { 0,2,3,4,5,1,6,7,8,9 } ) );
         std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( { symA, symB, symC, symD } ) );

         dual.reset( new DualGraph( sym, shape ) );
         dual->addVertex( 0, Icosahedron()[0] );
         dual->addVertex( 5, (Icosahedron()[0]*3 + Icosahedron()[1] + Icosahedron()[2]*.7).normalized() );
         dual->toggleEdge( dual->vertexWithId( 0 ), dual->vertexWithId( 1 ) );
         dual->toggleEdge( dual->vertexWithId( 1 ), dual->vertexWithId( 5001 ) );
         dual->toggleEdge( dual->vertexWithId( 1 ), dual->vertexWithId( 1001 ) );
         dual->toggleEdge( dual->vertexWithId( 1 ), dual->vertexWithId( 6001 ) );
      }

      if ( index == 2 )
      {
         std::shared_ptr<IGraphShape> shape( new GraphShapePlane );

         SymmetryGroup symA( Matrix4x4::translation( XYZ( 0, 2, 0 ) ), Perm( { 1,2,3,4,5,6,0,7,8,9 } ), -1, 2 );
         SymmetryGroup symB( Matrix4x4::translation( XYZ( 3, 0, 0 ) ), Perm( { 1,2,3,4,5,6,0,7,8,9 } ).pow( 5 ), -1, 2 );
         std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( { symA, symB } ) );

         dual.reset( new DualGraph( sym, shape ) );
         dual->addVertex( 0, XYZ( 0, 0, 0 ) );
         dual->addVertex( 3, XYZ( 1.5, 1, 0 ) );

         dual->toggleEdge( dual->vertexWithId( 0 ), dual->vertexWithId( 1 ) );
         dual->toggleEdge( dual->vertexWithId( 0 ), dual->vertexWithId( 16000 ) );
         dual->toggleEdge( dual->vertexWithId( 0 ), dual->vertexWithId( 240001 ) );
         dual->toggleEdge( dual->vertexWithId( 0 ), dual->vertexWithId( 255001 ) );
         dual->toggleEdge( dual->vertexWithId( 0 ), dual->vertexWithId( 15001 ) );
         dual->toggleEdge( dual->vertexWithId( 15001 ), dual->vertexWithId( 255001 ) );
      }


      if ( index == 3 )
      {
         std::shared_ptr<IGraphShape> shape( new GraphShapePlane );

         std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( {} ) );

         dual.reset( new DualGraph( sym, shape ) );
         dual->addVertex( 0, XYZ( 0, 0, 0 ) );
         dual->addVertex( 1, XYZ( 1.5, 1, 0 ) );
         dual->addVertex( 2, XYZ( 2, -1, 0 ) );
         dual->addVertex( 3, XYZ( .5, -1, 0 ) );
         dual->addVertex( 4, XYZ( 1, 0, 0 ) );
         dual->addVertex( 0, XYZ( 1, -1.5, 0 ) );
         dual->addVertex( 1, XYZ( .5, -1.5, 0 ) );

         dual->toggleEdge( 0, 1 );
         dual->toggleEdge( 1, 2 );
         dual->toggleEdge( 2, 3 );
         dual->toggleEdge( 3, 0 );
         dual->toggleEdge( 0, 4 );
         dual->toggleEdge( 1, 4 );
         dual->toggleEdge( 2, 4 );
         dual->toggleEdge( 3, 4 );
         dual->toggleEdge( 3, 6 );
         dual->toggleEdge( 2, 5 );
         dual->toggleEdge( 5, 6 );
      }

      if ( index == 4 )
      {
         std::shared_ptr<IGraphShape> shape( new GraphShapePlane );

         SymmetryGroup symA( Matrix4x4::translation( XYZ( 0, 2, 0 ) ), Perm( { 1,2,3,4,5,6,0,7,8,9 } ), -1, 2 );
         std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( { symA } ) );

         dual.reset( new DualGraph( sym, shape ) );
         dual->addVertex( 0, XYZ( 0, 0, 0 ) );
         dual->addVertex( 4, XYZ( 1.5, 1, 0 ) );
         dual->addVertex( 6, XYZ( 2, -1, 0 ) );
         dual->addVertex( 7, XYZ( .5, -1, 0 ) );
         dual->addVertex( 2, XYZ( 1, 0, 0 ) );

         dual->toggleEdge( 0, 4 );
         dual->toggleEdge( 0, 3 );
         dual->toggleEdge( 0, 1003 );
         dual->toggleEdge( 4, 1003 );
         dual->toggleEdge( 4, 1 );
         dual->toggleEdge( 4, 3 );
         dual->toggleEdge( 4, 15001 );
         dual->toggleEdge( 1, 1002 );
         dual->toggleEdge( 1, 1003 );
         dual->toggleEdge( 2, 1002 );
      }

      return dual;
   }
}


GraphUI::GraphUI( QWidget *parent )
   : QWidget( parent )
{
   ui.setupUi( this );

   //SymmetryGroup g5( Matrix4x4::rotation( XYZ(0,0,1), 2*PI/5 ), Perm( { 1,2,3,4,0,5,6,7,8,9 } ) );
   //SymmetryGroup g3( Matrix4x4::rotation( XYZ(0,0,1), 2*PI/3 ) * Matrix4x4::translation( XYZ(3,0,0) ), Perm( { 1,2,0,3,4,5,6,7,8,9 } ) );   
   //std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( { g3, g5 } ) );

   _Simulation.reset( new Simulation );

   loadHardcodedGraph( 3 );



   //for ( double t = .02; t < 1; t += .02 )
   //   _DualGraph->addVertex( 2, (Icosahedron()[0]*(1-t) + Icosahedron()[1]*t + Icosahedron()[2]*.01 ).normalized() );
   //_DualGraph->addVertex( 5, XYZ(0,0,0) );
   //_DualGraph->addVertex( 1, XYZ(1,0,0) );

   ui.radiusLineEdit->setValidator( new QDoubleValidator( .3, 100., 1000, this ) );
      
   connect( ui.radiusLineEdit, &QLineEdit::editingFinished, [&]() {
      setRadius( ui.radiusLineEdit->text().toDouble() );
   } );

   connect( ui.playButton, &QPushButton::clicked, [&]() {  
      if ( _Timer.isActive() )
      {
         _Timer.stop();
         ui.playButton->setText( "Play" );
      }
      else
      {
         _Timer.start( 50 );
         ui.playButton->setText( "Pause" );
      }
   } );

   connect( &_Timer, &QTimer::timeout, [this]() {
      double error = _Simulation->step( 50 );
      ui.errorLabel->setText( "Err:" + QString::number( error ) );
      ui.paddingErrorLabel->setText( "Pad:" + QString::number( _Simulation->_PaddingError ) );      
      updateDrawing();
   } );

   connect( ui.dualToTileButton, &QPushButton::clicked, [&](){  
      _Simulation->_TileGraph = makeTileGraph( *_Simulation->_DualGraph, 1. );
      _Simulation->init( _Simulation->_TileGraph );
      updateDrawing();
   } );

   
   connect( ui.showTileGraphCheckBox, &QCheckBox::toggled, [&]() {
      ui.drawing->_ShowRigids = ui.showRigidsCheckBox->isChecked();
      updateDrawing();
   } );

   connect( ui.showTileGraphCheckBox, &QCheckBox::toggled, [&]() {
      ui.drawing->_ShowTileGraph = ui.showTileGraphCheckBox->isChecked();
      updateDrawing();
   } );

   connect( ui.showDualGraphCheckBox, &QCheckBox::toggled, [&]() {
      ui.drawing->_ShowDualGraph = ui.showDualGraphCheckBox->isChecked();
      updateDrawing();
   } );

   connect( ui.showLabelsCheckBox, &QCheckBox::toggled, [&]() {
      ui.drawing->_ShowLabels = ui.showLabelsCheckBox->isChecked();
      updateDrawing();
   } );

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


   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F1), this ), &QShortcut::activated, [this]() { loadHardcodedGraph( 1 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F2), this ), &QShortcut::activated, [this]() { loadHardcodedGraph( 2 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F3), this ), &QShortcut::activated, [this]() { loadHardcodedGraph( 3 ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F4), this ), &QShortcut::activated, [this]() { loadHardcodedGraph( 4 ); } );

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

void GraphUI::setRadius( double radius )
{
   ui.radiusLineEdit->setText( QString::number( radius ) );
   _Simulation->setRadius( ui.radiusLineEdit->text().toDouble() );
   ui.drawing->refresh();
}

void GraphUI::loadHardcodedGraph( int index )
{
   _Simulation->_DualGraph = hardcodedDualGraph( index );
   _Simulation->_TileGraph = nullptr;
   _DragDualVtx            = DualGraph::VertexPtr();
   _DragDualEdgeStartVtx   = DualGraph::VertexPtr();
   _DragTileVtx            = TileGraph::VertexPtr();
   ui.drawing->setGraphShape( _Simulation->_DualGraph->shape() );
   setRadius( _Simulation->_DualGraph->shape()->radius() );
   updateDrawing();
}

bool GraphUI::getMousePos( XYZ& mousePos ) const
{
   return ui.drawing->getModelPos( ui.drawing->mapFromGlobal( QCursor::pos() ), mousePos );
}

DualGraph::VertexPtr GraphUI::dualVertexAtMouse( double maxPixelDist ) const
{
   if ( !_Simulation->_DualGraph )
      return DualGraph::VertexPtr();

   XYZ mousePos;
   if ( !getMousePos( mousePos ) )
      return DualGraph::VertexPtr();

   return _Simulation->_DualGraph->vertexAt( mousePos, ui.drawing->toModel( maxPixelDist ) );
}

TileGraph::VertexPtr GraphUI::tileVertexAtMouse( double maxPixelDist ) const
{
   if ( !_Simulation->_TileGraph )
      return TileGraph::VertexPtr();

   XYZ mousePos;
   if ( !getMousePos( mousePos ) )
      return TileGraph::VertexPtr();

   return _Simulation->_TileGraph->vertexAt( mousePos, ui.drawing->toModel( maxPixelDist ) );
}

void GraphUI::addVertex( int color )
{
   DualGraph::VertexPtr vtx = dualVertexAtMouse( 6 );
   if ( vtx.isValid() ) // change vtx color
   {
      _Simulation->_DualGraph->setVertexColor( vtx, color );
   }
   else                 // add vtx
   {
      XYZ mousePos;
      if ( getMousePos( mousePos ) )
         _Simulation->_DualGraph->addVertex( color, mousePos );
   }
   updateDrawing();
}

void GraphUI::updateDrawing()
{
   ui.drawing->updateDrawing( _Simulation );
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
         _Simulation->_DualGraph->toggleEdge( _DragDualEdgeStartVtx, dualVertexAtMouse( 30. ) );
         updateDrawing();
      }

      _DragDualVtx = DualGraph::VertexPtr();
      _DragDualEdgeStartVtx = DualGraph::VertexPtr();      
      _Simulation->_FixedVertex = _DragTileVtx = TileGraph::VertexPtr();
   }

   if ( isClick )
   {
      if ( isKeyDown( 'E' ) )
         _DragDualEdgeStartVtx = dualVertexAtMouse( 12. );
      else if ( isKeyDown( 'M' ) )
         _DragDualVtx = dualVertexAtMouse( 8. );
      else
         _Simulation->_FixedVertex = _DragTileVtx = tileVertexAtMouse( 8. );

      if ( _DragTileVtx.isValid() )
      {
         std::trace << "_DragTileVtx.tiles() = " << _DragTileVtx.tiles() << std::endl;
         std::trace << "_DragTileVtx.neighbors() = " << _DragTileVtx.neighbors() << std::endl;
      }
   }

   if ( isMove )
   {
      if ( isKeyDown( 'M' ) )
      {
         if ( _DragDualVtx.isValid() )
         {
            _Simulation->_DualGraph->setVertexPos( _DragDualVtx, mousePos );
            updateDrawing();
         }
      }
      else
      {
         if ( _DragTileVtx.isValid() )
         {
            _Simulation->_TileGraph->setVertexPos( _DragTileVtx, mousePos );
            updateDrawing();
         }
      }
   }   
}