#pragma once

#include <QWidget>
#include "ui_GraphUI.h"

#include <Core/DualGraph.h>

class GraphUI : public QWidget
{
   Q_OBJECT

public:
   GraphUI( QWidget *parent = Q_NULLPTR );
   ~GraphUI();

   void updateDrawing();
   void addVertex( int color );

   void handleMouse( const XYZ& mousePos, bool isMove, bool isClick, bool isUnclick );

   XYZ mousePos() const;
   DualGraph::VertexPtr dualVertexAtMouse( double maxPixelDist ) const;

private:
   Ui::GraphUI ui;

   std::shared_ptr<DualGraph> _DualGraph;
   DualGraph::VertexPtr _DragDualVtx;
   DualGraph::VertexPtr _DragDualEdgeStartVtx;
};
