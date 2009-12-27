#include "TxtReader.h"
//#include <QDebug>
TxtReader::TxtReader(QWidget * parent,QString infile)
	:QWidget(parent)
{
	//w1 = 0;
	version = QString("0.5.2");
	setWindowTitle( QString(tr("TXT Reader %1 --- By lsyer")).arg(version));
	qApp->installTranslator(&appTranslator);
	setAcceptDrops(true);
	
    QSettings settings("lsyer", "txtreader");
	QPoint pos = settings.value("pos", QPoint(100, 60)).toPoint();
	QSize size = settings.value("size", QSize(800, 600)).toSize();
	curFile = settings.value("curFile", QString("")).toString();
	txtFont.fromString(settings.value("txtFont"/*,txtFont.toString()*/).toString());
	txtColor=QColor(settings.value("txtColor", QColor("#000000")).toString());
	index = settings.value("index", 1).toInt();
	o_isfullscreen = settings.value("o_isfullscreen", 0).toBool();
	bgColor=QColor(settings.value("bgColor", QColor("#ffffff")).toString());
	bgImage = settings.value("bgImage", QString("")).toString();
	codecstr = settings.value("codecstr", QString("GB2312")).toString();
	uilang = settings.value("uilang", QString("zh_CN")).toString();
	int j = settings.value("bookmarknum", 0).toInt()+1;
	for(int k=1;k < j;++k){
		QAction * bookmark = new QAction(settings.value(tr("bookmark%1words").arg(k)/*, QString(tr("未知标签"))*/).toString(), this);
		bookmark->setData(settings.value(tr("bookmark%1index").arg(k)/*, 1*/).toInt());
		connect(bookmark, SIGNAL(triggered()), this, SLOT(openBookmark()));
		bookmarkArray[settings.value(tr("bookmark%1index").arg(k)/*, 1*/).toInt()] = bookmark;
	}

	createActions();
	createLanguageMenu();
	createTrayIcon();
	setStyleSheet( "TxtReader {background-color: "+bgColor.name()+";background-image: url("+bgImage+");background-position:center;;}");
	doc.setDefaultFont(txtFont);
	
	if(o_isfullscreen){
		viewfullscreenAction->setEnabled(false);
		nofullscreenAction->setEnabled(true);
		showFullScreen();
	}else{
		viewfullscreenAction->setEnabled(true);
		nofullscreenAction->setEnabled(false);
		resize(size);
		move(pos);
	}
	restoreAction->setEnabled(false);

	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
	icon = QIcon(":/images/icon.png");
	trayIcon->setIcon(icon);
	this->setWindowIcon(icon);
	trayIcon->setToolTip(QString("TxtReader %1").arg(version));
	trayIcon->show();
	QGridLayout *layout = new QGridLayout;
	viewer=new readerView(&doc,this);
	viewer->ctx.palette.setColor(QPalette::Text, txtColor);
	//viewer->viewpage(index);
	pagelabel.setFixedHeight(15);
	pagelabel.setAlignment(Qt::AlignRight);
	//pagelabel.setText(QString(tr("第 %1 页/共 %2 页")).arg(index).arg(doc.pageCount()));
	layout->addWidget(viewer,0,0);
	layout->addWidget(&pagelabel,1,0);
	this->setLayout(layout);
	connect(viewer,SIGNAL(pagecountchanged()),this,SLOT(changepagecount()));
	
    appTranslator.load(":/i18n/txtreader_" + uilang);
	retranslateUi();
	//qDebug()<<"infile2:"<< infile <<"\n";
	if(infile.isEmpty())
		loadFile(curFile);
	else
		loadFile(infile);
}

void TxtReader::resizeEvent(QResizeEvent *)
{
}

void TxtReader::changepagecount()
{
	setindex(index);
}

void TxtReader::setindex(int newindex)
{
	if(newindex < 1)
		index = 1;
	else if(newindex > doc.pageCount())
		index = doc.pageCount();
	else
		index = newindex;
	
	viewer->viewpage(index);
	pagelabel.setText(QString(tr("Page %1 /Total %2")).arg(index).arg(doc.pageCount()));
}
int TxtReader::loadFile(const QString &fileName)
{
	if (fileName.isEmpty()) {
		showinstruction();
		return 1;
    }
    if (curFile!=fileName) {
		qDeleteAll(bookmarkArray);
		bookmarkArray.clear();
        setindex(1);
    }
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		showinstruction();
		return 1;
    }

	QTextStream in(&file);
	//printf("codec:%s\n",(const char *)codecstr.toLocal8Bit());
	in.setCodec((const char *)codecstr.toLocal8Bit());
	QApplication::setOverrideCursor(Qt::WaitCursor);
	doc.setPlainText(in.readAll());
	QApplication::restoreOverrideCursor();

	setCurrentFile(fileName);
	//statusBar()->showMessage(tr("File loaded"), 2000);
	return 1;
}

