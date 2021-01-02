#pragma once

#include <QWidget>
#include "ui_GraphUI.h"

#include <Core/DualGraph.h>
#include <Core/TileGraph.h>

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

private:
   Ui::GraphUI ui;

   std::shared_ptr<DualGraph> _DualGraph;
   std::shared_ptr<TileGraph> _TileGraph;
   DualGraph::VertexPtr _DragDualVtx;
   DualGraph::VertexPtr _DragDualEdgeStartVtx;
};
