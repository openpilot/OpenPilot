/**
 ******************************************************************************
 *
 * @file       videowidget.cpp
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "videowidget.h"

#include "overlay.h"
#include "pipelineevent.h"

#include <QtCore>
#include <QPainter>
#include <QDebug>
#include <QRect>
#include <QTextDocument>
#include <QApplication>

#include <string>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

class GstOverlayImpl : public Overlay {
public:
    GstOverlayImpl(GstVideoOverlay *gst_overlay) :
        gst_overlay(gst_overlay)
    {}
    void expose()
    {
        if (gst_overlay) {
            gst_video_overlay_expose(gst_overlay);
        }
    }
private:
    GstVideoOverlay *gst_overlay;
};

class BusSyncHandler {
public:
    BusSyncHandler(QWidget *widget, WId wid) :
        widget(widget), wId(wid)
    {}
    bool handleMessage(GstMessage *msg);
private:
    QWidget *widget;
    WId wId;
};

static GstElement *createPipelineFromDesc(const char *, QString &lastError);
// static GstElement *createTestPipeline();

static GstBusSyncReply gst_bus_sync_handler(GstBus *, GstMessage *, BusSyncHandler *);

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent), pipeline(NULL), overlay(NULL)
{
    qDebug() << "VideoWidget::VideoWidget";

    // initialize gstreamer
    gst::init(NULL, NULL);

    // make the widget native so it gets its own native window id that we will pass to gstreamer
    setAttribute(Qt::WA_NativeWindow);
    // setAttribute(Qt::WA_DontCreateNativeAncestors);

    // set black background
    QPalette pal(palette());
    pal.setColor(backgroundRole(), Qt::black);
    setPalette(pal);

    // calling winId() will realize the window if it is not yet realized
    // so wee need to call winId() here and not later from a gstreamer thread...
    WId wid = winId();
    qDebug() << QString("VideoWidget::VideoWidget - video winId : %0").arg((gulong)wid);
    handler = new BusSyncHandler(this, wid);

    // init widget state (see setOverlay() for more information)
    // setOverlay(NULL);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_PaintOnScreen, false);

    // init state
    lastError = "";
}

VideoWidget::~VideoWidget()
{
    dispose();
    if (handler) {
        delete handler;
        handler = NULL;
    }
}

bool VideoWidget::isPlaying()
{
    return pipeline && GST_STATE(pipeline) == GST_STATE_PLAYING;
}

QString VideoWidget::pipelineDesc()
{
    return m_pipelineDesc;
}

void VideoWidget::setPipelineDesc(QString pipelineDesc)
{
    qDebug() << QString("VideoWidget::setPipelineDesc - %0").arg(pipelineDesc);
    this->m_pipelineDesc = pipelineDesc;
}

void VideoWidget::start()
{
    qDebug() << QString("VideoWidget::start - %0").arg(m_pipelineDesc);
    init();
    update();
    if (pipeline) {
        gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
    }
}

void VideoWidget::pause()
{
    init();
    update();
    if (pipeline) {
        if (GST_STATE(pipeline) == GST_STATE_PAUSED) {
            gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
        } else if (GST_STATE(pipeline) == GST_STATE_PLAYING) {
            gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
        }
    }
}

void VideoWidget::stop()
{
    dispose();
    update();
}

void VideoWidget::init()
{
    if (pipeline) {
        // if pipeline is already created, reset some state and return
        qDebug() << QString("VideoWidget::init - reseting pipeline state : %0").arg(m_pipelineDesc);
        lastError = "";
        return;
    }

    // reset state
    lastError = "";

    // create pipeline
    qDebug() << QString("VideoWidget::init - initializing pipeline : %0").arg(m_pipelineDesc);
    pipeline = createPipelineFromDesc(m_pipelineDesc.toStdString().c_str(), lastError);
    if (pipeline) {
        // gst_pipeline_set_auto_flush_bus(GST_PIPELINE(pipeline), false);

        // register bus synchronous handler
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
        gst_bus_set_sync_handler(bus, (GstBusSyncHandler)gst_bus_sync_handler, handler, NULL);
        gst_object_unref(bus);
    }
}

void VideoWidget::dispose()
{
    qDebug() << QString("VideoWidget::dispose - %0").arg(m_pipelineDesc);

    setOverlay(NULL);

    if (pipeline) {
        gst_pipeline_set_auto_flush_bus(GST_PIPELINE(pipeline), true);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = NULL;
    }
    // TODO should be done async?
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    // qDebug() << QString("VideoWidget::paintEvent - %0").arg((long)winId());
    if (overlay) {
        overlay->expose();
    } else {
        QWidget::paintEvent(event);
        paintStatus(event);
    }
}

void VideoWidget::paintStatus(QPaintEvent *event)
{
    Q_UNUSED(event);

    QTextDocument doc;
    doc.setDefaultStyleSheet("* { color:red; }");

    QString html     = "<p align=center><b><font size=+2>" + getStatus() + "</font></b></p>"
                       "<p align=center>" + getStatusMessage() + "</p>";

    QRect widgetRect = QWidget::rect();
    int x      = 0;
    int w      = widgetRect.width();
    int hh     = widgetRect.height() / 4;
    int y      = (widgetRect.height() - hh) / 2;
    int h      = widgetRect.height() - y;
    QRect rect = QRect(x, y, w, h);

    doc.setHtml(html);
    doc.setTextWidth(rect.width());

    QPainter painter(this);
    painter.save();
    painter.translate(rect.topLeft());
    doc.drawContents(&painter, rect.translated(-rect.topLeft()));
    painter.restore();
    // painter.drawRect(rect);

    // QBrush brush( Qt::yellow );
    // painter.setBrush( brush );		// set the yellow brush
    // painter.setPen( Qt::NoPen );		// do not draw outline
    // painter.drawRect(0, 0, width(), height());	// draw filled rectangle
    // painter.end();

    // QFont font = QApplication::font();
    // font.setPixelSize( rect.height() );
    // painter.setFont( font );
}

QString VideoWidget::getStatus()
{
    if (!lastError.isEmpty()) {
        return "*** ERROR ***";
    } else if (!pipeline && m_pipelineDesc.isEmpty()) {
        return "*** NO PIPELINE ***";
    }
    return "";
}

QString VideoWidget::getStatusMessage()
{
    if (!lastError.isEmpty()) {
        return lastError;
    }
    return "";
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    if (overlay) {
        overlay->expose();
    } else {
        QWidget::resizeEvent(event);
    }
}

QPaintEngine *VideoWidget::paintEngine() const
{
    // bypass double buffering, see setOverlay() for explanation
    return overlay ? NULL : QWidget::paintEngine();
}

// TODO find a better way and move away from this file
static Pipeline::State cvt(GstState state)
{
    switch (state) {
    case GST_STATE_VOID_PENDING:
        return Pipeline::VoidPending;

    case GST_STATE_NULL:
        return Pipeline::Null;

    case GST_STATE_READY:
        return Pipeline::Ready;

    case GST_STATE_PAUSED:
        return Pipeline::Paused;

    case GST_STATE_PLAYING:
        return Pipeline::Playing;
    }
    return Pipeline::Null;
}


// TODO find a better way and move away from this file
const char *VideoWidget::name(Pipeline::State state)
{
    switch (state) {
    case Pipeline::VoidPending:
        return "VoidPending";

    case Pipeline::Null:
        return "Null";

    case Pipeline::Ready:
        return "Ready";

    case Pipeline::Paused:
        return "Paused";

    case Pipeline::Playing:
        return "Playing";
    }
    return "<unknown>";
}

static StreamStatusEvent::StreamStatusType cvt(GstStreamStatusType type)
{
    switch (type) {
    case GST_STREAM_STATUS_TYPE_CREATE:
        return StreamStatusEvent::Create;

    case GST_STREAM_STATUS_TYPE_ENTER:
        return StreamStatusEvent::Enter;

    case GST_STREAM_STATUS_TYPE_LEAVE:
        return StreamStatusEvent::Leave;

    case GST_STREAM_STATUS_TYPE_DESTROY:
        return StreamStatusEvent::Destroy;

    case GST_STREAM_STATUS_TYPE_START:
        return StreamStatusEvent::Start;

    case GST_STREAM_STATUS_TYPE_PAUSE:
        return StreamStatusEvent::Pause;

    case GST_STREAM_STATUS_TYPE_STOP:
        return StreamStatusEvent::Stop;
    }
    return StreamStatusEvent::Null;
}

static ProgressEvent::ProgressType cvt(GstProgressType type)
{
    switch (type) {
    case GST_PROGRESS_TYPE_START:
        return ProgressEvent::Start;

    case GST_PROGRESS_TYPE_CONTINUE:
        return ProgressEvent::Continue;

    case GST_PROGRESS_TYPE_COMPLETE:
        return ProgressEvent::Complete;

    case GST_PROGRESS_TYPE_CANCELED:
        return ProgressEvent::Cancelled;

    case GST_PROGRESS_TYPE_ERROR:
        return ProgressEvent::Error;
    }
    return ProgressEvent::Error;
}

bool VideoWidget::event(QEvent *event)
{
    // qDebug().noquote() << QString("VideoWidget::event (%0) : %1").arg((long) QThread::currentThreadId()).arg(event->type());
    if (event->type() == PipelineEvent::PrepareWindowId) {
        PrepareWindowIdEvent *pe = static_cast<PrepareWindowIdEvent *>(event);

        // we take ownership of the overlay object
        setOverlay(pe->getOverlay());

        QString msg = QString("element %0 prepare window id").arg(pe->src);
        qDebug() << "VideoWidget::event -" << msg;
        emit message(msg);

        return true;
    } else if (event->type() == PipelineEvent::StateChange) {
        StateChangedEvent *sce = static_cast<StateChangedEvent *>(event);

        QString msg = QString("%0 changed state from %1 to %2")
                      .arg(sce->src).arg(name(sce->getOldState())).arg(name(sce->getNewState()));
        qDebug() << "VideoWidget::event -" << msg;
        emit stateChanged(sce->getNewState());
        // emit message(msg);

        return true;
    } else if (event->type() == PipelineEvent::StreamStatus) {
        StreamStatusEvent *sse = static_cast<StreamStatusEvent *>(event);

        QString msg = QString("%0 stream status %1 %2").arg(sse->src).arg(sse->getStatusName()).arg(sse->getOwner());
        qDebug() << "VideoWidget::event -" << msg;
        // emit message(msg);

        return true;
    } else if (event->type() == PipelineEvent::NewClock) {
        NewClockEvent *nce = static_cast<NewClockEvent *>(event);

        QString msg = QString("%0 has new clock %1").arg(nce->src).arg(nce->getName());
        qDebug() << "VideoWidget::event -" << msg;
        // emit message(msg);

        return true;
    } else if (event->type() == PipelineEvent::ClockProvide) {
        ClockProvideEvent *cpe = static_cast<ClockProvideEvent *>(event);

        QString msg = QString("%0 clock provide %1 ready=%2").arg(cpe->src).arg(cpe->getName()).arg(cpe->isReady());
        qDebug() << "VideoWidget::event -" << msg;
        // emit message(msg);

        return true;
    } else if (event->type() == PipelineEvent::ClockLost) {
        ClockLostEvent *cle = static_cast<ClockLostEvent *>(event);

        QString msg = QString("%0 lost clock %1").arg(cle->src).arg(cle->getName());
        qDebug() << "VideoWidget::event -" << msg;
        // emit message(msg);

        return true;
    } else if (event->type() == PipelineEvent::NewClock) {
        NewClockEvent *nce = static_cast<NewClockEvent *>(event);

        QString msg = QString("%0 changed has new clock %1").arg(nce->src).arg(nce->getName());
        qDebug() << "VideoWidget::event -" << msg;
        // emit message(msg);

        return true;
    } else if (event->type() == PipelineEvent::Progress) {
        ProgressEvent *pe = static_cast<ProgressEvent *>(event);
        QString msg = QString("element %0 sent progress event: %1 %2 (%3)").arg(pe->src).arg(pe->getProgressType()).arg(
            pe->getCode()).arg(pe->getText());
        qDebug() << "VideoWidget::event -" << msg;
        emit message(msg);
        return true;
    } else if (event->type() == PipelineEvent::Qos) {
        QosEvent *qe = static_cast<QosEvent *>(event);
        QString msg  = QString("element %0 sent qos event: %1 %2 %3").arg(qe->src).arg(qe->getData().timestamps()).arg(
            qe->getData().values()).arg(qe->getData().stats());
        // qWarning() << "VideoWidget::event -" << msg;
        // emit message(msg);
        return true;
    } else if (event->type() == PipelineEvent::Error) {
        ErrorEvent *ee = static_cast<ErrorEvent *>(event);
        QString msg    = QString("element %0 sent error event: %1 (%2)").arg(ee->src).arg(ee->getMessage()).arg(
            ee->getDebug());
        qCritical() << "VideoWidget::event -" << msg;
        emit message(msg);
        if (lastError.isEmpty()) {
            // remember first error only (usually the most useful)
            lastError = QString("Pipeline error: %0").arg(ee->getMessage());
            // stop pipeline...
            stop();
        } else {
            // TODO record subsequent errors separately
        }
        return true;
    } else if (event->type() == PipelineEvent::Warning) {
        WarningEvent *we = static_cast<WarningEvent *>(event);
        QString msg = QString("element %0 sent warning event: %1 (%2)").arg(we->src).arg(we->getMessage()).arg(
            we->getDebug());
        qWarning() << "VideoWidget::event -" << msg;
        emit message(msg);
        return true;
    } else if (event->type() == PipelineEvent::Info) {
        InfoEvent *ie = static_cast<InfoEvent *>(event);
        QString msg   = QString("element %0 sent info event: %1 (%2)").arg(ie->src).arg(ie->getMessage()).arg(
            ie->getDebug());
        qDebug() << "VideoWidget::event -" << msg;
        emit message(msg);
        return true;
    }
    return QWidget::event(event);
}

void VideoWidget::setOverlay(Overlay *overlay)
{
    if (this->overlay != overlay) {
        Overlay *oldOverlay = this->overlay;
        this->overlay = overlay;
        if (oldOverlay) {
            delete oldOverlay;
        }
    }

    bool hasOverlay = overlay ? true : false;

    setAutoFillBackground(!hasOverlay);

    // disable background painting to avoid flickering when resizing
    setAttribute(Qt::WA_OpaquePaintEvent, hasOverlay);
    // setAttribute(Qt::WA_NoSystemBackground, hasOverlay); // not sure it is needed

    // disable double buffering to avoid flickering when resizing
    // for this to work we also need to override paintEngine() and make it return NULL.
    // see http://qt-project.org/faq/answer/how_does_qtwa_paintonscreen_relate_to_the_backing_store_widget_composition_
    // drawback is that this widget won't participate in composition...
    setAttribute(Qt::WA_PaintOnScreen, hasOverlay);
}

// static GstElement * createTestPipeline() {
// GstElement *pipeline = gst_pipeline_new("pipeline");
// g_assert(pipeline);
//
// GstElement *src = gst_element_factory_make("videotestsrc", "src");
// g_assert(src);
//
// GstElement *sink = gst_element_factory_make("directdrawsink", "sink");
// g_assert(sink);
//
// gst_bin_add_many(GST_BIN(pipeline), src, sink, NULL);
// gst_element_link_many(src, sink, NULL);
//
// return pipeline;
// }

static GstElement *createPipelineFromDesc(const char *desc, QString &lastError)
{
    qDebug() << QString("VideoWidget::createPipelineFromDesc - creating pipeline : %0").arg(desc);
    GError *error = NULL;
    GstElement *pipeline = gst_parse_launch_full(desc, NULL, GST_PARSE_FLAG_FATAL_ERRORS, &error);
    if (!pipeline) {
        if (error != NULL) {
            // no pipeline and error...
            // report error to user
            QString msg = QString("Failed to create pipeline: %0").arg(error->message);
            qCritical() << "VideoWidget::createPipelineFromDesc -" << msg;
            lastError = msg;
        } else {
            // no pipeline and no error...
            // report generic error
            QString msg = QString("Failed to create pipeline (no error reported!)");
            qCritical() << "VideoWidget::createPipelineFromDesc -" << msg;
            lastError = msg;
        }
    } else if (error) {
        // pipeline and error...
        // report error to user?
        // warning?
        QString msg = QString("Created pipeline with error: %0").arg(error->message);
        qWarning() << "VideoWidget::createPipelineFromDesc -" << msg;
    }
    if (error) {
        g_error_free(error);
    }
    return pipeline;
}

bool BusSyncHandler::handleMessage(GstMessage *message)
{
    // this method is called by gstreamer as a callback
    // and as such is not necessarily called on the QT event handling thread
    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_ELEMENT:
        if (gst_is_video_overlay_prepare_window_handle_message(message)) {
            qDebug() << QString("VideoWidget::handleMessage - element %0 prepare window with id #%1").arg(GST_OBJECT_NAME(message->src)).arg((gulong)wId);
            // prepare-xwindow-id must be handled synchronously in order to have gstreamer use our window
            GstVideoOverlay *gst_video_overlay = GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message));
            gst_video_overlay_set_window_handle(gst_video_overlay, (gulong)wId);
            // and now post event asynchronously
            Overlay *overlay = new GstOverlayImpl(gst_video_overlay);
            QString src(GST_OBJECT_NAME(message->src));
            QCoreApplication::postEvent(widget, new PrepareWindowIdEvent(src, overlay));
        }
        break;
    case GST_MESSAGE_STATE_CHANGED:
    {
        if (GST_IS_PIPELINE(message->src)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);

            QString src(GST_OBJECT_NAME(message->src));
            QCoreApplication::postEvent(widget, new StateChangedEvent(src, cvt(old_state), cvt(new_state), cvt(pending_state)));
        }
        break;
    }
    case GST_MESSAGE_STREAM_STATUS:
    {
        GstStreamStatusType type;
        GstElement *owner;
        gst_message_parse_stream_status(message, &type, &owner);

        QString src(GST_OBJECT_NAME(message->src));
        QString name(GST_OBJECT_NAME(owner));
        QCoreApplication::postEvent(widget, new StreamStatusEvent(src, cvt(type), name));
        break;
    }
    case GST_MESSAGE_NEW_CLOCK:
    {
        if (GST_IS_PIPELINE(message->src)) {
            GstClock *clock;

            gst_message_parse_new_clock(message, &clock);

            QString src(GST_OBJECT_NAME(message->src));
            QString name(GST_OBJECT_NAME(clock));
            QCoreApplication::postEvent(widget, new NewClockEvent(src, name));
        }
        break;
    }
    case GST_MESSAGE_CLOCK_PROVIDE:
    {
        if (GST_IS_PIPELINE(message->src)) {
            GstClock *clock;
            gboolean ready;

            gst_message_parse_clock_provide(message, &clock, &ready);

            QString src(GST_OBJECT_NAME(message->src));
            QString name(GST_OBJECT_NAME(clock));
            QCoreApplication::postEvent(widget, new ClockProvideEvent(src, name, ready));
        }
        break;
    }
    case GST_MESSAGE_CLOCK_LOST:
    {
        if (GST_IS_PIPELINE(message->src)) {
            GstClock *clock;

            gst_message_parse_clock_lost(message, &clock);

            QString src(GST_OBJECT_NAME(message->src));
            QString name(GST_OBJECT_NAME(clock));
            QCoreApplication::postEvent(widget, new ClockLostEvent(src, name));
        }
        break;
    }
    case GST_MESSAGE_PROGRESS:
    {
        GstProgressType type;
        gchar *code;
        gchar *text;
        gst_message_parse_progress(message, &type, &code, &text);

        QString src(GST_OBJECT_NAME(message->src));
        QCoreApplication::postEvent(widget, new ProgressEvent(src, cvt(type), QString(code), QString(text)));

        g_free(code);
        g_free(text);

        break;
    }
    case GST_MESSAGE_QOS:
    {
        QosData data;
        gboolean live;
        gst_message_parse_qos(message, &live, &data.running_time, &data.stream_time, &data.timestamp, &data.duration);
        data.live = (live == TRUE) ? true : false;

        gdouble proportion;
        gst_message_parse_qos_values(message, &data.jitter, &proportion, &data.quality);
        data.proportion = proportion;

        gst_message_parse_qos_stats(message, NULL, &data.processed, &data.dropped);

        QString src(GST_OBJECT_NAME(message->src));
        QCoreApplication::postEvent(widget, new QosEvent(src, data));

        break;
    }
    case GST_MESSAGE_EOS:
        /* end-of-stream */
        // g_main_loop_quit (loop);
        break;
    case GST_MESSAGE_ERROR:
    {
        GError *err;
        gchar *debug;
        gst_message_parse_error(message, &err, &debug);

        QString src(GST_OBJECT_NAME(message->src));
        QCoreApplication::postEvent(widget, new ErrorEvent(src, QString(err->message), QString(debug)));

        g_error_free(err);
        g_free(debug);
        break;
    }
    case GST_MESSAGE_WARNING:
    {
        GError *err;
        gchar *debug;
        gst_message_parse_warning(message, &err, &debug);

        QString src(GST_OBJECT_NAME(message->src));
        QCoreApplication::postEvent(widget, new WarningEvent(src, QString(err->message), QString(debug)));

        g_error_free(err);
        g_free(debug);
        break;
    }
    case GST_MESSAGE_INFO:
    {
        GError *err;
        gchar *debug;
        gst_message_parse_info(message, &err, &debug);

        QString src(GST_OBJECT_NAME(message->src));
        QCoreApplication::postEvent(widget, new InfoEvent(src, QString(err->message), QString(debug)));

        g_error_free(err);
        g_free(debug);
        break;
    }
    default:
        break;
    }
    return false;
}

static GstBusSyncReply gst_bus_sync_handler(GstBus *bus, GstMessage *message, BusSyncHandler *handler)
{
    qDebug().noquote() << QString("VideoWidget::gst_bus_sync_handler (%0) : %1 : %2")
        .arg((long)QThread::currentThreadId())
        .arg(GST_MESSAGE_SRC_NAME(message))
        .arg(GST_MESSAGE_TYPE_NAME(message));
    if (handler->handleMessage(message)) {
        gst_message_unref(message);
        return GST_BUS_DROP;
    }
    return GST_BUS_PASS;
}
