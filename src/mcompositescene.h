/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef DUICOMPOSITESCENE_H
#define DUICOMPOSITESCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <X11/Xlib.h>
#include <map>

class QMouseEvent;

/*!
 * The QGraphicsScene used by MCompositor to render the MGLXTexturePixmap
 * items.
 *
 * We use a custom scene to capture mouse and keyboard events generated by
 * the user as they manipulate the composited items on the screen
 */
class MCompositeScene: public QGraphicsScene
{
    Q_OBJECT
public:

    /*!
     * Constructs a MCompositeScene
     *
     * \param parent QObject, defaults to 0
     */
    MCompositeScene(QObject *parent = 0);

    /*!
     * Prepares the root window by hooking up to its event delivery system and
     * ensuring future created children redirect themselves to offscreen pixmaps
     */
    void prepareRoot();

protected:
    void drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget);

private:

    Window root;
    bool drawActive;

signals:

    void switchWindow();
};

#endif
