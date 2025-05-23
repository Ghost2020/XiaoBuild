// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QCOLORSPACE_H
#define QCOLORSPACE_H

#include <QtGui/qtguiglobal.h>
#include <QtGui/qcolortransform.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qpoint.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QColorSpacePrivate;

QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QColorSpacePrivate, Q_GUI_EXPORT)

class Q_GUI_EXPORT QColorSpace
{
    Q_GADGET
public:
    enum NamedColorSpace {
        SRgb = 1,
        SRgbLinear,
        AdobeRgb,
        DisplayP3,
        ProPhotoRgb,
        Bt2020,
        Bt2100Pq,
        Bt2100Hlg,
    };
    Q_ENUM(NamedColorSpace)
    enum class Primaries {
        Custom = 0,
        SRgb,
        AdobeRgb,
        DciP3D65,
        ProPhotoRgb,
        Bt2020,
    };
    Q_ENUM(Primaries)
    enum class TransferFunction {
        Custom = 0,
        Linear,
        Gamma,
        SRgb,
        ProPhotoRgb,
        Bt2020,
        St2084,
        Hlg,
    };
    Q_ENUM(TransferFunction)
    enum class TransformModel : uint8_t {
        ThreeComponentMatrix = 0,
        ElementListProcessing,
    };
    Q_ENUM(TransformModel)
    enum class ColorModel : uint8_t {
        Undefined = 0,
        Rgb = 1,
        Gray = 2,
        Cmyk = 3,
    };
    Q_ENUM(ColorModel)

    struct PrimaryPoints
    {
        Q_GUI_EXPORT static PrimaryPoints fromPrimaries(Primaries primaries);
        Q_GUI_EXPORT bool isValid() const noexcept;
        QPointF whitePoint;
        QPointF redPoint;
        QPointF greenPoint;
        QPointF bluePoint;
    };

    QColorSpace() noexcept = default;
    QColorSpace(NamedColorSpace namedColorSpace);
    explicit QColorSpace(QPointF whitePoint, TransferFunction transferFunction, float gamma = 0.0f);
    explicit QColorSpace(QPointF whitePoint, const QList<uint16_t> &transferFunctionTable);
    QColorSpace(Primaries primaries, TransferFunction transferFunction, float gamma = 0.0f);
    QColorSpace(Primaries primaries, float gamma);
    QColorSpace(Primaries primaries, const QList<uint16_t> &transferFunctionTable);
    QColorSpace(const QPointF &whitePoint, const QPointF &redPoint,
                const QPointF &greenPoint, const QPointF &bluePoint,
                TransferFunction transferFunction, float gamma = 0.0f);
    QColorSpace(const PrimaryPoints &primaryPoints,
                TransferFunction transferFunction, float gamma = 0.0f);
    QColorSpace(const QPointF &whitePoint, const QPointF &redPoint,
                const QPointF &greenPoint, const QPointF &bluePoint,
                const QList<uint16_t> &transferFunctionTable);
    QColorSpace(const QPointF &whitePoint, const QPointF &redPoint,
                const QPointF &greenPoint, const QPointF &bluePoint,
                const QList<uint16_t> &redTransferFunctionTable,
                const QList<uint16_t> &greenTransferFunctionTable,
                const QList<uint16_t> &blueTransferFunctionTable);
    ~QColorSpace();

    QColorSpace(const QColorSpace &colorSpace) noexcept;
    QColorSpace &operator=(const QColorSpace &colorSpace) noexcept
    {
        QColorSpace copy(colorSpace);
        swap(copy);
        return *this;
    }

    QColorSpace(QColorSpace &&colorSpace) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QColorSpace)

    void swap(QColorSpace &colorSpace) noexcept
    { d_ptr.swap(colorSpace.d_ptr); }

    Primaries primaries() const noexcept;
    TransferFunction transferFunction() const noexcept;
    float gamma() const noexcept;

    QString description() const noexcept;
    void setDescription(const QString &description);

    void setTransferFunction(TransferFunction transferFunction, float gamma = 0.0f);
    void setTransferFunction(const QList<uint16_t> &transferFunctionTable);
    void setTransferFunctions(const QList<uint16_t> &redTransferFunctionTable,
                              const QList<uint16_t> &greenTransferFunctionTable,
                              const QList<uint16_t> &blueTransferFunctionTable);
    QColorSpace withTransferFunction(TransferFunction transferFunction, float gamma = 0.0f) const;
    QColorSpace withTransferFunction(const QList<uint16_t> &transferFunctionTable) const;
    QColorSpace withTransferFunctions(const QList<uint16_t> &redTransferFunctionTable,
                                      const QList<uint16_t> &greenTransferFunctionTable,
                                      const QList<uint16_t> &blueTransferFunctionTable) const;

    void setPrimaries(Primaries primariesId);
    void setPrimaries(const QPointF &whitePoint, const QPointF &redPoint,
                      const QPointF &greenPoint, const QPointF &bluePoint);
    void setWhitePoint(QPointF whitePoint);
    QPointF whitePoint() const;
    void setPrimaryPoints(const PrimaryPoints &primaryPoints);
    PrimaryPoints primaryPoints() const;

    TransformModel transformModel() const noexcept;
    ColorModel colorModel() const noexcept;
    void detach();
    bool isValid() const noexcept;
    bool isValidTarget() const noexcept;

    friend inline bool operator==(const QColorSpace &colorSpace1, const QColorSpace &colorSpace2)
    { return colorSpace1.equals(colorSpace2); }
    friend inline bool operator!=(const QColorSpace &colorSpace1, const QColorSpace &colorSpace2)
    { return !(colorSpace1 == colorSpace2); }

    static QColorSpace fromIccProfile(const QByteArray &iccProfile);
    QByteArray iccProfile() const;

    QColorTransform transformationToColorSpace(const QColorSpace &colorspace) const;

    operator QVariant() const;

private:
    friend class QColorSpacePrivate;
    bool equals(const QColorSpace &other) const;

    QExplicitlySharedDataPointer<QColorSpacePrivate> d_ptr;

#ifndef QT_NO_DEBUG_STREAM
    friend Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QColorSpace &colorSpace);
#endif
};

Q_DECLARE_SHARED(QColorSpace)

// QColorSpace stream functions
#if !defined(QT_NO_DATASTREAM)
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QColorSpace &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QColorSpace &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QColorSpace &);
#endif

QT_END_NAMESPACE

#endif // QCOLORSPACE_P_H
