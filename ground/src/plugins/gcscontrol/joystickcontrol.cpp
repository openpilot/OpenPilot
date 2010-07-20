#include "joystickcontrol.h"
#include "extensionsystem/pluginmanager.h"
#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QMouseEvent>
#include <QtGlobal>

JoystickControl::JoystickControl(QWidget *parent) :
    QGraphicsView(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
    setRenderHints(QPainter::Antialiasing);

    m_renderer = new QSvgRenderer();
    Q_ASSERT( m_renderer->load(QString(":/gcscontrol/images/joystickBackground.svg")) );

    m_background = new QGraphicsSvgItem(QString(":/gcscontrol/images/joystickBackground.svg"));
    m_background->setSharedRenderer(m_renderer);

    QGraphicsScene *l_scene = scene();
    l_scene->clear(); // This also deletes all items contained in the scene.
    l_scene->addItem(m_background);
    l_scene->setSceneRect(m_background->boundingRect());
}

JoystickControl::~JoystickControl()
{

}

ManualControlCommand* JoystickControl::getMCC()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    ManualControlCommand* obj = dynamic_cast<ManualControlCommand*>( objManager->getObject(QString("ManualControlCommand")) );
    return obj;
}

void JoystickControl::mousePressEvent(QMouseEvent *event)
{
    Qt::MouseButton button = event->button();
    QPoint point = event->pos();
    QSize widgetSize = this->size();

    double x = 2 * ( ((double)point.x()) / ((double)widgetSize.width()) - .5 );
    double y = 2 * ( ((double)point.y()) / ((double)widgetSize.height()) - .5);

    if( button == Qt::LeftButton && this->objectName() == QString("widgetLeftStick"))
    {

        ManualControlCommand::DataFields data = getMCC()->getData();
        data.Pitch = x;
        data.Yaw = y;
        getMCC()->setData(data);
    }

    if( button == Qt::LeftButton && this->objectName() == QString("widgetRightStick"))
    {

        ManualControlCommand::DataFields data = getMCC()->getData();
        data.Throttle = x;
        data.Roll = y;
        getMCC()->setData(data);
    }
}

void JoystickControl::paint()
{
    update();
}

void JoystickControl::paintEvent(QPaintEvent *event)
{
    // Skip painting until the dial file is loaded
    if (! m_renderer->isValid()) {
        qDebug()<<"Dial file not loaded, not rendering";
//        return;
    }

    QGraphicsView::paintEvent(event);
}

void JoystickControl::resizeEvent(QResizeEvent *event)
{
    fitInView(m_background, Qt::KeepAspectRatio );
}
