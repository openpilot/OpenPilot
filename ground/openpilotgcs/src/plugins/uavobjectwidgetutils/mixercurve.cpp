#include <math.h>
#include "mixercurve.h"

MixerCurve::MixerCurve(QWidget *parent) :
    QFrame(parent),
    m_mixerUI(new Ui::MixerCurve),
    m_curveType(MixerCurve::MIXERCURVE_THROTTLE)
{
    m_mixerUI->setupUi(this);

    m_curve = m_mixerUI->CurveWidget;
    m_settings = m_mixerUI->CurveSettings;

    UpdateCurveSettings();

    connect(m_mixerUI->CurveType, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateCurveSettings()));
    connect(m_mixerUI->ResetCurve, SIGNAL(clicked()), this, SLOT(ResetCurve()));
    connect(m_mixerUI->GenerateCurve, SIGNAL(clicked()), this, SLOT(GenerateCurve()));
    connect(m_curve, SIGNAL(curveUpdated()), this, SLOT(UpdateSettings()));

    connect(m_mixerUI->CurveMin, SIGNAL(valueChanged(double)), this, SLOT(CurveMinChanged(double)));
    connect(m_mixerUI->CurveMax, SIGNAL(valueChanged(double)), this, SLOT(CurveMaxChanged(double)));
    connect(m_mixerUI->CurveStep, SIGNAL(valueChanged(double)), this, SLOT(GenerateCurve()));
}

MixerCurve::~MixerCurve()
{
    delete m_mixerUI;
}

void MixerCurve::ResetCurve()
{
    m_curve->setMin((m_curveType == MixerCurve::MIXERCURVE_THROTTLE) ? 0.0 : -1.0);
    m_curve->setMax(1.0);

    m_mixerUI->CurveMin->setValue(m_curve->getMin());
    m_mixerUI->CurveMax->setValue(m_curve->getMax());
    m_mixerUI->CurveType->setCurrentIndex(m_mixerUI->CurveType->findText("Linear"));

    initLinearCurve(MixerCurveWidget::NODE_NUMELEM, m_curve->getMax(), m_curve->getMin());

    UpdateSettings();
}

void MixerCurve::UpdateCurveSettings()
{
    //get the user settings
    QString curveType = m_mixerUI->CurveType->currentText();

    switch (m_curveType) {
        case MixerCurve::MIXERCURVE_THROTTLE:
        {
            m_mixerUI->CurveMin->setMinimum(0.0);
            m_mixerUI->CurveMax->setMinimum(0.0);
            m_mixerUI->CurveStep->setMinimum(0.0);
            break;
        }
        case MixerCurve::MIXERCURVEPITCH:
        {
            m_mixerUI->CurveMin->setMinimum(-1.0);
            m_mixerUI->CurveMax->setMinimum(-1.0);
            m_mixerUI->CurveStep->setMinimum(0.0);
            break;
        }
    }
    m_mixerUI->CurveMin->setMaximum(1.0);
    m_mixerUI->CurveMax->setMaximum(1.0);
    m_mixerUI->CurveStep->setMaximum(100.0);

    //set default visible
    m_mixerUI->minLabel->setText("Min");
    m_mixerUI->minLabel->setVisible(true);
    m_mixerUI->CurveMin->setVisible(true);
    m_mixerUI->maxLabel->setVisible(false);
    m_mixerUI->CurveMax->setVisible(false);
    m_mixerUI->stepLabel->setVisible(false);
    m_mixerUI->CurveStep->setVisible(false);

    if ( curveType.compare("Flat")==0)
    {
        m_mixerUI->minLabel->setText("Value");
    }
    if ( curveType.compare("Linear")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
    }
    if ( curveType.compare("Step")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
        m_mixerUI->stepLabel->setText("Step at");
        m_mixerUI->stepLabel->setVisible(true);
        m_mixerUI->CurveStep->setVisible(true);
    }
    if ( curveType.compare("Exp")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
        m_mixerUI->stepLabel->setText("Strength");
        m_mixerUI->stepLabel->setVisible(true);
        m_mixerUI->CurveStep->setVisible(true);
    }
    if ( curveType.compare("Log")==0)
    {
        m_mixerUI->maxLabel->setVisible(true);
        m_mixerUI->CurveMax->setVisible(true);
        m_mixerUI->stepLabel->setText("Strength");
        m_mixerUI->stepLabel->setVisible(true);
        m_mixerUI->CurveStep->setVisible(true);
    }

    GenerateCurve();
}

