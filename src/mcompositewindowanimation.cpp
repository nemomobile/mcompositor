/***************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of mcompositor.
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/
/*!
  \class MCompositeWindowAnimation
  \brief MCompositeWindowAnimation class which provides full control 
  of position transformation and opacity animations of composite window objects.

  To create more complex animations, re-implement the virtual functions
  windowShown(), windowClosed(), widowIconified() and windowRestored(); 
  A QParallelAnimationGroup object is provided which can be a container
  for more complex animations for more flexibility.
*/
// TODO: import icongeometry. export signals when animation done to hook
// to composting on off

#include <QRectF>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <mcompositewindow.h>
#include <mcompositewindowanimation.h>
#include <QApplication>
#include <QParallelAnimationGroup>
#include <mcompositemanager.h>
#include <mcompositemanager_p.h>

class McParallelAnimation: public QParallelAnimationGroup
{
public:
    McParallelAnimation(MCompositeWindowAnimation* p)
        :QParallelAnimationGroup(p),
         parent(p)
    {}
        
protected:
    virtual void updateCurrentTime(int currentTime)
    {        
        MCompositeWindow::update();
        QParallelAnimationGroup::updateCurrentTime(currentTime);
    }

    virtual void updateState(QAbstractAnimation::State newState, 
                             QAbstractAnimation::State oldState)
    {   
        if (newState == QAbstractAnimation::Running && 
            oldState == QAbstractAnimation::Stopped) {
            parent->ensureAnimationVisible();
            if (parent->targetWindow())
                parent->targetWindow()->beginAnimation();
            if (parent->targetWindow2())
                parent->targetWindow2()->beginAnimation();
        } else if (newState == QAbstractAnimation::Stopped) {
            if (parent->targetWindow())
                parent->targetWindow()->endAnimation();
            if (parent->targetWindow2())
                parent->targetWindow2()->endAnimation();
        }
        QParallelAnimationGroup::updateState(newState, oldState);
    }
private:
    MCompositeWindowAnimation* parent;
};

class MCompositeWindowAnimationPrivate
{
public:
    MCompositeWindowAnimationPrivate(MCompositeWindowAnimation* animation)
        : crossfade(0),
          pending_animation(MCompositeWindowAnimation::NoAnimation)
    {
        scale = new QPropertyAnimation(animation);
        scale->setPropertyName("scale");
        int duration = ((MCompositeManager*)qApp)->config().value(
                                   "startup-anim-duration", 200).toInt();
        scale->setDuration(duration);
        
        position = new QPropertyAnimation(animation);
        position->setPropertyName("pos");
        position->setDuration(duration);
        
        opacity = new QPropertyAnimation(animation);
        opacity->setPropertyName("opacity");
        opacity->setDuration(duration);
        
        scalepos = new McParallelAnimation(animation);
        scalepos->addAnimation(scale);
        scalepos->addAnimation(position);
        scalepos->addAnimation(opacity);
        
        QObject::connect(scalepos, SIGNAL(finished()), animation,
                         SLOT(finalizeState()));
    }

    void setTargetWindow(MCompositeWindow* window)
    {
        if (scale && position && opacity) {
            scale->setTargetObject(window);
            position->setTargetObject(window);
            opacity->setTargetObject(window);
        }
    }

    QPointer<MCompositeWindow> target_window, target_window2;
    QPointer<QPropertyAnimation> scale;
    QPointer<QPropertyAnimation> position;
    QPointer<QPropertyAnimation> opacity;
    McParallelAnimation* scalepos, *crossfade;
    MCompositeWindowAnimation::AnimationType pending_animation;
};

MCompositeWindowAnimation::MCompositeWindowAnimation(QObject* parent)
    :QObject(parent),
     d_ptr(new MCompositeWindowAnimationPrivate(this))
{
}

MCompositeWindowAnimation::~MCompositeWindowAnimation()
{
}

void MCompositeWindowAnimation::setTargetWindow(MCompositeWindow* window)
{
    Q_D(MCompositeWindowAnimation);

    // replace the old animator if there is one
    if (window->animator && (window->animator != this))
        delete window->animator;

    d->target_window = window;
    d->target_window->animator = this;
    disconnect(SIGNAL(q_finalizeState()));
    connect(this, SIGNAL(q_finalizeState()), window, SLOT(finalizeState()));
    
    d->setTargetWindow(window);
}

void MCompositeWindowAnimation::finalizeState()
{
    emit q_finalizeState();
}

// returns a group animation for this animator 
QParallelAnimationGroup* MCompositeWindowAnimation::animationGroup() const
{
    Q_D(const MCompositeWindowAnimation);
    return d->scalepos;
}

MCompositeWindow* MCompositeWindowAnimation::targetWindow() const
{
    Q_D(const MCompositeWindowAnimation);
    return d->target_window;
}

MCompositeWindow* MCompositeWindowAnimation::targetWindow2() const
{
    Q_D(const MCompositeWindowAnimation);
    return d->target_window2;
}

// Exposed animation properties
QPropertyAnimation* MCompositeWindowAnimation::scaleAnimation() const
{
    Q_D(const MCompositeWindowAnimation);
    return d->scale;
}

QPropertyAnimation* MCompositeWindowAnimation::positionAnimation() const
{
    Q_D(const MCompositeWindowAnimation);
    return d->position;
}

QPropertyAnimation* MCompositeWindowAnimation::opacityAnimation() const
{
    Q_D(const MCompositeWindowAnimation);
    return d->opacity;
}

