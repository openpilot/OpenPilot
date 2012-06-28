#ifndef MIXERCURVE_H
#define MIXERCURVE_H

#include <QFrame>

namespace Ui {
class MixerCurve;
}

class MixerCurve : public QFrame
{
    Q_OBJECT
    
public:
    explicit MixerCurve(QWidget *parent = 0);
    ~MixerCurve();
    
private:
    Ui::MixerCurve *ui;
};

#endif // MIXERCURVE_H
