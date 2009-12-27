#ifndef TXTREADER_H
#define TXTREADER_H
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QTextDocument>
#include <QGridLayout>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QMenu>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QInputDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QStyleOption>
#include <QTranslator>
#include <QUrl>
#include "readerview.h"
class TxtReader : public QWidget
{
	Q_OBJECT
public:
	TxtReader(QWidget * parent = 0,QString infile = "");
protected:
	void keyPressEvent(QKeyEvent * event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void closeEvent ( QCloseEvent * /*event*/ );
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
//for context menu
	void contextMenuEvent(QContextMenuEvent *event);
	void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
private:
	QString version,bgImage,uilang;
	QFont txtFont;
	QColor txtColor,bgColor;
	int index;
	bool o_isfullscreen;
	QLabel pagelabel;
	QTextDocument doc;
	readerView *viewer;
	QGridLayout *layout;
	QPoint dragPosition;
	void setindex(int page);
	int loadFile(const QString &fileName);
	QIcon icon;
	QAction *openAct;
	QAction *aboutAct;
	QAction *instructionAct;
	QString codecstr;
	QMenu *codecMenu;
	QAction *GB2312Act;
	QAction *UTF8Act;
	QActionGroup *codecActionGroup;
	QAction *jumpAct;
	QMenu *bookmarkMenu;
	QMap<int,QAction *> bookmarkArray;
	QAction *addBookmarkAct;
	QAction *delBookmarkAct;
	QMenu *setfontMenu;
	QAction *addTxtSizeAct;
	QAction *subTxtSizeAct;
	QAction *setfontAct;
	QAction *setfontcolorAct;
	QMenu * bgMenu;
	QAction * setbgcolorAct;
	QAction * setbgimageAct;
	QAction * delbgimageAct;
	QMenu * languageMenu;
	QActionGroup *languageActionGroup;
	void createLanguageMenu();
	QTranslator appTranslator;
//for trayIcon
	void createActions();
	void createTrayIcon();
	//QAction *maximizeAction;
	QAction *viewfullscreenAction;
	QAction *nofullscreenAction;
	QAction *hiddenAction;
	QAction *restoreAction;
	QAction *quitAction;

	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;

// for recentfiles menu
	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();
	QString strippedName(const QString &fullFileName);
	QString curFile;
	QMenu *recentFilesMenu;
	enum { MaxRecentFiles = 5 };
	QAction *recentFileActs[MaxRecentFiles];
	QAction *separatorAct;
	void showinstruction();
private slots:
	void open();
	void about();
	void readOrInstruction();
	void setcodecstr(QAction *action);
	void jumptoindex();
	void addBookmark();
	void delBookmark();
	void openBookmark();
	void addTxtSize();
	void subTxtSize();
	void setfont();
	void setfontcolor();
	void setbgcolor();
	void setbgimage();
	void delbgimage();
	void hideAndRestore();
	void myhide();
	void myrestore();
	void myshowfullscreen();
	void myexitfullscreen();
	void switchLanguage(QAction *action);
	void retranslateUi();
//for trayIcon
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void quitaction();
//for recentfiles
	void openRecentFile();
	void changepagecount();
};
#endif 
