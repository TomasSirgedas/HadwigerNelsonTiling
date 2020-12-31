#include "HadwigerNelsonTiling.h"
#include "GraphUI.h"

HadwigerNelsonTiling::HadwigerNelsonTiling( QWidget *parent )
   : QMainWindow( parent )
{
   ui.setupUi( this );

   setCentralWidget( new GraphUI( this ) );
   
   resize( QSize( 900, 900 ) );
}
