#include "ViewNavigator.h"
using QTools::ViewNavigator;
#include "ui_ViewNavigator.h"

static const bool USE_POINTS = true;


ViewNavigator::ViewNavigator(QWidget *parent) : QWidget(parent), ui(new Ui::ViewNavigator)
{
    ui->setupUi(this);

    // View navigation
    connect( ui->frontRadioButton, SIGNAL( clicked()), this, SLOT( userSelectViewDirectionSlot()));
    connect( ui->leftRadioButton, SIGNAL( clicked()), this, SLOT( userSelectViewDirectionSlot()));
    connect( ui->rearRadioButton, SIGNAL( clicked()), this, SLOT( userSelectViewDirectionSlot()));
    connect( ui->rightRadioButton, SIGNAL( clicked()), this, SLOT( userSelectViewDirectionSlot()));

    // View type (image or points)
    connect( ui->viewImageRadioButton, SIGNAL( clicked()), this, SIGNAL( selectedImage()));
    connect( ui->viewDepthRadioButton, SIGNAL( clicked()), this, SIGNAL( selectedDepth()));

    ui->viewPointsRadioButton->setEnabled(USE_POINTS);
    ui->viewPointsRadioButton->setVisible(USE_POINTS);
    if ( USE_POINTS)
        connect( ui->viewPointsRadioButton, SIGNAL( clicked()), this, SIGNAL( selectedPoints()));
}   // end ctor


ViewNavigator::~ViewNavigator()
{
    delete ui;
}   // end dtor


// static
string ViewNavigator::viewFaceToString(const ViewFace &face)
{
    string fs = "-1";
    switch (face)
    {
    case ViewNavigator::FRONT:
        fs = "0"; break;
    case ViewNavigator::LEFT:
        fs = "1"; break;
    case ViewNavigator::REAR:
        fs = "2"; break;
    case ViewNavigator::RIGHT:
        fs = "3"; break;
    default:
        fs = "-1";
    }   // end switch
    return fs;
}   // end viewFaceToString


ViewNavigator::ViewFace ViewNavigator::getViewDirection() const
{
    ViewFace d = NONE;
    if ( ui->frontRadioButton->isChecked())
        d = FRONT;
    else if ( ui->leftRadioButton->isChecked())
        d = LEFT;
    else if ( ui->rearRadioButton->isChecked())
        d = REAR;
    else if ( ui->rightRadioButton->isChecked())
        d = RIGHT;
    return d;
}   // end getViewDirection


void ViewNavigator::setViewDirection( ViewFace vf)
{
    switch ( vf)
    {
        case FRONT:
            ui->frontRadioButton->setChecked(true);
            break;
        case LEFT:
            ui->leftRadioButton->setChecked(true);
            break;
        case REAR:
            ui->rearRadioButton->setChecked(true);
            break;
        case RIGHT:
            ui->rightRadioButton->setChecked(true);
            break;
    }   // end switch
}   // end setViewDirection



ViewNavigator::ViewType ViewNavigator::getViewType() const
{
    ViewType t = IMAGE;
    if ( ui->viewDepthRadioButton->isChecked())
        t = DEPTH;
    else if ( USE_POINTS && ui->viewPointsRadioButton->isChecked())
        t = POINTS;
    return t;
}   // end getViewType


void ViewNavigator::setViewType( ViewNavigator::ViewType vt)
{
    switch ( vt)
    {
        case ViewNavigator::IMAGE:
            ui->viewImageRadioButton->setChecked(true);
            break;
        case ViewNavigator::DEPTH:
            ui->viewDepthRadioButton->setChecked(true);
            break;
        case ViewNavigator::POINTS:
            if (USE_POINTS)
                ui->viewPointsRadioButton->setChecked(true);
            break;
    }   // end switch
}   // end setViewType


void ViewNavigator::enableControls( bool enable)
{
    ui->frontRadioButton->setEnabled(enable);
    ui->leftRadioButton->setEnabled(enable);
    ui->rearRadioButton->setEnabled(enable);
    ui->rightRadioButton->setEnabled(enable);

    ui->viewImageRadioButton->setEnabled(enable);
    ui->viewDepthRadioButton->setEnabled(enable);
    if ( USE_POINTS)
        ui->viewPointsRadioButton->setEnabled(enable);
}   // end enableControls


void ViewNavigator::reset()
{
    enableControls( false);
    ui->frontRadioButton->setChecked(true);
    ui->viewImageRadioButton->setChecked(true);
}   // end reset


// private slot
void ViewNavigator::userSelectViewDirectionSlot()
{
    const ViewFace vd = getViewDirection();
    emit selectedView( vd);
}   // end userSelectViewDirectionSlot
