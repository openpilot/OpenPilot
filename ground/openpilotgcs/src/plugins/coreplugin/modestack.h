#ifndef MODESTACK_H
#define MODESTACK_H

#include <QDeclarativeView>
#include <coreplugin/imode.h>

namespace Core{

class ModeStackModel;

class ModeStack : public QObject
{
    Q_OBJECT
public:
    explicit ModeStack(QWidget *parent = 0);

    ~ModeStack();

    int currentIndex(){return m_currentIndex;}
    void setCurrentIndex(int index);
    void setItemIcon(int index, QIcon icon);
    void setItemText(int index, QString text);
    void setItemToolTip(int index, QString tooltip);
    void setIconSize(QSize size);

    QWidget* getWidget(){return m_view;}

    void insertItem(int index, Core::IMode* mode);
    void moveItem(int curIndex, int newIndex);
    void removeItem(int index);

signals:
    void currentChanged(int index);

public slots:

private:
    QDeclarativeView* m_view;
    ModeStackModel* m_model;
    QGraphicsObject* m_qmlObject;
    int m_currentIndex;

private slots:
    void indexChanged(int index){m_currentIndex = index;}
};
}
#endif // MODESTACK_H