void TxtReader::readOrInstruction(){
	static int i = 0;
	static int page;
	i = ++i % 2;
	if(i){
		page = index;
		showinstruction();
		instructionAct->setText(tr("&Go Back"));
	}else{
		loadFile(curFile);
		setindex(page);
		instructionAct->setText(tr("&Instruction"));
	}
}

void TxtReader::showinstruction(){
	QFile file(QString(":/README_%1.HTML").arg(uilang));
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, QString("TxtReader %1").arg(version),
				tr("Cannot read file,please contact to author!"));
		return;
    }

	QTextStream in(&file);
	in.setCodec("UTF-8");
	doc.setHtml(in.readAll());
	setindex(1);
}
void TxtReader::createActions()
{
    openAct = new QAction(QIcon(":/images/open.png"),"", this);
    //openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    jumpAct = new QAction(QIcon(":/images/jump.png"),"", this);
    //openAct->setShortcut(tr("Ctrl+O"));
    connect(jumpAct, SIGNAL(triggered()), this, SLOT(jumptoindex()));

    addBookmarkAct = new QAction(QIcon(":/images/addbookmark.png"),"", this);
    //openAct->setShortcut(tr("Ctrl+O"));
    connect(addBookmarkAct, SIGNAL(triggered()), this, SLOT(addBookmark()));

    delBookmarkAct = new QAction(QIcon(":/images/delbookmark.png"),"", this);
    //openAct->setShortcut(tr("Ctrl+O"));
    connect(delBookmarkAct, SIGNAL(triggered()), this, SLOT(delBookmark()));

    addTxtSizeAct = new QAction(QIcon(":/images/zoom-in.png"),"", this);
    //openAct->setShortcut(tr("Ctrl+O"));
    connect(addTxtSizeAct, SIGNAL(triggered()), this, SLOT(addTxtSize()));

    subTxtSizeAct = new QAction(QIcon(":/images/zoom-out.png"),"", this);
    //openAct->setShortcut(tr("Ctrl+O"));
    connect(subTxtSizeAct, SIGNAL(triggered()), this, SLOT(subTxtSize()));

    setfontAct = new QAction(QIcon(":/images/setfont.png"),"", this);
    connect(setfontAct, SIGNAL(triggered()), this, SLOT(setfont()));

    setfontcolorAct = new QAction(QIcon(":/images/bgcolor.png"),"", this);
    connect(setfontcolorAct, SIGNAL(triggered()), this, SLOT(setfontcolor()));

    setbgcolorAct = new QAction(QIcon(":/images/bgcolor.png"),"", this);
    connect(setbgcolorAct, SIGNAL(triggered()), this, SLOT(setbgcolor()));

    setbgimageAct = new QAction(QIcon(":/images/bgimage.png"),"", this);
    connect(setbgimageAct, SIGNAL(triggered()), this, SLOT(setbgimage()));
    
    delbgimageAct = new QAction(QIcon(":/images/resetbg.png"),"", this);
    connect(delbgimageAct, SIGNAL(triggered()), this, SLOT(delbgimage()));

    codecActionGroup = new QActionGroup(this);
    connect(codecActionGroup, SIGNAL(triggered(QAction *)),
            this, SLOT(setcodecstr(QAction *)));
    GB2312Act = new QAction(tr("&GB2312"), this);
    GB2312Act->setData("GB2312");
    GB2312Act->setCheckable(true);
	codecActionGroup->addAction(GB2312Act);
	if (codecstr == "GB2312") GB2312Act->setChecked(true);

    UTF8Act = new QAction(tr("&UTF-8"), this);
    UTF8Act->setData("UTF-8");
    UTF8Act->setCheckable(true);
	codecActionGroup->addAction(UTF8Act);
	if (codecstr == "UTF-8") UTF8Act->setChecked(true);

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    //maximizeAction = new QAction(tr("Ma&ximize"), this);
    //connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));
    
    viewfullscreenAction = new QAction(QIcon(":/images/fullscreen.png"),"", this);
    connect(viewfullscreenAction, SIGNAL(triggered()), this, SLOT(myshowfullscreen()));
    
    nofullscreenAction = new QAction(QIcon(":/images/nofullscreen.png"),"", this);
    connect(nofullscreenAction, SIGNAL(triggered()), this, SLOT(myexitfullscreen()));
    
    hiddenAction = new QAction(QIcon(":/images/min.png"),"", this);
    connect(hiddenAction, SIGNAL(triggered()), this, SLOT(myhide()));

    restoreAction = new QAction(QIcon(":/images/restore.png"),"", this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(myrestore()));

    quitAction = new QAction(QIcon(":/images/exit.png"),"", this);
    //quitAction->setShortcut(tr("Ctrl+Q"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitaction()));

    aboutAct = new QAction(QIcon(":/images/about.png"),"", this);
    //aboutAct->setShortcut(tr("Ctrl+A"));
    //aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    instructionAct = new QAction(QIcon(":/images/instruction.png"),"", this);
    //instructionAct->setShortcut(tr("Ctrl+A"));
    //instructionAct->setStatusTip(tr("Show the application's About box"));
    connect(instructionAct, SIGNAL(triggered()), this, SLOT(readOrInstruction()));
	return ;
}

