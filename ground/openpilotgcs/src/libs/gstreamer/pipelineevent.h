/*
 * PipelineEvent.h
 *
 *  Created on: 8 déc. 2012
 *      Author: filnet
 */

#ifndef PIPELINEEVENT_H_
#define PIPELINEEVENT_H_

#include <QEvent>
#include <QString>

#include "pipeline.h"
#include "overlay.h"

class PipelineEvent : public QEvent {
public:
    // event types
    static const QEvent::Type PrepareWindowId;
    static const QEvent::Type StateChange;
    static const QEvent::Type StreamStatus;
    static const QEvent::Type NewClock;
    static const QEvent::Type ClockProvide;
    static const QEvent::Type ClockLost;
    static const QEvent::Type Progress;
    static const QEvent::Type Qos;
    static const QEvent::Type Error;
    static const QEvent::Type Warning;
    static const QEvent::Type Info;

    PipelineEvent(QEvent::Type type, QString src) :
        QEvent(type), src(src)
    {}
    virtual ~PipelineEvent()
    {}
public:
    QString src;
};

const QEvent::Type PipelineEvent::PrepareWindowId = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::StateChange     = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::StreamStatus    = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::NewClock = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::ClockProvide    = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::ClockLost = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::Progress  = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::Qos = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::Error     = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::Warning   = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type PipelineEvent::Info = static_cast<QEvent::Type>(QEvent::registerEventType());

class PrepareWindowIdEvent : public PipelineEvent {
public:
    PrepareWindowIdEvent(QString src, Overlay *overlay) :
        PipelineEvent(PrepareWindowId, src), overlay(overlay)
    {}
    Overlay *getOverlay()
    {
        return overlay;
    }
    static QEvent::Type type()
    {
        return PrepareWindowId;
    }
private:
    Overlay *overlay;
};

class StateChangedEvent : public PipelineEvent {
public:
    StateChangedEvent(QString src, Pipeline::State oldState, Pipeline::State newState, Pipeline::State pendingState) :
        PipelineEvent(StateChange, src), oldState(oldState), newState(newState), pendingState(pendingState)
    {}
    static QEvent::Type type()
    {
        return StateChange;
    }
    Pipeline::State getOldState()
    {
        return oldState;
    }
    Pipeline::State getNewState()
    {
        return newState;
    }
    Pipeline::State getPendingState()
    {
        return pendingState;
    }
    static const char *stateName(Pipeline::State state)
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
private:
    Pipeline::State oldState;
    Pipeline::State newState;
    Pipeline::State pendingState;
};

class StreamStatusEvent : public PipelineEvent {
public:
    enum StreamStatusType {
        Create, Enter, Leave, Destroy, Start, Pause, Stop, Null
    };
    StreamStatusEvent(QString src, StreamStatusType status, QString owner) :
        PipelineEvent(StreamStatus, src), status(status), owner(owner)
    {}
    static QEvent::Type type()
    {
        return StreamStatus;
    }
    StreamStatusType getStatus()
    {
        return status;
    }
    const char *getStatusName()
    {
        return statusName(status);
    }
    static const char *statusName(StreamStatusType status)
    {
        switch (status) {
        case StreamStatusEvent::Create:
            return "Create";

        case StreamStatusEvent::Enter:
            return "Enter";

        case StreamStatusEvent::Leave:
            return "Leave";

        case StreamStatusEvent::Destroy:
            return "Destroy";

        case StreamStatusEvent::Start:
            return "Start";

        case StreamStatusEvent::Pause:
            return "Pause";

        case StreamStatusEvent::Stop:
            return "Stop";

        case StreamStatusEvent::Null:
            return "Null";
        }
        return "<unknown>";
    }
    QString getOwner()
    {
        return owner;
    }
private:
    StreamStatusType status;
    QString owner;
};

class ClockEvent : public PipelineEvent {
public:
    ClockEvent(QEvent::Type type, QString src, QString name) : PipelineEvent(type, src), name(name)
    {}
    QString getName()
    {
        return name;
    }
private:
    QString name;
};

class NewClockEvent : public ClockEvent {
public:
    NewClockEvent(QString src, QString name) : ClockEvent(NewClock, src, name)
    {}
    static QEvent::Type type()
    {
        return NewClock;
    }
};

