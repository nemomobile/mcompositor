#include <QtTest/QtTest>
#include <QtGui>
#include <QGLWidget>
#include <mcompositemanager.h>
#include <mcompositemanager_p.h>
#include <mwindowpropertycache.h>
#include <mcompositewindow.h>
#include <mcompositewindowanimation.h>
#include <mtexturepixmapitem.h>
#include "ut_anim.h"

#include <QtDebug>

#include <X11/Xlib.h>

static int dheight, dwidth;

// Skip bad window messages for mock windows
static int error_handler(Display * , XErrorEvent *)
{    
    return 0;
}

class fake_LMT_window : public MWindowPropertyCache
{
public:
    fake_LMT_window(Window w, bool is_mapped = true)
        : MWindowPropertyCache(None, &attrs)
    {
        window = w;
        memset(&attrs, 0, sizeof(attrs));
        setIsMapped(is_mapped);
        setRealGeometry(QRect(0, 0, dwidth, dheight));
        // icon geometry can be required for iconifying animation
        icon_geometry = QRect(0, 0, dwidth / 2, dheight / 2);
        type_atoms.append(ATOM(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE));
        window_state = NormalState;
        has_alpha = 0;
        // mark valid to create animation object
        is_valid = true;
    }

    xcb_get_window_attributes_reply_t attrs;
    friend class ut_Anim;
};

class fake_desktop_window : public MWindowPropertyCache
{
public:
    fake_desktop_window(Window w)
        : MWindowPropertyCache(None, &attrs)
    {
        window = w;
        memset(&attrs, 0, sizeof(attrs));
        setIsMapped(true);
        setRealGeometry(QRect(0, 0, dwidth, dheight));
        type_atoms.append(ATOM(_NET_WM_WINDOW_TYPE_DESKTOP));
        window_state = NormalState;
        has_alpha = 0;
        is_valid = true;
    }

    xcb_get_window_attributes_reply_t attrs;
};

void ut_Anim::initTestCase()
{
    cmgr = (MCompositeManager*)qApp;

    // create a fake desktop window
    fake_desktop_window *pc = new fake_desktop_window(1000);
    cmgr->d->prop_caches[1000] = pc;
    XMapEvent e;
    memset(&e, 0, sizeof(e));
    e.window = 1000;
    e.event = QX11Info::appRootWindow();
    cmgr->d->mapEvent(&e);
    MCompositeWindow *cw = cmgr->d->windows.value(1000, 0);
    QCOMPARE(cw != 0, true);
    QCOMPARE(cw->isValid(), true);
}

void ut_Anim::testStartupAnimForFirstTimeMapped()
{
    fake_LMT_window *pc = new fake_LMT_window(1, false);
    cmgr->d->prop_caches[1] = pc;
    // create a fake MapNotify event
    XMapEvent e;
    memset(&e, 0, sizeof(e));
    e.window = 1;
    e.event = QX11Info::appRootWindow();
    cmgr->d->mapEvent(&e);

    MCompositeWindow *cw = cmgr->d->windows.value(1, 0);
    QCOMPARE(cw != 0, true);
    QCOMPARE(cw->isValid(), true);
    QCOMPARE(cw->windowAnimator() != 0, true);

    cw->damageReceived();
    cw->damageReceived();

    QCOMPARE(cw->windowAnimator()->isActive(), true);

    while (cw->windowAnimator()->isActive())
        QTest::qWait(1000); // wait the animation to finish

    QCOMPARE(cw->propertyCache()->windowState(), NormalState);
    int d_i = cmgr->d->stacking_list.indexOf(1000);
    int w_i = cmgr->d->stacking_list.indexOf(1);
    QCOMPARE(d_i >= 0 && w_i >= 0, true);
    QCOMPARE(d_i < w_i, true);
}

