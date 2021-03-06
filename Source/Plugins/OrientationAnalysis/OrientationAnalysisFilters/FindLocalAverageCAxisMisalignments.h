/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#ifndef _FindLocalAverageCAxisMisalignments_H_
#define _FindLocalAverageCAxisMisalignments_H_

#include <QtCore/QObject>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"

#include "DREAM3DLib/DataArrays/IDataArray.h"
#include "DREAM3DLib/DataArrays/NeighborList.hpp"
#include "DREAM3DLib/DataContainers/DataContainer.h"
#include "OrientationLib/SpaceGroupOps/SpaceGroupOps.h"

/**
 * @class FindLocalAverageCAxisMisalignments FindLocalAverageCAxisMisalignments.h Plugins/Statistics/StatisticsFilters/FindLocalAverageCAxisMisalignments.h
 * @brief
 * @author Joseph C. Tucker
 * @date Jan 30, 2014
 * @version 5.0
 */
class FindLocalAverageCAxisMisalignments : public AbstractFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    DREAM3D_SHARED_POINTERS(FindLocalAverageCAxisMisalignments)
    DREAM3D_STATIC_NEW_MACRO(FindLocalAverageCAxisMisalignments)
    DREAM3D_TYPE_MACRO_SUPER(FindLocalAverageCAxisMisalignments, AbstractFilter)


    virtual ~FindLocalAverageCAxisMisalignments();

    DREAM3D_FILTER_PARAMETER(DataArrayPath, NewCellFeatureAttributeMatrixName)
    Q_PROPERTY(DataArrayPath NewCellFeatureAttributeMatrixName READ getNewCellFeatureAttributeMatrixName WRITE setNewCellFeatureAttributeMatrixName)

    DREAM3D_FILTER_PARAMETER(bool, CalcUnbiasedAvg)
    Q_PROPERTY(bool CalcUnbiasedAvg READ getCalcUnbiasedAvg WRITE setCalcUnbiasedAvg)

    DREAM3D_FILTER_PARAMETER(bool, CalcBiasedAvg)
    Q_PROPERTY(bool CalcBiasedAvg READ getCalcBiasedAvg WRITE setCalcBiasedAvg)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, NeighborListArrayPath)
    Q_PROPERTY(DataArrayPath NeighborListArrayPath READ getNeighborListArrayPath WRITE setNeighborListArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, CAxisMisalignmentListArrayPath)
    Q_PROPERTY(DataArrayPath CAxisMisalignmentListArrayPath READ getCAxisMisalignmentListArrayPath WRITE setCAxisMisalignmentListArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, FeatureIdsArrayPath)
    Q_PROPERTY(DataArrayPath FeatureIdsArrayPath READ getFeatureIdsArrayPath WRITE setFeatureIdsArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, CellParentIdsArrayPath)
    Q_PROPERTY(DataArrayPath CellParentIdsArrayPath READ getCellParentIdsArrayPath WRITE setCellParentIdsArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, AvgCAxisMisalignmentsArrayPath)
    Q_PROPERTY(DataArrayPath AvgCAxisMisalignmentsArrayPath READ getAvgCAxisMisalignmentsArrayPath WRITE setAvgCAxisMisalignmentsArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, FeatureParentIdsArrayPath)
    Q_PROPERTY(DataArrayPath FeatureParentIdsArrayPath READ getFeatureParentIdsArrayPath WRITE setFeatureParentIdsArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, CrystalStructuresArrayPath)
    Q_PROPERTY(DataArrayPath CrystalStructuresArrayPath READ getCrystalStructuresArrayPath WRITE setCrystalStructuresArrayPath)

    DREAM3D_FILTER_PARAMETER(QString, UnbiasedLocalCAxisMisalignmentsArrayName)
    Q_PROPERTY(QString UnbiasedLocalCAxisMisalignmentsArrayName READ getUnbiasedLocalCAxisMisalignmentsArrayName WRITE setUnbiasedLocalCAxisMisalignmentsArrayName)

    DREAM3D_FILTER_PARAMETER(QString, LocalCAxisMisalignmentsArrayName)
    Q_PROPERTY(QString LocalCAxisMisalignmentsArrayName READ getLocalCAxisMisalignmentsArrayName WRITE setLocalCAxisMisalignmentsArrayName)

    DREAM3D_FILTER_PARAMETER(QString, NumFeaturesPerParentArrayName)
    Q_PROPERTY(QString NumFeaturesPerParentArrayName READ getNumFeaturesPerParentArrayName WRITE setNumFeaturesPerParentArrayName)

    /**
     * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getCompiledLibraryName();

    /**
     * @brief newFilterInstance Reimplemented from @see AbstractFilter class
     */
    virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters);

    /**
     * @brief getGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getGroupName();

    /**
     * @brief getSubGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getSubGroupName();

    /**
     * @brief getHumanLabel Reimplemented from @see AbstractFilter class
     */
    virtual const QString getHumanLabel();

    /**
     * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void setupFilterParameters();

    /**
     * @brief writeFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
     * @brief readFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

   /**
    * @brief execute Reimplemented from @see AbstractFilter class
    */
    virtual void execute();

    /**
    * @brief preflight Reimplemented from @see AbstractFilter class
    */
    virtual void preflight();

  signals:
    /**
     * @brief updateFilterParameters Emitted when the Filter requests all the latest Filter parameters
     * be pushed from a user-facing control (such as a widget)
     * @param filter Filter instance pointer
     */
    void updateFilterParameters(AbstractFilter* filter);

    /**
     * @brief parametersChanged Emitted when any Filter parameter is changed internally
     */
    void parametersChanged();

    /**
     * @brief preflightAboutToExecute Emitted just before calling dataCheck()
     */
    void preflightAboutToExecute();

    /**
     * @brief preflightExecuted Emitted just after calling dataCheck()
     */
    void preflightExecuted();

  protected:
    FindLocalAverageCAxisMisalignments();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

  private:
    DEFINE_DATAARRAY_VARIABLE(int32_t, FeatureIds)
    DEFINE_DATAARRAY_VARIABLE(int32_t, CellParentIds)
    DEFINE_DATAARRAY_VARIABLE(int32_t, FeatureParentIds)
    DEFINE_DATAARRAY_VARIABLE(int32_t, NumFeaturesPerParent)
    DEFINE_DATAARRAY_VARIABLE(float, AvgCAxisMisalignments)
    DEFINE_DATAARRAY_VARIABLE(float, LocalCAxisMisalignments)
    DEFINE_DATAARRAY_VARIABLE(float, UnbiasedLocalCAxisMisalignments)
    DEFINE_DATAARRAY_VARIABLE(unsigned int, CrystalStructures)

    NeighborList<int>::WeakPointer m_NeighborList;
    NeighborList<float>::WeakPointer m_CAxisMisalignmentList;
    QVector<SpaceGroupOps::Pointer> m_OrientationOps;

    FindLocalAverageCAxisMisalignments(const FindLocalAverageCAxisMisalignments&); // Copy Constructor Not Implemented
    void operator=(const FindLocalAverageCAxisMisalignments&); // Operator '=' Not Implemented
};

#endif /* FindLocalAverageCAxisMisalignments_H_ */