void TxtReader::createLanguageMenu()
{
	languageMenu = new QMenu(this);
	languageMenu->setIcon(QIcon(":/images/lang.png"));

    languageActionGroup = new QActionGroup(this);
    connect(languageActionGroup, SIGNAL(triggered(QAction *)),
            this, SLOT(switchLanguage(QAction *)));

    QDir qmDir(":/i18n/");
    QStringList fileNames =
            qmDir.entryList(QStringList("txtreader_*.qm")); 

    for (int i = 0; i < fileNames.size(); ++i) {
    	QString locale = fileNames[i];
        locale.remove(0, locale.indexOf('_') + 1);
        locale.chop(3);
        
        QAction *action = new QAction(tr("&%1 %2")
                                      .arg(i + 1).arg(locale), this);
        action->setCheckable(true);
        action->setData(locale);

        languageMenu->addAction(action);
        languageActionGroup->addAction(action);

        if (locale == uilang)
            action->setChecked(true);
    }
}

void TxtReader::createTrayIcon()
{
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(openAct);
	trayIconMenu->addAction(jumpAct);

	bookmarkMenu = trayIconMenu->addMenu(QIcon(":/images/bookmark.png"),"");
	bookmarkMenu->addAction(addBookmarkAct);
	bookmarkMenu->addAction(delBookmarkAct);
	bookmarkMenu->addSeparator();
	QMapIterator<int,QAction *> i(bookmarkArray);
	while (i.hasNext()){
		bookmarkMenu->addAction(i.next().value());
	}

	setfontMenu = trayIconMenu->addMenu(QIcon(":/images/font.png"),"");
	setfontMenu->addAction(addTxtSizeAct);
	setfontMenu->addAction(subTxtSizeAct);
	setfontMenu->addAction(setfontAct);
	setfontMenu->addAction(setfontcolorAct);
	
	bgMenu = trayIconMenu->addMenu(QIcon(":/images/bg.png"),"");
	bgMenu->addAction(setbgcolorAct);
	bgMenu->addAction(setbgimageAct);
	bgMenu->addAction(delbgimageAct);

	codecMenu = trayIconMenu->addMenu(QIcon(":/images/codec.png"),"");
	codecMenu->addAction(GB2312Act);
	codecMenu->addAction(UTF8Act);

	separatorAct = trayIconMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i)
		trayIconMenu->addAction(recentFileActs[i]);

    trayIconMenu->addSeparator();
    //trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(viewfullscreenAction);
    trayIconMenu->addAction(nofullscreenAction);
    trayIconMenu->addAction(hiddenAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addMenu(languageMenu);
    trayIconMenu->addAction(instructionAct);
    trayIconMenu->addAction(aboutAct);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    updateRecentFileActions();

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
	return ;
}

