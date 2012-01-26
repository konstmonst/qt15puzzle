#include "mainwindow.h"
#include "state.h"
int main(int argc, char** argv)
{    
	QApplication app(argc, argv);
    Q_INIT_RESOURCE(icons);    

    Q_ASSERT(stateTests());

    MainWindow win;
	win.show();
	return app.exec();
}
