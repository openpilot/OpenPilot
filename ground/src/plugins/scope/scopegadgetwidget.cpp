/*
 * scopegadgetwidget.cpp
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */
#include "scopegadgetwidget.h"
#include "qwt/src/qwt_plot_curve.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

ScopeGadgetWidget::ScopeGadgetWidget(QWidget *parent) : QwtPlot(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    // Show a title
    setTitle("Sine");

    // Show a legend at the bottom
//    setAutoLegend( true );
//    setLegendPos( Qwt::Bottom );

    // Show the axes
    setAxisTitle( xBottom, "x" );
    setAxisTitle( yLeft, "y" );

    // Calculate the data, 500 points each
    const int points = 500;
    double x[ points ];
    double sn[ points ];
    double sg[ points ];

    for( int i=0; i<points; i++ )
    {
        x[i] = (3.0*3.14/double(points))*double(i);
        sn[i] = 2.0*sin( x[i] );
        sg[i] = (sn[i]>0)?1:((sn[i]<0)?-1:0);
    }
    // add curves
    QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
    QwtPlotCurve *curve2 = new QwtPlotCurve("Curve 2");

    // copy the data into the curves
    curve1->setData(x, sn, points);
    curve2->setData(x, sg, points);

    curve1->attach(this);
    curve2->attach(this);

    // finally, refresh the plot
    replot();
}

ScopeGadgetWidget::~ScopeGadgetWidget()
{
   // Do nothing
}

