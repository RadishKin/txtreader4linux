#include "readerview.h"


readerView::readerView(QTextDocument *document, QWidget *parent)
    : QWidget(parent)
{
    this->setBackgroundRole(QPalette::Dark);

    doc = document;
    scale = 1.0;
    interPageSpacing = 1;
    page=0;
}

void readerView::viewpage(int whichpage)
{
	page = whichpage-1;
    this->update();
}

void readerView::resizeEvent(QResizeEvent *)
{
    viewRect=QRectF(QPointF(0, 0), 
    					QPointF(rect().width()-2*interPageSpacing, 
    							rect().height()-2*interPageSpacing));
    doc->setPageSize(viewRect.size());
    emit pagecountchanged();
}

void readerView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    

    p.translate(interPageSpacing, interPageSpacing);

    //const int pages = doc->pageCount();
    //for (int i = 0; i < pages; ++i) {
        p.save();
        p.scale(scale, scale);

        paintPage(&p, page);

        p.restore();
        p.translate(0, interPageSpacing + viewRect.height() * scale);
    //}
}

void readerView::paintPage(QPainter *painter, int page)
{
    QColor col(Qt::black);

    painter->setPen(col);
    //painter->setBrush(Qt::white);
    painter->drawRect(viewRect);
    painter->setBrush(Qt::NoBrush);
/*
    col = col.light();
    //painter->drawLine(QLineF(viewRect.width(), 1,
                             //viewRect.width(), viewRect.height() - 1));

    col = col.light();
    //painter->drawLine(QLineF(viewRect.width(), 2,
                             //viewRect.width(), viewRect.height() - 2));
*/
    QRectF docRect(QPointF(0, page * viewRect.height()), viewRect.size());
    //QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.clip = docRect;

    // don't use the system palette text as default text color, on HP/UX
    // for example that's white, and white text on white paper doesn't
    // look that nice
    //ctx.palette.setColor(QPalette::Text, Qt::black);

    painter->translate(0, - page * viewRect.height());

    painter->translate(viewRect.topLeft());
    painter->setClipRect(docRect);//.translated(printPreview->pageTopLeft));
    doc->documentLayout()->draw(painter, ctx);
}
