/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of duicompositor.
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
#ifndef MDECORATORWINDOW_H
#define MDECORATORWINDOW_H

#include <MWindow>
#include <mstatusbar.h>

#include <X11/Xlib.h>
#ifdef HAVE_SHAPECONST
#include <X11/extensions/shapeconst.h>
#else
#include <X11/extensions/shape.h>
#endif

#include <QObject>

class MSceneManager;
class MDecorator;

class MDecoratorWindow : public MWindow
{
    Q_OBJECT

public:
    explicit MDecoratorWindow(QWidget *parent = 0);
    virtual ~MDecoratorWindow();
    
    const QRect availableClientRect() const;
    bool x11Event(XEvent *e);
    void setWindowTitle(const QString& title);
    void setOnlyStatusbar(bool mode);
    /*!
     * \brief Sets the region of the window that can receive input events.
     *
     * Input events landing on the area outside this region will fall directly
     * to the windows below.
     */
    void setInputRegion();

protected:
    virtual void closeEvent(QCloseEvent * event );

private slots:

    void screenRotated(const M::Orientation &orientation);

signals:

    void homeClicked();
    void escapeClicked();

private:
    void setSceneSize();
    void setMDecoratorWindowProperty();
    XRectangle itemRectToScreenRect(const QRect& r);

    MStatusBar *statusBar;
    QRect decoratorRect;
    bool only_statusbar;
    Atom onlyStatusbarAtom, managedWindowAtom;
    MDecorator *d;

    Q_DISABLE_COPY(MDecoratorWindow);
};

#endif