// Default effect
void MCompositeWindowAnimation::windowShown()
{
#define OPAQUE 1.0
#define DIMMED 0.1
    Q_D(MCompositeWindowAnimation);

    if (!d->target_window)
        return;

    const qreal scaleStart = 0.2;
    const QRectF &iconGeometry = d->target_window->propertyCache()->iconGeometry();
    QPointF topLeft = iconGeometry.topLeft();

    if (iconGeometry.isEmpty()) {
        const QRectF d = QApplication::desktop()->availableGeometry();
        topLeft.setX(d.width()/2.0f * (1.0f-scaleStart));
        topLeft.setY(d.height()/2.0f * (1.0f-scaleStart));
    }

    d->target_window->setPos(topLeft);
    positionAnimation()->setEasingCurve(QEasingCurve::OutQuad);
    positionAnimation()->setStartValue(topLeft);
    positionAnimation()->setEndValue(
        QPointF(d->target_window->propertyCache()->realGeometry().x(),
                d->target_window->propertyCache()->realGeometry().y()));
    scaleAnimation()->setEasingCurve(QEasingCurve::OutQuad);
    
    // TODO: use icon geometry signal
    scaleAnimation()->setStartValue(scaleStart);
    scaleAnimation()->setEndValue(1.0);
    opacityAnimation()->setEasingCurve(QEasingCurve::OutQuad);
    opacityAnimation()->setStartValue(DIMMED);
    opacityAnimation()->setEndValue(OPAQUE);

    animationGroup()->setDirection(QAbstractAnimation::Forward);
    animationGroup()->start();
}

void MCompositeWindowAnimation::windowClosed()
{
    positionAnimation()->setEasingCurve(QEasingCurve::InQuad);
    scaleAnimation()->setEasingCurve(QEasingCurve::InQuad);
    opacityAnimation()->setEasingCurve(QEasingCurve::InQuad);
    animationGroup()->setDirection(QAbstractAnimation::Backward);
    animationGroup()->start();
}

void MCompositeWindowAnimation::deferAnimation(MCompositeWindowAnimation::AnimationType type)
{
    Q_D(MCompositeWindowAnimation);
    d->pending_animation = type;
}

void MCompositeWindowAnimation::windowIconified()
{
    positionAnimation()->setEasingCurve(QEasingCurve::InQuad);
    scaleAnimation()->setEasingCurve(QEasingCurve::InQuad);
    opacityAnimation()->setEasingCurve(QEasingCurve::InQuad);
    animationGroup()->setDirection(QAbstractAnimation::Backward);
    animationGroup()->start();
}

void MCompositeWindowAnimation::windowRestored()
{
    positionAnimation()->setEasingCurve(QEasingCurve::OutQuad);
    scaleAnimation()->setEasingCurve(QEasingCurve::OutQuad);
    opacityAnimation()->setEasingCurve(QEasingCurve::OutQuad);
    animationGroup()->setDirection(QAbstractAnimation::Forward);
    animationGroup()->start();
}

void MCompositeWindowAnimation::crossFadeTo(MCompositeWindow *cw)
{
    Q_D(MCompositeWindowAnimation);

    if (d->crossfade)
        delete d->crossfade;
    d->crossfade = new McParallelAnimation(this);

    QPropertyAnimation *op = new QPropertyAnimation(this);
    op->setTargetObject(cw);
    op->setEasingCurve(QEasingCurve::Linear);
    op->setPropertyName("opacity");
    int duration = ((MCompositeManager*)qApp)->config().value(
                               "crossfade-duration", 250).toInt();
    op->setDuration(duration);
    op->setStartValue(0);
    op->setEndValue(1);
    d->crossfade->addAnimation(op);

    d->target_window2 = cw;

    d->crossfade->setDirection(QAbstractAnimation::Forward);
}

void MCompositeWindowAnimation::startTransition()
{        
    Q_D(MCompositeWindowAnimation);
    
    switch (d->pending_animation) {
    case Showing:
        windowShown();
        d->pending_animation = NoAnimation;
        break;
    case Closing:
        windowClosed();
        d->pending_animation = NoAnimation;
        break;
    case Iconify:
        windowIconified();
        d->pending_animation = NoAnimation;
        break;
    case Restore:
        windowRestored();
        d->pending_animation = NoAnimation;
        break;
    case CrossFade:
        if (targetWindow2())
            targetWindow2()->show();
        d->crossfade->start();
        d->pending_animation = NoAnimation;
        break;
    default:  break;
    }
}

void MCompositeWindowAnimation::ensureAnimationVisible()
{
    // Always ensure the animation is REALLY visible. Z-values get corrected 
    // later at checkStacking if needed
    if (targetWindow())
        targetWindow()->setZValue(
              ((MCompositeManager*)qApp)->d->stacking_list.size() + 1);
    if (targetWindow2())
        targetWindow2()->setZValue(
              ((MCompositeManager*)qApp)->d->stacking_list.size() + 2);
}

// plays the animation group;
void MCompositeWindowAnimation::start()
{
    animationGroup()->start();
}

// pauses the animation group;
void MCompositeWindowAnimation::pause()
{
    animationGroup()->pause();
}

bool MCompositeWindowAnimation::isActive()
{
    return (animationGroup()->state() != QAbstractAnimation::Stopped);
}

MCompositeWindowAnimation::AnimationType MCompositeWindowAnimation::pendingAnimation() const 
{ 
    Q_D(const MCompositeWindowAnimation);
    
    return d->pending_animation; 
}
