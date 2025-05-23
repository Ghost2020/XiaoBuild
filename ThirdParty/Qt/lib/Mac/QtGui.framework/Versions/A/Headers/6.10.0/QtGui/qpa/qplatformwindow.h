// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QPLATFORMWINDOW_H
#define QPLATFORMWINDOW_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QPA API and is not meant to be used
// in applications. Usage of this API may make your code
// source and binary incompatible with future versions of Qt.
//

#include <QtGui/qtguiglobal.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qrect.h>
#include <QtCore/qmargins.h>
#include <QtCore/qstring.h>
#include <QtGui/qwindowdefs.h>
#include <QtGui/qwindow.h>
#include <qpa/qplatformopenglcontext.h>
#include <qpa/qplatformsurface.h>

QT_BEGIN_NAMESPACE

#define QWINDOWSIZE_MAX ((1<<24)-1)

class QPlatformScreen;
class QPlatformWindowPrivate;
class QScreen;
class QWindow;
class QIcon;
class QRegion;

class Q_GUI_EXPORT QPlatformWindow : public QPlatformSurface
{
    Q_DECLARE_PRIVATE(QPlatformWindow)
public:
    Q_DISABLE_COPY_MOVE(QPlatformWindow)

    explicit QPlatformWindow(QWindow *window);
    ~QPlatformWindow() override;

    virtual void initialize();

    QWindow *window() const;
    QPlatformWindow *parent() const;

    QPlatformScreen *screen() const override;

    virtual QSurfaceFormat format() const override;

    virtual void setGeometry(const QRect &rect);
    virtual QRect geometry() const;
    virtual QRect normalGeometry() const;

    virtual QMargins frameMargins() const;
    virtual QMargins safeAreaMargins() const;

    virtual void setVisible(bool visible);
    virtual void setWindowFlags(Qt::WindowFlags flags);
    virtual void setWindowState(Qt::WindowStates state);

    virtual WId winId() const;
    virtual void setParent(const QPlatformWindow *window);

    virtual void setWindowTitle(const QString &title);
    virtual QString windowTitle() const;
    virtual void setWindowFilePath(const QString &title);
    virtual void setWindowIcon(const QIcon &icon);
    virtual bool close();
    virtual void raise();
    virtual void lower();

    virtual bool isExposed() const;
    virtual bool isActive() const;
    virtual bool isAncestorOf(const QPlatformWindow *child) const;
    virtual bool isEmbedded() const;
    virtual bool isForeignWindow() const { return false; }
    virtual QPoint mapToGlobal(const QPoint &pos) const;
    QPointF mapToGlobalF(const QPointF &pos) const;
    virtual QPoint mapFromGlobal(const QPoint &pos) const;
    QPointF mapFromGlobalF(const QPointF &pos) const;

    virtual void propagateSizeHints();

    virtual void setOpacity(qreal level);
    virtual void setMask(const QRegion &region);
    virtual void requestActivateWindow();

    virtual void handleContentOrientationChange(Qt::ScreenOrientation orientation);

    virtual qreal devicePixelRatio() const;

    virtual bool setKeyboardGrabEnabled(bool grab);
    virtual bool setMouseGrabEnabled(bool grab);

    virtual bool setWindowModified(bool modified);

    virtual bool windowEvent(QEvent *event);

    virtual bool startSystemResize(Qt::Edges edges);
    virtual bool startSystemMove();

    virtual void setFrameStrutEventsEnabled(bool enabled);
    virtual bool frameStrutEventsEnabled() const;

    virtual void setAlertState(bool enabled);
    virtual bool isAlertState() const;

    virtual void invalidateSurface();

    static QRect initialGeometry(const QWindow *w, const QRect &initialGeometry,
                                 int defaultWidth, int defaultHeight,
                                 const QScreen **resultingScreenReturn = nullptr);

    virtual void requestUpdate();
    bool hasPendingUpdateRequest() const;
    virtual void deliverUpdateRequest();
    virtual bool allowsIndependentThreadedRendering() const;

    // Window property accessors. Platform plugins should use these
    // instead of accessing QWindow directly.
    QSize windowMinimumSize() const;
    QSize windowMaximumSize() const;
    QSize windowBaseSize() const;
    QSize windowSizeIncrement() const;
    QRect windowGeometry() const;
    QRect windowFrameGeometry() const;
    QRectF windowClosestAcceptableGeometry(const QRectF &nativeRect) const;
    static QRectF closestAcceptableGeometry(const QWindow *w, const QRectF &nativeRect);

protected:
    static QString formatWindowTitle(const QString &title, const QString &separator);
    QPlatformScreen *screenForGeometry(const QRect &newGeometry) const;
    static QSize constrainWindowSize(const QSize &size);

    QScopedPointer<QPlatformWindowPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif //QPLATFORMWINDOW_H
