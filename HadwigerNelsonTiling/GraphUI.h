#pragma once

#include <QWidget>
#include <QTimer>
#include "ui_GraphUI.h"

#include <Core/DualGraph.h>
#include <Core/TileGraph.h>

class Simulation;

class GraphUI : public QWidget
{
   Q_OBJECT

public:
   GraphUI( QWidget *parent = Q_NULLPTR );
   ~GraphUI();

   void updateDrawing();
   void addVertex( int color );

   void handleMouse( const QPoint& mouseBitmapPos, bool isMove, bool isClick, bool isUnclick );

   bool getMousePos( XYZ& pos ) const;
   DualGraph::VertexPtr dualVertexAtMouse( double maxPixelDist ) const;
   TileGraph::VertexPtr tileVertexAtMouse( double maxPixelDist ) const;

   void loadHardcodedGraph( int index );
   void setRadius( double radius );

private:
   Ui::GraphUI ui;

   QTimer _Timer;
   std::shared_ptr<Simulation> _Simulation;
   DualGraph::VertexPtr _DragDualVtx;
   DualGraph::VertexPtr _DragDualEdgeStartVtx;
   TileGraph::VertexPtr _DragTileVtx;
};
