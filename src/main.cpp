#include "TxtReader.h"
#include <QTextCodec>
int main(int argc,char ** argv)
{
	QApplication app(argc,argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	TxtReader * txtReader = new TxtReader(0,argv[1]);
	//txtReader->setWindowFlags(Qt::FramelessWindowHint);
	//txtReader->setWindowTitle(QObject::tr("TXT Reader --- By lsyer"));
	txtReader->setMinimumSize(200,150);
	//txtReader->setMaximumSize(1000,750);
	//txtReader->resize(800,600);
	txtReader->show();
	return app.exec();
}
