#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include <QWidget>

class FrameWidget : public QWidget
{
     Q_OBJECT
public:
    explicit FrameWidget(QWidget *parent = nullptr);
    void setImage(const QPixmap& image);
    void setCirclePhoto(const QPixmap& image);

protected:
    void paintEvent(QPaintEvent* event) ;

private:
     QPixmap m_img;
     bool m_isCirclePhoto;
};
#endif // FRAMEWIDGET_H
