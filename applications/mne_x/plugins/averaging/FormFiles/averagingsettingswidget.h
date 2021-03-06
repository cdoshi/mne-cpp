#ifndef AVERAGINGSETTINGSWIDGET_H
#define AVERAGINGSETTINGSWIDGET_H

//*************************************************************************************************************
//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================

#include <QWidget>
#include <QSpinBox>
#include <QPair>

#include <QComboBox>
#include <QCheckBox>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE AveragingPlugin
//=============================================================================================================

namespace AveragingPlugin
{

//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

class Averaging;


class AveragingSettingsWidget : public QWidget
{
    Q_OBJECT

    friend class Averaging;

public:
    typedef QSharedPointer<AveragingSettingsWidget> SPtr;         /**< Shared pointer type for AveragingAdjustmentWidget. */
    typedef QSharedPointer<AveragingSettingsWidget> ConstSPtr;    /**< Const shared pointer type for AveragingAdjustmentWidget. */

    explicit AveragingSettingsWidget(Averaging *toolbox, QWidget *parent = 0);

signals:

public slots:

private:
    QComboBox* m_pComboBoxChSelection;
    Averaging* m_pAveragingToolbox;
    QSpinBox* m_pSpinBoxNumAverages;
    QSpinBox* m_pSpinBoxPreStimSamples;
    QSpinBox* m_pSpinBoxPostStimSamples;
};

} // NAMESPACE

#endif // AVERAGINGSETTINGSWIDGET_H
