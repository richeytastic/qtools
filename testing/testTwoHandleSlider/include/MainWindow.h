#include <QMainWindow>
#include "TwoHandleSlider.h"

class MainWindow : public QMainWindow
{ Q_OBJECT
public:
    MainWindow();

private slots:
    void doOnValuesChanged();

private:
    QTools::TwoHandleSlider* _slider;
};  // end class
