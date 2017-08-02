#ifndef QTOOLS_TWO_HANDLE_SLIDER_H
#define QTOOLS_TWO_HANDLE_SLIDER_H

#include <QWidget>
#include "QTools_Export.h"

namespace QTools
{

class QTools_EXPORT TwoHandleSlider : public QWidget
{ Q_OBJECT
public:
    TwoHandleSlider( QWidget *parent=NULL, bool orientVertically=false);
    virtual ~TwoHandleSlider();

    void resetRange( float min, float max);
    void reset();   // Reset handles to existing min and max
    void resetMinimum();
    void resetMaximum();

    void setPageStep( float pstep);
    void setSingleStep( float sstep);

    float value0() const;
    float value1() const;
    void setValue0( float);
    void setValue1( float);

    float minimum() const;
    float maximum() const;

signals:
    void value0Changed( float);
    void value1Changed( float);

private:
    struct Slider;
    Slider *_s0, *_s1;
    void updateSlidersMidpoint( float, float);

private slots:
    void doOnValue0Changed();
    void doOnValue1Changed();
};  // end class


}   // end namespace

#endif