void TxtReader::setCurrentFile(const QString &fileName)
{
    curFile = fileName;

    setWindowTitle(tr("%1 - Txt Reader %2").arg(strippedName(curFile))
                                    .arg(version));

    QSettings settings("lsyer", "txtreader");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    updateRecentFileActions();
	return ;
}

void TxtReader::updateRecentFileActions()
{
	QSettings settings("lsyer", "txtreader");
	QStringList files = settings.value("recentFileList").toStringList();
	
	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
	
	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		recentFileActs[j]->setVisible(false);

	separatorAct->setVisible(numRecentFiles > 0);
	return ;
}

QString TxtReader::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void TxtReader::open()
{
	QString fileName = QFileDialog::getOpenFileName(this,tr("Open new file"),QFileInfo(curFile).dir().path(),tr("text files (*.txt)"));
	loadFile(fileName);
}
void TxtReader::about()
{
	//this->show();
	QMessageBox::about(this, QString("TxtReader %1").arg(version),
		QString(tr("<p align=center>Txt Reader %1 </p><p align=right> Design by <a href='http://lishao378.blog.sohu.com' target=_blank>lsyer</a></p><p align=center>CopyLeft(C)2006-2008</p>")).arg(version));
	return ;
}
void TxtReader::setcodecstr(QAction *action)
{
	//QAction *action = qobject_cast<QAction *>(sender());
	//if (action){
	codecstr = action->data().toString();
	if (codecstr == "UTF-8"){
		UTF8Act->setChecked(true);
		GB2312Act->setChecked(false);
	} else {
		UTF8Act->setChecked(false);
		GB2312Act->setChecked(true);
	}
	loadFile(curFile);
	setindex(index);
	//}
	return ;
}
void TxtReader::jumptoindex()
{
     bool ok;
     int i = QInputDialog::getInteger(this, QString("TxtReader %1").arg(version),
                                          tr("Jump to:"), index,1,doc.pageCount(),1, &ok);
     if (ok)
         setindex(i);

	return ;
}
void TxtReader::addBookmark()
{
	bool ok;
	QString text = QInputDialog::getText(this, QString("TxtReader %1").arg(version),
                                          tr("Please input the mark:"), QLineEdit::Normal,
                                          tr("%1").arg(index), &ok);
	if (ok && !text.isEmpty()){
		QAction * bookmark = new QAction(tr("%2 - (page %1)").arg(index).arg(text), this);
		bookmark->setData(index);
		connect(bookmark, SIGNAL(triggered()), this, SLOT(openBookmark()));
		bookmarkArray[index] = bookmark;
		bookmarkMenu->addAction(bookmark);
	}
	return ;
}
void TxtReader::delBookmark()
{
	if (bookmarkArray.contains(index)){
		delete bookmarkArray.value(index);
		bookmarkArray.remove(index);
	}
	return ;
}
void TxtReader::openBookmark()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action){
		setindex(action->data().toInt());
	}
	return ;
}
void TxtReader::addTxtSize()
{
	if(txtFont.pointSize() < 25){
		txtFont.setPointSize(txtFont.pointSize()+1);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		doc.setDefaultFont(txtFont);
		QApplication::restoreOverrideCursor();
		setindex(index);
	}
	return;
}
void TxtReader::subTxtSize()
{
	if(txtFont.pointSize() > 5){
		txtFont.setPointSize(txtFont.pointSize()-1);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		doc.setDefaultFont(txtFont);
		QApplication::restoreOverrideCursor();
		setindex(index);
	}
	return;
}
void TxtReader::setfont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, txtFont, this,QString(tr("Set Font - TxtReader %1")).arg(version));
	if (ok && font!=txtFont) {
		txtFont = font;
		QApplication::setOverrideCursor(Qt::WaitCursor);
		doc.setDefaultFont(txtFont);
		QApplication::restoreOverrideCursor();
		setindex(index);
	}
	return;
}
void TxtReader::setfontcolor()
{
	QColor Color = QColorDialog::getColor(txtColor, this);
	if (Color.isValid()) {
		txtColor=Color;
		viewer->ctx.palette.setColor(QPalette::Text, txtColor);
		setindex(index);
	}
	return ;
}
void TxtReader::setbgcolor()
{
	QColor Color = QColorDialog::getColor(bgColor, this);
	if (Color.isValid()) {
		bgColor=Color;
		setStyleSheet( "TxtReader {background-color: "+bgColor.name()+";background-image: url("+bgImage+");background-position:center;;}");
	}
	return ;
}
void TxtReader::setbgimage()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select bg-image"),QFileInfo(bgImage).dir().path(),tr("picture files (*.png *.jpg *.jpeg *.gif)"));
    if (!fileName.isEmpty()) {
    	bgImage=fileName;
		setStyleSheet( "TxtReader {background-color: "+bgColor.name()+";background-image: url("+bgImage+");background-position:center;;}");  	
    }
	return ;
}
void TxtReader::delbgimage()
{
    bgImage = "";
    bgColor=QColor(Qt::white);
	setStyleSheet( "TxtReader {background-color: "+bgColor.name()+";background-image: url("+bgImage+");background-position:center;}");  	
}
void TxtReader::hideAndRestore()
{
   	if(isHidden()){
		myrestore();
  	}else{
   		myhide();
  	}
}
void TxtReader::myhide()
{
	if(isHidden())
		return;

	o_isfullscreen = isFullScreen();
	hiddenAction->setEnabled(false);
	restoreAction->setEnabled(true);
   	hide();
}
void TxtReader::myrestore()
{
	if(o_isfullscreen){
		showFullScreen();
	}else{
		show();
	}

	hiddenAction->setEnabled(true);
	restoreAction->setEnabled(false);
}
void TxtReader::myshowfullscreen()
{
    QSettings settings("lsyer", "txtreader");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
	
	o_isfullscreen = true;
	viewfullscreenAction->setEnabled(false);
	nofullscreenAction->setEnabled(true);
	//showFullScreen();
	myrestore();
}
void TxtReader::myexitfullscreen()
{
    QSettings settings("lsyer", "txtreader");
	QPoint pos = settings.value("pos", QPoint(100, 60)).toPoint();
	QSize size = settings.value("size", QSize(800, 600)).toSize();
	
	o_isfullscreen = false;
	viewfullscreenAction->setEnabled(true);
	nofullscreenAction->setEnabled(false);
	
	//showMaximized();
	resize(size);
	move(pos);
	myrestore();
}