void MixerCurve::GenerateCurve()
{
   double scale;
   double newValue;

   //get the user settings
   double value1 = getCurveMin();
   double value2 = getCurveMax();
   double value3 = getCurveStep();

   QString CurveType = m_mixerUI->CurveType->currentText();

   QList<double> points;

   for (int i=0; i<MixerCurveWidget::NODE_NUMELEM; i++)
   {
       scale =((double)i/(double)(MixerCurveWidget::NODE_NUMELEM - 1));

       if ( CurveType.compare("Flat")==0)
       {
           points.append(value1);
       }
       if ( CurveType.compare("Linear")==0)
       {
           newValue =value1 +(scale*(value2-value1));
           points.append(newValue);
       }
       if ( CurveType.compare("Step")==0)
       {
           if (scale*100<value3)
           {
               points.append(value1);
           }
           else
           {
               points.append(value2);
           }
       }
       if ( CurveType.compare("Exp")==0)
       {
           newValue =value1 +(((exp(scale*(value3/10))-1))/(exp((value3/10))-1)*(value2-value1));
           points.append(newValue);
       }
       if ( CurveType.compare("Log")==0)
       {
           newValue = value1 +(((log(scale*(value3*2)+1))/(log(1+(value3*2))))*(value2-value1));
           points.append(newValue);
       }
   }

   initCurve(&points);
}

void MixerCurve::initCurve (const QList<double>* points)
{
    m_curve->setCurve(points);
    UpdateSettings();
}

QList<double> MixerCurve::getCurve()
{
    return m_curve->getCurve();
}

void MixerCurve::initLinearCurve(int numPoints, double maxValue, double minValue)
{
    m_curve->initLinearCurve(numPoints, maxValue, minValue);
}

void MixerCurve::setCurve(const QList<double>* points)
{
    m_curve->setCurve(points);
    UpdateSettings();
}

void MixerCurve::setMin(double value)
{
    m_curve->setMin(value);
}

double MixerCurve::getMin()
{
    return m_curve->getMin();
}

void MixerCurve::setMax(double value)
{
    m_curve->setMax(value);
}

double MixerCurve::getMax()
{
    return m_curve->getMax();
}

double MixerCurve::getCurveMin()
{
    return m_mixerUI->CurveMin->value();
}
double MixerCurve::getCurveMax()
{
    return m_mixerUI->CurveMax->value();
}

double MixerCurve::getCurveStep()
{
    return m_mixerUI->CurveStep->value();
}

void MixerCurve::UpdateSettings()
{
    QList<double> points = m_curve->getCurve();

    int ptCnt = points.count();
    for (int i=0; i<ptCnt; i++)
    {
        QTableWidgetItem* item = m_settings->item((ptCnt - 1) - i, 0);
        if (item)
            item->setText(QString().sprintf("%.2f",points.at(i)));
    }
}

void MixerCurve::CurveMinChanged(double value)
{
    //setMin(value);

    // the min changed so redraw the curve
    //  mixercurvewidget::setCurve will trim any points below min
    QList<double> points = getCurve();
    points.removeFirst();
    points.push_front(value);
    setCurve(&points);
}

void MixerCurve::CurveMaxChanged(double value)
{
    //setMax(value);

    // the max changed so redraw the curve
    //  mixercurvewidget::setCurve will trim any points above max
    QList<double> points = getCurve();
    points.removeLast();
    points.append(value);
    setCurve(&points);
}

double MixerCurve::setRange(double min, double max)
{
    return m_curve->setRange(min, max);
}

void MixerCurve::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    //fitInView(this, Qt::KeepAspectRatio);

}

void MixerCurve::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    //fitInView(this, Qt::KeepAspectRatio);
}
