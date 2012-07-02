#ifndef MIXERCURVE_H
#define MIXERCURVE_H

#include <QFrame>
#include <QtGui/QWidget>
#include <QList>
#include <QTableWidget>

#include "ui_mixercurve.h"
#include "mixercurvewidget.h"
#include "uavobjectwidgetutils_global.h"

namespace Ui {
class MixerCurve;
}

class UAVOBJECTWIDGETUTILS_EXPORT MixerCurve : public QFrame
{
    Q_OBJECT
    
public:
    explicit MixerCurve(QWidget *parent = 0);
    ~MixerCurve();


    /* Enumeration options for ThrottleCurves */
    typedef enum { MIXERCURVE_THROTTLE=0, MIXERCURVEPITCH=1 } MixerCurveType;

    void setMixerType(MixerCurveType curveType);
    void initCurve (const QList<double>* points);
    QList<double> getCurve();
    void initLinearCurve(int numPoints, double maxValue = 1, double minValue = 0);
    void setCurve(const QList<double>* points);
    void setMin(double value);
    double getMin();
    void setMax(double value);
    double getMax();
    double getCurveMin();
    double getCurveMax();
    double getCurveStep();
    double setRange(double min, double max);


signals:

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

public slots:
    void ResetCurve();
    void GenerateCurve();
    void UpdateSettings();

private slots:
    void CurveMinChanged(double value);
    void CurveMaxChanged(double value);
    void UpdateCurveSettings();

private:
    Ui::MixerCurve* m_mixerUI;
    MixerCurveWidget* m_curve;
    QTableWidget* m_settings;
    MixerCurveType m_curveType;

};

#endif // MIXERCURVE_H