class ClockProvideEvent : public ClockEvent {
public:
    ClockProvideEvent(QString src, QString name, bool ready) : ClockEvent(ClockProvide, src, name), ready(ready)
    {}
    static QEvent::Type type()
    {
        return ClockProvide;
    }
    bool isReady()
    {
        return ready;
    }
private:
    bool ready;
};


class ClockLostEvent : public ClockEvent {
public:
    ClockLostEvent(QString src, QString name) : ClockEvent(ClockLost, src, name)
    {}
    static QEvent::Type type()
    {
        return ClockLost;
    }
};

class ProgressEvent : public PipelineEvent {
public:
    enum ProgressType {
        Start, Continue, Complete, Cancelled, Error
    };

    ProgressEvent(QString src, ProgressType progressType, QString code, QString text) :
        PipelineEvent(Progress, src), progressType(progressType), code(code), text(text)
    {}
    static QEvent::Type type()
    {
        return Progress;
    }
    ProgressType getProgressType()
    {
        return progressType;
    }
    QString getCode()
    {
        return code;
    }
    QString getText()
    {
        return text;
    }
private:
    ProgressType progressType;
    QString code;
    QString text;
};

class QosData {
public:
    // timestamps and live status
    // If the message was generated by a live element
    bool live;
    // running_time, stream_time, timestamp and duration of the dropped buffer.
    // Values of GST_CLOCK_TIME_NONE mean unknown values.
    quint64 running_time;
    quint64 stream_time;
    quint64 timestamp;
    quint64 duration;

    // values
    // The difference of the running-time against the deadline.
    qint64 jitter;
    // Long term prediction of the ideal rate relative to normal rate to get optimal quality.
    qreal proportion; // won't work on ARM?
    // An element dependent integer value that specifies the current quality level of the element.
    // The default maximum quality is 1000000.
    qint32 quality;

    // stats
    // QoS stats representing the history of the current continuous pipeline playback period.
    // When format is GST_FORMAT_UNDEFINED both dropped and processed are invalid.
    // Values of -1 for either processed or dropped mean unknown values.

    // Units of the 'processed' and 'dropped' fields.
    // Video sinks and video filters will use GST_FORMAT_BUFFERS (frames).
    // Audio sinks and audio filters will likely use GST_FORMAT_DEFAULT (samples)
    // GstFormat format;
    // Total number of units correctly processed since the last state change to READY or a flushing operation.
    quint64 processed;
    // Total number of units dropped since the last state change to READY or a flushing operation.
    quint64 dropped;

    QString timestamps()
    {
        return QString("live: %0; running time: %1; stream time: %2; timestamp: %3; duration: %4").arg(live).arg(
            running_time).arg(stream_time).arg(timestamp).arg(duration);
    }
    QString values()
    {
        return QString("jitter: %0; proportion: %1; quality: %2;").arg(jitter).arg(proportion).arg(quality);
    }
    QString stats()
    {
        return QString("format: %0; processed: %1; dropped: %2;").arg("").arg(processed).arg(dropped);
    }
};

class QosEvent : public PipelineEvent {
public:
    QosEvent(QString src, QosData data) : PipelineEvent(Qos, src), data(data)
    {}
    static QEvent::Type type()
    {
        return Qos;
    }
    QosData getData()
    {
        return data;
    }
private:
    QosData data;
};

class MessageEvent : public PipelineEvent {
public:
    MessageEvent(QEvent::Type type, QString src, QString message, QString debug) :
        PipelineEvent(type, src), message(message), debug(debug)
    {}
    QString getMessage()
    {
        return message;
    }
    QString getDebug()
    {
        return debug;
    }
private:
    QString message;
    QString debug;
};

class ErrorEvent : public MessageEvent {
public:
    ErrorEvent(QString src, QString message, QString debug) : MessageEvent(Error, src, message, debug)
    {}
    static QEvent::Type type()
    {
        return Error;
    }
};

class WarningEvent : public MessageEvent {
public:
    WarningEvent(QString src, QString message, QString debug) : MessageEvent(Warning, src, message, debug)
    {}
    static QEvent::Type type()
    {
        return Warning;
    }
};

class InfoEvent : public MessageEvent {
public:
    InfoEvent(QString src, QString message, QString debug) : MessageEvent(Info, src, message, debug)
    {}
    static QEvent::Type type()
    {
        return Info;
    }
};

#endif /* PIPELINEEVENT_H_ */