void TxtReader::switchLanguage(QAction *action)
{
    QString locale = action->data().toString();
	uilang = locale;
    appTranslator.load(":/i18n/txtreader_" + locale);
    retranslateUi();
}

void TxtReader::retranslateUi()
{
	pagelabel.setText(QString(tr("Page %1 /Total %2")).arg(index).arg(doc.pageCount()));
    //newAction->setText(tr("&New"));//
    openAct->setText(tr("&Open"));// = new QAction(QIcon(":/images/open.png"),tr("(&O)打  开"), this);
    jumpAct->setText(tr("&Jump"));// = new QAction(QIcon(":/images/jump.png"),tr("(&J)跳  转"), this);
    addBookmarkAct->setText(tr("&Add Bookmark"));// = new QAction(QIcon(":/images/addbookmark.png"),tr("(&A)添加书签"), this);
    delBookmarkAct->setText(tr("&Del Bookmark"));// = new QAction(QIcon(":/images/delbookmark.png"),tr("(&D)删除书签"), this);
    addTxtSizeAct->setText(tr("&Zoom In"));// = new QAction(QIcon(":/images/zoom-in.png"),tr("(&F)放  大"), this);
    subTxtSizeAct->setText(tr("&Zoom Out"));// = new QAction(QIcon(":/images/zoom-out.png"),tr("(&S)缩  小"), this);
    setfontAct->setText(tr("&Set Font"));// = new QAction(QIcon(":/images/setfont.png"),tr("(&A)设  置"), this);
    setfontcolorAct->setText(tr("&Color"));// = new QAction(QIcon(":/images/bgcolor.png"),tr("(&C)颜  色"), this);
    setbgcolorAct->setText(tr("&Bg color"));// = new QAction(QIcon(":/images/bgcolor.png"),tr("(&C)颜  色"), this);
    setbgimageAct->setText(tr("&Bg image"));// = new QAction(QIcon(":/images/bgimage.png"),tr("(&P)图  片"), this);    
    delbgimageAct->setText(tr("&Reset"));// = new QAction(QIcon(":/images/resetbg.png"),tr("(&D)重  设"), this);    
    viewfullscreenAction->setText(tr("&Full Screen"));// = new QAction(QIcon(":/images/fullscreen.png"),tr("(&F)全  屏"), this);    
    nofullscreenAction->setText(tr("&Exit Fullscreen"));// = new QAction(QIcon(":/images/fullscreen.png"),tr("(&N)退出全屏"), this);    
    hiddenAction->setText(tr("&Hide"));// = new QAction(QIcon(":/images/min.png"),tr("(&H)隐  藏"), this);
    restoreAction->setText(tr("&Restore"));// = new QAction(QIcon(":/images/restore.png"),tr("(&R)恢  复"), this);
    quitAction->setText(tr("&Quit"));// = new QAction(QIcon(":/images/exit.png"),tr("(&Q)退  出"), this);
    aboutAct->setText(tr("&About"));// = new QAction(QIcon(":/images/about.png"),tr("(&A)关  于"), this);
    instructionAct->setText(tr("&Instruction"));// = new QAction(QIcon(":/images/instruction.png"),tr("(&I)程序说明"), this);
    
    languageMenu->setTitle(tr("&Language"));
	bookmarkMenu->setTitle(tr("&Bookmark"));// = trayIconMenu->addMenu(QIcon(":/images/bookmark.png"),tr("(&B)书  签"));
	setfontMenu->setTitle(tr("&Font"));// = trayIconMenu->addMenu(QIcon(":/images/font.png"),tr("(&F)字  体"));
	bgMenu->setTitle(tr("&Background"));// = trayIconMenu->addMenu(QIcon(":/images/bg.png"),tr("(&G)背  景"));
	codecMenu->setTitle(tr("&Codec"));// = trayIconMenu->addMenu(QIcon(":/images/codec.png"),tr("(&C)编  码"));
}