void ut_Anim::testIconifyingAnimation()
{
    // iconifies window 1
    cmgr->d->exposeSwitcher();
    MCompositeWindow *cw = cmgr->d->windows.value(1, 0);
    QCOMPARE(cw != 0, true);
    QCOMPARE(cw->isValid(), true);
    QCOMPARE(cw->windowAnimator() != 0, true);
    QCOMPARE(cw->windowAnimator()->isActive(), true);
    while (cw->windowAnimator()->isActive())
        QTest::qWait(1000); // wait the animation to finish

    QCOMPARE(cw->propertyCache()->windowState(), IconicState);
    int d_i = cmgr->d->stacking_list.indexOf(1000);
    int w_i = cmgr->d->stacking_list.indexOf(1);
    QCOMPARE(d_i >= 0 && w_i >= 0, true);
    QCOMPARE(d_i > w_i, true);
}

void ut_Anim::testRestoreAnimation()
{
    MCompositeWindow *cw = cmgr->d->windows.value(1, 0);
    XClientMessageEvent cme;
    memset(&cme, 0, sizeof(cme));
    cme.window = 1;
    cme.type = ClientMessage;
    cme.message_type = ATOM(_NET_ACTIVE_WINDOW);
    cmgr->d->rootMessageEvent(&cme);
    QCOMPARE(cw->windowAnimator()->isActive(), true);
    while (cw->windowAnimator()->isActive())
        QTest::qWait(1000); // wait the animation to finish

    QCOMPARE(cw->propertyCache()->windowState(), NormalState);
    int d_i = cmgr->d->stacking_list.indexOf(1000);
    int w_i = cmgr->d->stacking_list.indexOf(1);
    QCOMPARE(d_i >= 0 && w_i >= 0, true);
    QCOMPARE(d_i < w_i, true);
}

void ut_Anim::testCloseAnimation()
{
    MCompositeWindow *cw = cmgr->d->windows.value(1, 0);
    XUnmapEvent ue;
    memset(&ue, 0, sizeof(ue));
    ue.window = 1;
    ue.event = QX11Info::appRootWindow();
    // use a bogus pixmap id
    ((MTexturePixmapItem*)cw)->d->windowp = 1;
    cmgr->d->unmapEvent(&ue);
    QCOMPARE(cw->windowAnimator()->isActive(), true);
    while (cw->windowAnimator()->isActive())
        QTest::qWait(1000); // wait the animation to finish
}

void ut_Anim::testStartupAnimForSecondTimeMapped()
{
    fake_LMT_window *pc = new fake_LMT_window(2, false);
    cmgr->d->prop_caches[2] = pc;
    // create a fake MapNotify event
    XMapEvent me;
    memset(&me, 0, sizeof(me));
    me.window = 2;
    me.event = QX11Info::appRootWindow();
    cmgr->d->mapEvent(&me);

    MCompositeWindow *cw = cmgr->d->windows.value(2, 0);
    QCOMPARE(cw != 0, true);
    QCOMPARE(cw->isValid(), true);

    cw->damageReceived();
    cw->damageReceived();

    // create a fake UnmapNotify event
    XUnmapEvent ue;
    memset(&ue, 0, sizeof(ue));
    ue.window = 2;
    ue.event = QX11Info::appRootWindow();
    // use a bogus pixmap id
    ((MTexturePixmapItem*)cw)->d->windowp = 1;
    cmgr->d->unmapEvent(&ue);

    while (cw->windowAnimator()->isActive())
        QTest::qWait(1000); // wait the animation to finish

    // map it again and check that there is an animation
    cmgr->d->mapEvent(&me);

    cw->damageReceived();
    cw->damageReceived();

    QCOMPARE(cw->windowAnimator()->isActive(), true);

    while (cw->windowAnimator()->isActive())
        QTest::qWait(1000); // wait the animation to finish

    QCOMPARE(cw->propertyCache()->windowState(), NormalState);
    int d_i = cmgr->d->stacking_list.indexOf(1000);
    int w_i = cmgr->d->stacking_list.indexOf(1);
    QCOMPARE(d_i >= 0 && w_i >= 0, true);
    QCOMPARE(d_i < w_i, true);
}

