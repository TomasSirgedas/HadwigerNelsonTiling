#include "GraphUI.h"
#include "PlatformSpecific.h"

#include <Core/DataTypes.h>
#include <Core/GraphUtil.h>
#include <Core/Util.h>
#include <Core/Simulation.h>
#include <Core/DualAnalysis.h>

#include <QShortcut>
#include <QMouseEvent>
#include <QDebug>
#include <QValidator>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>
#include <QElapsedTimer>

namespace
{
   QJsonValue toQJsonValue( const Json& j )
   {
      QJsonValue ret;
      switch ( j.type() )
      {
      case Json::OBJECT: { QJsonObject obj; for ( const auto& e : j.toMap() ) obj[QString::fromStdString(e.first)] = toQJsonValue( e.second ); ret = obj; break; }
      case Json::ARRAY: { QJsonArray arr; for ( const Json& e : j.toArray() ) arr.append( toQJsonValue( e ) ); ret = arr; break; }
      case Json::STRING: { ret = QString::fromStdString( j.toString() ); break; }
      case Json::NUMBER: { ret = j.toDouble(); break; }
      case Json::BOOL: { ret = j.toBool(); break; }
      }
      return ret;
   }
   Json toJson( const QJsonValue& j )
   {      
      Json ret;
      switch ( j.type() )
      {
      case QJsonValue::Object: { QJsonObject obj = j.toObject(); for ( const auto& key : obj.keys() ) ret[key.toStdString()] = toJson( obj[key] ); break; }
      case QJsonValue::Array: { QJsonArray arr = j.toArray(); for ( const auto& val : arr ) ret.push_back( toJson( val ) ); break; }
      case QJsonValue::String: { ret = j.toString().toStdString(); break; }
      case QJsonValue::Double: { ret = j.toDouble(); break; }
      case QJsonValue::Bool: { ret = j.toBool(); break; }
      }
      return ret;
   }
   void saveDual( const QString& filename, const DualGraph& dual )
   {
      if ( filename.isEmpty() )
         return;

      QJsonObject graph = toQJsonValue( dual.toJson() ).toObject();
      {
         QFile f( filename );
         f.open(QFile::WriteOnly);
         f.write(QJsonDocument( graph ).toJson());            
      }   
   }
   std::shared_ptr<DualGraph> loadDual( const QString& filename )
   {
      if ( filename.isEmpty() )
         return nullptr;

      QFile f( filename );
      f.open( QFile::ReadOnly );
      QJsonDocument doc = QJsonDocument::fromJson( f.readAll() );
      Json j = toJson( doc.object() );
      return std::shared_ptr<DualGraph>( new DualGraph( j ) );
   }

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

