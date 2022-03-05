/**
 * @author: delta1037
 * @mail:geniusrabbit@qq.com
 * @brief:
 */
#ifndef NOTE_CLIENT_SVG_COLOR_H
#define NOTE_CLIENT_SVG_COLOR_H

#include <QGraphicsSvgItem>
#include <QDomDocument>
#include <QList>
#include <QPainter>
#include <QIcon>

class SvgColor : public QGraphicsSvgItem
{
public:
    SvgColor(QString filePath, QGraphicsItem *parent = nullptr);
    virtual ~SvgColor();

    QIcon setColor(QColor c);

private:
    QDomDocument _svgXML;

    void change_attr(QString attName, QString attValue);
    void recur_change_attr(QDomNode node, QString attName, QString attValue);
};

#endif // NOTE_CLIENT_SVG_COLOR_H
