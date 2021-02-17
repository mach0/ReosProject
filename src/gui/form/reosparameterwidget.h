/***************************************************************************
  reosparameterwidget.h - ReosParameterWidget

 ---------------------
 begin                : 22.1.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef REOSPARAMETERWIDGET_H
#define REOSPARAMETERWIDGET_H

#include <QPointer>
#include <QWidget>

class QLabel;
class QLineEdit;
class QComboBox;
class QToolButton;
class QDateTimeEdit;
class QBoxLayout;
class QSpacerItem;

#include "reosparameter.h"

class ReosParameterWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit ReosParameterWidget( const QString &defaultName, QWidget *parent = nullptr );

    //! Sets the keyboard focus on the edit part of the widget
    virtual void setFocusOnEdit() = 0;
    static ReosParameterWidget *createWidget( ReosParameter *parameter, QWidget *parent = nullptr );

    void enableSpacer( bool b );
    void setDefaultName( const QString &defaultName );
    void hideWhenVoid( bool b );

  public slots:
    //! Updates the dispayed value from the parameter
    virtual void updateValue() = 0;
    //! Sets the parameter value with the displayed value
    virtual void applyValue() = 0;
    virtual void askDerivation();

  signals:
    void valueChanged();
    void unitChanged();

  protected:
    void finalizeWidget();
    void setParameter( ReosParameter *param );
    bool mHideWhenVoid = true;

    QPointer<ReosParameter> mParameter = nullptr;

  private:
    QLabel *mLabelName = nullptr;
    QToolButton *mDerivationButton = nullptr;
    QBoxLayout *mLayout = nullptr;
    QSpacerItem *mSpacer = nullptr;
    QString mDefaultName;

};

class ReosParameterInLineWidget : public ReosParameterWidget
{
  public:
    ReosParameterInLineWidget( QWidget *parent = nullptr, const QString &defaultName = QString() );
    void setFocusOnEdit() override;

  protected:
    void setTextValue( double value );
    void setTextValue( const QString &str );
    double value() const;
    QString textValue() const;
    bool textHasChanged() const;
  private:
    QLineEdit *mLineEdit = nullptr;
    mutable QString mCurrentText;
};

class ReosParameterDoubleWidget : public ReosParameterInLineWidget
{
  public:
    explicit ReosParameterDoubleWidget( QWidget *parent = nullptr, const QString &defaultName = QString() );
    explicit ReosParameterDoubleWidget( ReosParameterDouble *value, QWidget *parent = nullptr );

    void setDouble( ReosParameterDouble *value );
    void updateValue();
    void applyValue();
    static QString type() {return QStringLiteral( "double" );}
    ReosParameterDouble *doubleParameter();
};

class ReosParameterStringWidget : public ReosParameterInLineWidget
{
  public:
    explicit ReosParameterStringWidget( QWidget *parent = nullptr, const QString &defaultName = QString() );
    explicit ReosParameterStringWidget( ReosParameterString *string, QWidget *parent = nullptr );

    void setString( ReosParameterString *string );
    void updateValue();
    void applyValue();

    static QString type() {return QStringLiteral( "string" );}
    ReosParameterString *stringParameter();
};

class ReosParameterAreaWidget: public ReosParameterInLineWidget
{
  public:
    explicit ReosParameterAreaWidget( QWidget *parent = nullptr, const QString &defaultName = QString() );
    explicit ReosParameterAreaWidget( ReosParameterArea *area, QWidget *parent = nullptr );

    void setArea( ReosParameterArea *area );
    void updateValue();
    void applyValue();

    static QString type() {return QStringLiteral( "area" );}

    ReosParameterArea *areaParameter() const;

  private:
    QComboBox *mUnitCombobox = nullptr;
};

class ReosParameterSlopeWidget: public ReosParameterInLineWidget
{
  public:
    explicit ReosParameterSlopeWidget( QWidget *parent = nullptr, const QString &defaultName = QString() );
    explicit ReosParameterSlopeWidget( ReosParameterSlope *slope, QWidget *parent = nullptr );

    void setSlope( ReosParameterSlope *slope );
    void updateValue();
    void applyValue();

    static QString type() {return QStringLiteral( "slope" );}

    ReosParameterSlope *slopeParameter() const;

  private:

    QLabel *mLabelSlopeUnit = nullptr;
    int mFactor = 100;
};


class ReosParameterDurationWidget: public ReosParameterInLineWidget
{
  public:
    explicit ReosParameterDurationWidget( QWidget *parent = nullptr, const QString &defaultName = QString() );
    explicit ReosParameterDurationWidget( ReosParameterDuration *area, QWidget *parent = nullptr );

    void setDuration( ReosParameterDuration *duration );
    void updateValue();
    void applyValue();

    static QString type() {return QStringLiteral( "duration" );}

    ReosParameterDuration *durationParameter() const;

  private:

    QComboBox *mUnitCombobox = nullptr;
};

class ReosParameterDateTimeWidget: public ReosParameterWidget
{
  public:
    explicit ReosParameterDateTimeWidget( QWidget *parent = nullptr, const QString &defaultName = QString() );
    explicit ReosParameterDateTimeWidget( ReosParameterDateTime *dateTime, QWidget *parent = nullptr );

    void setDateTime( ReosParameterDateTime *dateTime );
    void updateValue() override;
    void applyValue() override;
    void setFocusOnEdit() override;

    static QString type() {return QStringLiteral( "date-time" );}

    ReosParameterDateTime *dateTimeParameter() const;

  private:
    QDateTimeEdit *mDateTimeEdit = nullptr;



};
#endif // REOSPARAMETERWIDGET_H