      if ( index == 5 )
      {
         std::shared_ptr<IGraphShape> shape( new GraphShapePlane );

         SymmetryGroup symA( Matrix4x4::rotation( XYZ( 0, 0, 1 ), PI*2/5 ), Perm( { 0,4,5,1,2,3,6,7,8,9 } ) );
         std::shared_ptr<IGraphSymmetry> sym( new GraphSymmetry_Groups( { symA } ) );

         dual.reset( new DualGraph( sym, shape ) );
         dual->addVertex( 0, XYZ( 0, 0, 0 ) );
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

   //loadGraph( hardcodedDualGraph( 3 ) );
   //loadGraph( loadDual( R"(C:\Users\Tom\Desktop\dual\temp.dual)" ) );
   //loadGraph( loadDual( R"(D:/Proj/HadwigerNelson/HadwigerNelsonTiling/HadwigerNelsonTiling/1.dual)" ) );
   //loadGraph( loadDual( R"(D:\Proj\HadwigerNelson\SphereColoring\SphereColoring\GP4_3$.dual)" ) );
   loadGraph( loadDual( R"(temp.dual)" ) );

      



   //for ( double t = .02; t < 1; t += .02 )
   //   _DualGraph->addVertex( 2, (Icosahedron()[0]*(1-t) + Icosahedron()[1]*t + Icosahedron()[2]*.01 ).normalized() );
   //_DualGraph->addVertex( 5, XYZ(0,0,0) );
   //_DualGraph->addVertex( 1, XYZ(1,0,0) );

   ui.radiusLineEdit->setValidator( new QDoubleValidator( .3, 100., 1000, this ) );


   connect( ui.saveButton, &QPushButton::clicked, [&]() {
      QString filename = QFileDialog::getSaveFileName( this, "Save Graph", QString(), "*.dual" );
      saveDual( filename, *_Simulation->_DualGraph );
   } );
   connect( ui.loadButton, &QPushButton::clicked, [&]() {
      QString filename = QFileDialog::getOpenFileName( this, "Save Graph", QString(), "*.dual" );
      loadGraph( loadDual( filename ) );
   } );
      
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
      int simSteps = 50;
      QElapsedTimer t;
      t.start();
      double error = _Simulation->step( simSteps );
      double time = t.nsecsElapsed() * 1e-9;
      ui.errorLabel->setText( "Err:" + QString::number( error ) );
      ui.errorLabel->setText( "Err:" + QString::number( error ) );
      ui.paddingErrorLabel->setText( "Pad:" + QString::number( _Simulation->_PaddingError ) );      
      //ui.speedLabel->setText( "Speed(ms): " + QString::number( 1000*time/simSteps ) );
      updateDrawing();
   } );

   connect( ui.dualToTileButton, &QPushButton::clicked, [&](){  
      _Simulation->_TileGraph = makeTileGraph( *_Simulation->_DualGraph, 1. );
      _Simulation->init( _Simulation->_TileGraph );
      updateDrawing();
   } );

   connect( ui.centroidButton, &QPushButton::clicked, [&](){  
      _Simulation->moveDualVerticesToCentroid();
      updateDrawing();
   } );   
   
   connect( ui.showRigidsCheckBox, &QCheckBox::toggled, [&]() {
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

   connect( ui.outerRadiusSlider, &QSlider::valueChanged, [this]( int value ) {
      double r = value / 1000.;
      ui.outerRadiusEdit->setText( QString("%1").arg( r ) );
      _Simulation->_OuterRadius = r;
      updateModelFromUI();
      updateDrawing();
   } );
   connect( ui.innerRadiusSlider, &QSlider::valueChanged, [this]( int value ) {
      double r = value / 1000.;
      ui.innerRadiusEdit->setText( QString("%1").arg( r ) );
      _Simulation->_InnerRadius = r;
      updateModelFromUI();
      updateDrawing();
   } );
   connect( ui.stripWidthSlider, &QSlider::valueChanged, [this]( int value ) {
      double w = value / 1000.;
      ui.stripWidthEdit->setText( QString("%1").arg( w ) );
      _Simulation->_StripWidth = w;
      updateModelFromUI();
      updateDrawing();
   } );
   connect( ui.stripHeightSlider, &QSlider::valueChanged, [this]( int value ) {
      double h = value / 1000.;
      ui.stripHeightEdit->setText( QString("%1").arg( h ) );
      _Simulation->_StripHeight = h;
      updateModelFromUI();
      updateDrawing();
   } );
   connect( ui.outerRadiusEdit, &QLineEdit::editingFinished, [this](){
      ui.outerRadiusSlider->setValue( lround( ui.outerRadiusEdit->text().toDouble()*1000 ) );
   } );
   connect( ui.innerRadiusEdit, &QLineEdit::editingFinished, [this](){
      ui.innerRadiusSlider->setValue( lround( ui.innerRadiusEdit->text().toDouble()*1000 ) );
   } );
   connect( ui.stripWidthEdit, &QLineEdit::editingFinished, [this](){
      ui.stripWidthSlider->setValue( lround( ui.stripWidthEdit->text().toDouble()*1000 ) );
   } );
   connect( ui.stripHeightEdit, &QLineEdit::editingFinished, [this](){
      ui.stripHeightSlider->setValue( lround( ui.stripHeightEdit->text().toDouble()*1000 ) );
   } );
   connect( ui.zoomSlider, &QSlider::valueChanged, [this]( int value ) {
      ui.drawing->_Zoom = exp( ( value / 100. - .5 ) * 5 );
      ui.drawing->refresh();
      updateDrawing();
   } );
   connect( ui.diskRadioButton, &QAbstractButton::clicked, [this]() { updateModelFromUI(); updateDrawing(); } );
   connect( ui.stripRadioButton, &QAbstractButton::clicked, [this]() { updateModelFromUI(); updateDrawing(); } );
   connect( ui.noneRadioButton, &QAbstractButton::clicked, [this]() { updateModelFromUI(); updateDrawing(); } );
   ui.zoomSlider->valueChanged( ui.zoomSlider->value() );

   connect( ui.tileDistEdit, &QLineEdit::editingFinished, [this]() {
      _Simulation->_TileDist = ui.tileDistEdit->text().toDouble();
      updateDrawing();
   } );


   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F1), this ), &QShortcut::activated, [this]() { loadGraph( hardcodedDualGraph( 1 ) ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F2), this ), &QShortcut::activated, [this]() { loadGraph( hardcodedDualGraph( 2 ) ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F3), this ), &QShortcut::activated, [this]() { loadGraph( hardcodedDualGraph( 3 ) ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F4), this ), &QShortcut::activated, [this]() { loadGraph( hardcodedDualGraph( 4 ) ); } );
   QObject::connect( new QShortcut(QKeySequence(Qt::Key_F5), this ), &QShortcut::activated, [this]() { loadGraph( hardcodedDualGraph( 5 ) ); } );

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

   QObject::connect( new QShortcut(QKeySequence(Qt::Key_Delete), this ), &QShortcut::activated, [this]() { deleteVertex(); } );

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

