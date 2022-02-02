#include "FrameWidget.h"

#include <QPainter>
#include <QPaintEvent>

FrameWidget::FrameWidget(QWidget *parent)
    : QWidget(parent), m_isCirclePhoto(false)
{
}

void FrameWidget::setImage(const QPixmap &image)
{
    m_img = image;
    this->update();
}

void FrameWidget::setCirclePhoto(const QPixmap &image)
{
    m_isCirclePhoto = true;
    m_img = image;
    this->update();
}

void FrameWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect rect = event->rect();
    QRect imgRect = rect;
    if (!m_img.isNull()) {
        if(!m_isCirclePhoto){
            float xScale = float(rect.width()) / m_img.width();
            float yScale = float(rect.height()) / m_img.height();

            if (xScale <= yScale) {
                int height = int(xScale * m_img.height());
                int y = rect.y() + (rect.height() - height) / 2;
                imgRect = QRect(rect.x(), y, rect.width(), height);
            } else {
                int width = int(yScale * m_img.width());
                int x = rect.x() + (rect.width() - width) / 2;
                imgRect = QRect(x, rect.y(), width, rect.height());
            }
            painter.drawPixmap(imgRect, m_img);
        }
        else
        {
            QRect cropRect;
            if (m_img.height() >= m_img.width()) {
                int y = m_img.height() / 2 - m_img.width() / 2;
                cropRect = QRect(0, y, m_img.width(), m_img.width());
            } else {
                int x = m_img.width() / 2 - m_img.height() / 2;
                cropRect = QRect(x, 0, m_img.height(), m_img.height());
            }
            QPixmap cropped = m_img.copy(cropRect);
            m_img = cropped.scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QImage result(rect.width(), rect.height(), QImage::Format_RGBA8888);
            result.fill(qRgba(0, 0, 0, 0));
            QPainter painter1(&result);
            painter1.setRenderHint(QPainter::Antialiasing);
            painter1.setRenderHint(QPainter::HighQualityAntialiasing);
            painter1.setBrush(QBrush(QColor(0, 0, 0, 255)));
            painter1.drawEllipse(1, 1, rect.width() - 2, rect.height() - 2);
            painter1.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter1.drawPixmap(rect, m_img);
            painter1.end();
            painter.drawPixmap(rect, QPixmap::fromImage(result));
        }
    }
    else {
        QBrush painterBrush(QColor("#707070"), Qt::SolidPattern);
        painter.fillRect(imgRect, painterBrush);
    }
    QWidget::paintEvent(event);
}