void TxtReader::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
   // case QSystemTrayIcon::DoubleClick:
   		hideAndRestore();
        break;
    //case QSystemTrayIcon::MiddleClick:
        //showMessage();
        //break;
    default:
        break;
    }
	return ;
}

void TxtReader::quitaction()
{
    QSettings settings("lsyer", "txtreader");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
	settings.setValue("curFile", curFile);
	settings.setValue("index", index);
	settings.setValue("o_isfullscreen", o_isfullscreen);
	settings.setValue("txtFont", txtFont);
	settings.setValue("txtColor", txtColor);
	settings.setValue("bgColor", bgColor);
	settings.setValue("bgImage", bgImage);
	settings.setValue("codecstr", codecstr);
	settings.setValue("uilang", uilang);
	int j = bookmarkArray.size();
	settings.setValue("bookmarknum",j);
	if(j>0){
		QMapIterator<int,QAction *> i(bookmarkArray);
		int k = 0;
		while (i.hasNext()){
			i.next();
			++k;
			settings.setValue(tr("bookmark%1index").arg(k),i.key());
			settings.setValue(tr("bookmark%1words").arg(k),i.value()->data().toString());
		}
	}
	qApp->quit();
	return ;
}

void TxtReader::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action){
		if(action->data().toString()!=curFile && loadFile(action->data().toString())) setindex(1);
	}
	return ;
}

void TxtReader::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Down){
		setindex(index + 1);
	} else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up){
		setindex(index - 1);
	} else if (event->key() == Qt::Key_Escape) {
		myhide();
	} else {
		QWidget::keyPressEvent(event);
	}
	return ;
}
void TxtReader::mousePressEvent(QMouseEvent *event)
{
	if(this->x()+3*this->width()/4< event->globalX() && event->globalX() < this->x()+this->width()){
		setindex(index + 1);
	}else if(this->x()< event->globalX() && event->globalX() < this->x()+this->width()/4){
		setindex(index - 1);
	}else if (event->button() == Qt::LeftButton) {
		dragPosition = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
	return ;
}
void TxtReader::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton) {
		move(event->globalPos() - dragPosition);
		event->accept();
	}
	return ;
}
void TxtReader::contextMenuEvent(QContextMenuEvent *event)
{
    //QMenu menu(this);
   // menu.addAction(cutAct);
    //menu.addAction(copyAct);
    //menu.addAction(pasteAct);
    trayIconMenu->exec(event->globalPos());
	return ;
}

void TxtReader::closeEvent ( QCloseEvent * event )
{
	hideAndRestore();
	event->ignore();
}

void TxtReader::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void TxtReader::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;
	QString fileName = urls.first().toLocalFile();
	if (fileName.isEmpty())
		return;
	loadFile(fileName);
}

void TxtReader::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