void GraphUI::loadGraph( std::shared_ptr<DualGraph> dual )
{
   if ( !dual )
      return;

   _Simulation->_DualGraph = dual;
   _Simulation->_TileGraph = nullptr;
   _DragDualVtx            = DualGraph::VertexPtr();
   _DragDualEdgeStartVtx   = DualGraph::VertexPtr();
   _DragTileVtx            = TileGraph::VertexPtr();
   ui.drawing->setGraphShape( _Simulation->_DualGraph->shape() );
   setRadius( _Simulation->_DualGraph->shape()->radius() );
   onDualGraphModified();
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
   onDualGraphModified();
   updateDrawing();
}

void GraphUI::deleteVertex()
{
   DualGraph::VertexPtr vtx = dualVertexAtMouse( 100 );
   if ( !vtx.isValid() ) 
      return;

   _Simulation->_DualGraph->deleteVertex( vtx );
   onDualGraphModified();
   updateDrawing();
}

void GraphUI::updateDrawing()
{
   QElapsedTimer t;
   t.start();

   ui.drawing->updateDrawing( _Simulation, _DualAnalysis );

   double time = t.nsecsElapsed() * 1e-9;   
   ui.speedLabel->setText( "Speed(ms): " + QString::number( 1000*time ) );
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
         onDualGraphModified();
         updateDrawing();
      }
      if ( isKeyDown( 'D' ) )
      {
         _Simulation->_ShowDistanceVertices.first = _DistanceTileVtx.id();
         _Simulation->_ShowDistanceVertices.second = tileVertexAtMouse( 30. ).id();
         updateDrawing();
      }
      onDualGraphModified();

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
      else if ( isKeyDown( 'D' ) )
         _DistanceTileVtx = tileVertexAtMouse( 20. );
      else
         _Simulation->_FixedVertex = _DragTileVtx = tileVertexAtMouse( 8. );

      //if ( _DragTileVtx.isValid() )
      //{
      //   std::trace << "_DragTileVtx.tiles() = " << _DragTileVtx.tiles() << std::endl;
      //   std::trace << "_DragTileVtx.neighbors() = " << _DragTileVtx.neighbors() << std::endl;
      //}
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

void GraphUI::onDualGraphModified()
{
   if ( !_Simulation->_DualGraph )
   {
      _DualAnalysis.reset();
      return;
   }

   _Simulation->_DualGraph->sortNeighbors();
   _DualAnalysis.reset( new DualAnalysis( *_Simulation->_DualGraph ) );
}

void GraphUI::updateModelFromUI()
{
   _Simulation->_OuterRadius = ui.diskRadioButton->isChecked() ? ui.outerRadiusEdit->text().toDouble() : 0;
   _Simulation->_InnerRadius = ui.diskRadioButton->isChecked() ? ui.innerRadiusEdit->text().toDouble() : 0;
   _Simulation->_StripWidth  = ui.stripRadioButton->isChecked() ? ui.stripWidthEdit->text().toDouble() : 0;
   _Simulation->_StripHeight = ui.stripRadioButton->isChecked() ? ui.stripHeightEdit->text().toDouble() : 0;
}