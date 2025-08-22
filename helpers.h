// helpers.h - Version 1.0
// Chứa các lớp tiện ích dùng chung cho toàn bộ ứng dụng
#ifndef HELPERS_H
#define HELPERS_H

#include <QObject>
#include <QFrame>
#include <QGroupBox>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QToolTip>
#include <QStyle>
#include <QStyleOptionGroupBox>
#include <QCursor>

// --- Helper class cho ô màu có thể click ---
class ClickableFrame : public QFrame {
    Q_OBJECT
public:
    explicit ClickableFrame(QWidget *parent = nullptr) : QFrame(parent) {
        setCursor(Qt::PointingHandCursor);
    }
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
        QFrame::mousePressEvent(event);
    }
};

// --- Helper class cho tooltip của GroupBox ---
class TitleEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit TitleEventFilter(QObject *parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::Enter) {
            QGroupBox *groupBox = qobject_cast<QGroupBox*>(obj);
            if (groupBox && !groupBox->toolTip().isEmpty()) {
                QTimer::singleShot(1000, [=]() {
                    if (groupBox && groupBox->underMouse()) {
                        QStyleOptionGroupBox opt;
                        opt.initFrom(groupBox); 
                        QRect titleRect = groupBox->style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, groupBox);
                        if (titleRect.contains(groupBox->mapFromGlobal(QCursor::pos()))) {
                           QToolTip::showText(QCursor::pos(), groupBox->toolTip(), groupBox);
                        }
                    }
                });
            }
        } else if (event->type() == QEvent::Leave) {
            QToolTip::hideText();
        }
        return QObject::eventFilter(obj, event);
    }
};

#endif // HELPERS_H