void ut_Anim::testNoAnimations()
{
    fake_LMT_window *pc = new fake_LMT_window(3, false);
    pc->no_animations = true;
    cmgr->d->prop_caches[3] = pc;
    // create a fake MapNotify event
    XMapEvent e;
    memset(&e, 0, sizeof(e));
    e.window = 3;
    e.event = QX11Info::appRootWindow();
    cmgr->d->mapEvent(&e);

    MCompositeWindow *cw = cmgr->d->windows.value(3, 0);
    QCOMPARE(cw != 0, true);
    QCOMPARE(cw->isValid(), true);
    QCOMPARE(cw->windowAnimator() != 0, true);

    cw->damageReceived();
    cw->damageReceived();

    // check that there is no animation
    QCOMPARE(cw->windowAnimator()->isActive(), false);
    QCOMPARE(cw->isMapped(), true);
    QCOMPARE(cw->isVisible(), true);
    QCOMPARE(cw->propertyCache()->windowState(), NormalState);
    int d_i = cmgr->d->stacking_list.indexOf(1000);
    int w_i = cmgr->d->stacking_list.indexOf(3);
    QCOMPARE(d_i >= 0 && w_i >= 0, true);
    QCOMPARE(d_i < w_i, true);

    // check that iconifying does not have animation
    cmgr->d->exposeSwitcher();
    QCOMPARE(cw->windowAnimator()->isActive(), false);
    QCOMPARE(cw->propertyCache()->windowState(), IconicState);
    d_i = cmgr->d->stacking_list.indexOf(1000);
    w_i = cmgr->d->stacking_list.indexOf(3);
    QCOMPARE(d_i >= 0 && w_i >= 0, true);
    QCOMPARE(d_i > w_i, true);

    // check that restore does not have animation
    XClientMessageEvent cme;
    memset(&cme, 0, sizeof(cme));
    cme.window = 3;
    cme.type = ClientMessage;
    cme.message_type = ATOM(_NET_ACTIVE_WINDOW);
    cmgr->d->rootMessageEvent(&cme);
    QCOMPARE(cw->windowAnimator()->isActive(), false);

    // create a fake UnmapNotify event
    XUnmapEvent ue;
    memset(&ue, 0, sizeof(ue));
    ue.window = 3;
    ue.event = QX11Info::appRootWindow();
    // use a bogus pixmap id
    ((MTexturePixmapItem*)cw)->d->windowp = 1;
    cmgr->d->unmapEvent(&ue);
    QCOMPARE(cw->isVisible(), false);
    QCOMPARE(cw->isMapped(), false);

    // check that there is no animation
    QCOMPARE(cw->windowAnimator()->isActive(), false);
}

int main(int argc, char* argv[])
{
    // init fake but basic compositor environment
    QApplication::setGraphicsSystem("native");
    QCoreApplication::setLibraryPaths(QStringList());
    MCompositeManager app(argc, argv);
    
    XSetErrorHandler(error_handler);
    
    QGraphicsScene *scene = app.scene();
    QGraphicsView view(scene);
    view.setFrameStyle(0);
    
    QGLFormat fmt;
    fmt.setSamples(0);
    fmt.setSampleBuffers(false);

    QGLWidget w(fmt);
    w.setAttribute(Qt::WA_PaintOutsidePaintEvent);    
    w.setAutoFillBackground(false);
    dheight = QApplication::desktop()->height();
    dwidth = QApplication::desktop()->width();
    w.setMinimumSize(dwidth, dheight);
    w.setMaximumSize(dwidth, dheight);
    app.setGLWidget(&w);

    view.setViewport(&w);
    w.makeCurrent();

    ut_Anim anim;

    return QTest::qExec(&anim);
}