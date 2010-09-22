#include "QMap3DWidget.h"

#include <FTGL/ftgl.h>
#include <QPushButton>
#include <sys/time.h>

#include "CheetahModel.h"
#include "UASManager.h"
#include "UASInterface.h"

QMap3DWidget::QMap3DWidget(QWidget* parent)
     : Q3DWidget(parent)
     , uas(NULL)
     , lastRedrawTime(0.0)
{
    setFocusPolicy(Qt::StrongFocus);

    initialize(10, 10, 1000, 900, 10.0f);
    setCameraParams(0.05f, 0.5f, 0.001f, 0.5f, 30.0f, 0.01f, 400.0f);

    setDisplayFunc(display, this);
    addTimerFunc(100, timer, this);

    QPushButton* mapButton = new QPushButton(this);
    mapButton->setText("Grid");

    // display the MapControl in the application
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(2);
    //layout->addWidget(mc, 0, 0, 1, 2);
    layout->addWidget(mapButton, 1, 0);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 1, 1);
    layout->setRowStretch(0, 100);
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 50);
    setLayout(layout);

    font.reset(new FTTextureFont("images/Vera.ttf"));

    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)),
            this, SLOT(setActiveUAS(UASInterface*)));
}

QMap3DWidget::~QMap3DWidget()
{

}

void
QMap3DWidget::init(void)
{

}

void
QMap3DWidget::display(void* clientData)
{
    QMap3DWidget* map3d = reinterpret_cast<QMap3DWidget *>(clientData);
    map3d->displayHandler();
}

void
QMap3DWidget::displayHandler(void)
{
    if (cheetahModel.data() == 0)
    {
        cheetahModel.reset(new CheetahModel);
        cheetahModel->init(1.0f, 1.0f, 1.0f);
    }

    if (uas == NULL)
    {
        return;
    }

    // turn on smooth lines
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // clear window
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw Cheetah model
    glPushMatrix();

    glRotatef(uas->getYaw(), 0.0f, 0.0f, 1.0f);
    glRotatef(uas->getPitch(), 0.0f, 1.0f, 0.0f);
    glRotatef(uas->getRoll(), 1.0f, 0.0f, 0.0f);

    glLineWidth(3.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.3f, 0.0f, 0.0f);
    glEnd();

    cheetahModel->draw();

    glPopMatrix();

    setDisplayMode2D();

    // display pose information
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_POLYGON);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, 20.0f);
    glVertex2f(getWindowWidth(), 20.0f);
    glVertex2f(getWindowWidth(), 0.0f);
    glEnd();

    char buffer[6][255];

    sprintf(buffer[0], "x = %.2f", uas->getLocalX());
    sprintf(buffer[1], "y = %.2f", uas->getLocalY());
    sprintf(buffer[2], "z = %.2f", uas->getLocalZ());
    sprintf(buffer[3], "r = %.2f", uas->getRoll());
    sprintf(buffer[4], "p = %.2f", uas->getPitch());
    sprintf(buffer[5], "y = %.2f", uas->getYaw());

    font->FaceSize(10);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();

    glTranslatef(0.0f, 5.0f, 0.0f);
    for (int32_t i = 0; i < 6; ++i)
    {
        glTranslatef(60.0f, 0.0f, 0.0f);
        font->Render(buffer[i]);
    }
    glPopMatrix();
}

/**
 *
 * @param uas the UAS/MAV to monitor/display with the HUD
 */
void QMap3DWidget::setActiveUAS(UASInterface* uas)
{
    if (this->uas != NULL && this->uas != uas)
    {
        // Disconnect any previously connected active MAV
        //disconnect(uas, SIGNAL(valueChanged(UASInterface*,QString,double,quint64)), this, SLOT(updateValue(UASInterface*,QString,double,quint64)));
    }

    this->uas = uas;
}

void
QMap3DWidget::timer(void* clientData)
{
    QMap3DWidget* map3d = reinterpret_cast<QMap3DWidget *>(clientData);
    map3d->timerHandler();
}

void
QMap3DWidget::timerHandler(void)
{
    double timeLapsed = getTime() - lastRedrawTime;
    if (timeLapsed > 0.1)
    {
        forceRedraw();
        lastRedrawTime = getTime();
    }
    addTimerFunc(100, timer, this);
}

double
QMap3DWidget::getTime(void) const
{
     struct timeval tv;

     gettimeofday(&tv, NULL);

     return static_cast<double>(tv.tv_sec) +
             static_cast<double>(tv.tv_usec) / 1000000.0;
}