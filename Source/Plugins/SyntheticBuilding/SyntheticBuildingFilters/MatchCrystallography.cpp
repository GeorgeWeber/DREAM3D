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


#include "MatchCrystallography.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/SeparatorFilterParameter.h"
#include "DREAM3DLib/StatsData/PrecipitateStatsData.h"
#include "DREAM3DLib/StatsData/PrimaryStatsData.h"
#include "DREAM3DLib/Utilities/DREAM3DRandom.h"
#include "DREAM3DLib/Utilities/TimeUtilities.h"

#include "OrientationLib/SpaceGroupOps/CubicOps.h"
#include "OrientationLib/SpaceGroupOps/HexagonalOps.h"
#include "OrientationLib/SpaceGroupOps/SpaceGroupOps.h"
#include "OrientationLib/SpaceGroupOps/OrthoRhombicOps.h"
#include "OrientationLib/Texture/Texture.hpp"

#include "SyntheticBuilding/SyntheticBuildingConstants.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MatchCrystallography::MatchCrystallography() :
  AbstractFilter(),
  // These are from the Data Container that has the statistics, crystal structures and Phase Types (ideally),
  m_InputStatsArrayPath(DREAM3D::Defaults::StatsGenerator, DREAM3D::Defaults::CellEnsembleAttributeMatrixName, DREAM3D::EnsembleData::Statistics),
  m_CrystalStructuresArrayPath(DREAM3D::Defaults::StatsGenerator, DREAM3D::Defaults::CellEnsembleAttributeMatrixName, DREAM3D::EnsembleData::CrystalStructures),
  m_PhaseTypesArrayPath(DREAM3D::Defaults::StatsGenerator, DREAM3D::Defaults::CellEnsembleAttributeMatrixName, DREAM3D:: EnsembleData::PhaseTypes),
  m_FeatureIdsArrayPath(DREAM3D::Defaults::SyntheticVolumeDataContainerName, DREAM3D::Defaults::CellAttributeMatrixName, DREAM3D::CellData::FeatureIds),
  m_FeaturePhasesArrayPath(DREAM3D::Defaults::SyntheticVolumeDataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::Phases),
  m_SurfaceFeaturesArrayPath(DREAM3D::Defaults::SyntheticVolumeDataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::SurfaceFeatures),
  m_NeighborListArrayPath(DREAM3D::Defaults::SyntheticVolumeDataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::NeighborList),
  m_SharedSurfaceAreaListArrayPath(DREAM3D::Defaults::SyntheticVolumeDataContainerName, DREAM3D::Defaults::CellFeatureAttributeMatrixName, DREAM3D::FeatureData::SharedSurfaceAreaList),
  m_NumFeaturesArrayPath(DREAM3D::Defaults::SyntheticVolumeDataContainerName, DREAM3D::Defaults::CellEnsembleAttributeMatrixName, DREAM3D::EnsembleData::NumFeatures),
  m_CellEulerAnglesArrayName(DREAM3D::CellData::EulerAngles),
  m_VolumesArrayName(DREAM3D::FeatureData::Volumes),
  m_FeatureEulerAnglesArrayName(DREAM3D::FeatureData::EulerAngles),
  m_AvgQuatsArrayName(DREAM3D::FeatureData::AvgQuats),
  m_MaxIterations(1),
  m_FeatureIds(NULL),
  m_CellEulerAngles(NULL),
  m_SurfaceFeatures(NULL),
  m_FeaturePhases(NULL),
  m_Volumes(NULL),
  m_FeatureEulerAngles(NULL),
  m_AvgQuats(NULL),
  m_SyntheticCrystalStructures(NULL),
  m_CrystalStructures(NULL),
  m_PhaseTypes(NULL),
  m_NumFeatures(NULL)
{
  m_NeighborList = NeighborList<int32_t>::NullPointer();
  m_SharedSurfaceAreaList = NeighborList<float>::NullPointer();
  m_StatsDataArray = StatsDataArray::NullPointer();

  mdfchange = odfchange = 0.0f;

  actualodf = FloatArrayType::NullPointer();
  simodf = FloatArrayType::NullPointer();
  actualmdf = FloatArrayType::NullPointer();
  simmdf = FloatArrayType::NullPointer();

  m_OrientationOps = SpaceGroupOps::getOrientationOpsQVector();

  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MatchCrystallography::~MatchCrystallography()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FilterParameter::New("Maximum Number of Iterations (Swaps)", "MaxIterations", FilterParameterWidgetType::IntWidget, getMaxIterations(), FilterParameter::Parameter));

  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  parameters.push_back(FilterParameter::New("Feature Ids", "FeatureIdsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getFeatureIdsArrayPath(), FilterParameter::RequiredArray, ""));

  parameters.push_back(SeparatorFilterParameter::New("Cell Feature Data", FilterParameter::RequiredArray));
  parameters.push_back(FilterParameter::New("Phases", "FeaturePhasesArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getFeaturePhasesArrayPath(), FilterParameter::RequiredArray, ""));
  parameters.push_back(FilterParameter::New("Surface Features", "SurfaceFeaturesArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getSurfaceFeaturesArrayPath(), FilterParameter::RequiredArray, ""));
  parameters.push_back(FilterParameter::New("Neighbor List", "NeighborListArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getNeighborListArrayPath(), FilterParameter::RequiredArray, ""));
  parameters.push_back(FilterParameter::New("Shared Surface Area List", "SharedSurfaceAreaListArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getSharedSurfaceAreaListArrayPath(), FilterParameter::RequiredArray, ""));

  parameters.push_back(SeparatorFilterParameter::New("Cell Ensemble Data", FilterParameter::RequiredArray));
  parameters.push_back(FilterParameter::New("Statistics", "InputStatsArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getInputStatsArrayPath(), FilterParameter::RequiredArray, ""));
  parameters.push_back(FilterParameter::New("Crystal Structures", "CrystalStructuresArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getCrystalStructuresArrayPath(), FilterParameter::RequiredArray, ""));
  parameters.push_back(FilterParameter::New("Phase Types", "PhaseTypesArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getPhaseTypesArrayPath(), FilterParameter::RequiredArray, ""));
  parameters.push_back(FilterParameter::New("Number of Features", "NumFeaturesArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getNumFeaturesArrayPath(), FilterParameter::RequiredArray, ""));

  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(FilterParameter::New("Euler Angles", "CellEulerAnglesArrayName", FilterParameterWidgetType::StringWidget, getCellEulerAnglesArrayName(), FilterParameter::CreatedArray));

  parameters.push_back(SeparatorFilterParameter::New("Cell Feature Data", FilterParameter::CreatedArray));
  parameters.push_back(FilterParameter::New("Volumes", "VolumesArrayName", FilterParameterWidgetType::StringWidget, getVolumesArrayName(), FilterParameter::CreatedArray));
  parameters.push_back(FilterParameter::New("Average Euler Angles", "FeatureEulerAnglesArrayName", FilterParameterWidgetType::StringWidget, getFeatureEulerAnglesArrayName(), FilterParameter::CreatedArray));
  parameters.push_back(FilterParameter::New("Average Quaternions", "AvgQuatsArrayName", FilterParameterWidgetType::StringWidget, getAvgQuatsArrayName(), FilterParameter::CreatedArray));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setMaxIterations( reader->readValue("MaxIterations", getMaxIterations()) );
  setInputStatsArrayPath(reader->readDataArrayPath("InputStatsArrayPath", getInputStatsArrayPath() ) );
  setCrystalStructuresArrayPath(reader->readDataArrayPath("CrystalStructuresArrayPath", getCrystalStructuresArrayPath() ) );
  setPhaseTypesArrayPath(reader->readDataArrayPath("PhaseTypesArrayPath", getPhaseTypesArrayPath() ) );
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath() ) );
  setFeaturePhasesArrayPath(reader->readDataArrayPath("FeaturePhasesArrayPath", getFeaturePhasesArrayPath() ) );
  setSurfaceFeaturesArrayPath(reader->readDataArrayPath("SurfaceFeaturesArrayPath", getSurfaceFeaturesArrayPath() ) );
  setNeighborListArrayPath(reader->readDataArrayPath("NeighborListArrayPath", getNeighborListArrayPath() ) );
  setNumFeaturesArrayPath(reader->readDataArrayPath("NumFeaturesArrayPath", getNumFeaturesArrayPath() ) );
  setSharedSurfaceAreaListArrayPath(reader->readDataArrayPath("SharedSurfaceAreaListArrayPath", getSharedSurfaceAreaListArrayPath() ) );
  setCellEulerAnglesArrayName(reader->readString("CellEulerAnglesArrayName", getCellEulerAnglesArrayName() ) );
  setVolumesArrayName(reader->readString("VolumesArrayName", getVolumesArrayName() ) );
  setFeatureEulerAnglesArrayName(reader->readString("FeatureEulerAnglesArrayName", getFeatureEulerAnglesArrayName() ) );
  setAvgQuatsArrayName(reader->readString("AvgQuatsArrayName", getAvgQuatsArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int MatchCrystallography::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(FilterVersion)
  DREAM3D_FILTER_WRITE_PARAMETER(MaxIterations)
  DREAM3D_FILTER_WRITE_PARAMETER(InputStatsArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(CrystalStructuresArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(PhaseTypesArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(FeaturePhasesArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(FeatureIdsArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(SurfaceFeaturesArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(NeighborListArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(SharedSurfaceAreaListArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(NumFeaturesArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(CellEulerAnglesArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(VolumesArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(FeatureEulerAnglesArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(AvgQuatsArrayName)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::dataCheck()
{
  setErrorCondition(0);
  DataArrayPath tempPath;

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom, AbstractFilter>(this, getFeatureIdsArrayPath().getDataContainerName());

  QVector<size_t> cDims(1, 1);
  // Cell Data
  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this,  getFeatureIdsArrayPath(), cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_FeatureIdsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 3;
  tempPath.update(getFeatureIdsArrayPath().getDataContainerName(), getFeatureIdsArrayPath().getAttributeMatrixName(), getCellEulerAnglesArrayName());
  m_CellEulerAnglesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<FloatArrayType, AbstractFilter>(this, tempPath, 0, cDims);
  if( NULL != m_CellEulerAnglesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_CellEulerAngles = m_CellEulerAnglesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  // Feature Data
  cDims[0] = 1;
  m_FeaturePhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this,  getFeaturePhasesArrayPath(), cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_FeaturePhasesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_FeaturePhases = m_FeaturePhasesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  m_SurfaceFeaturesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>, AbstractFilter>(this,  getSurfaceFeaturesArrayPath(), cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_SurfaceFeaturesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_SurfaceFeatures = m_SurfaceFeaturesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  tempPath.update(getFeaturePhasesArrayPath().getDataContainerName(), getFeaturePhasesArrayPath().getAttributeMatrixName(), getVolumesArrayName());
  m_VolumesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>, AbstractFilter, float>(this,  tempPath, 0, cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_VolumesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_Volumes = m_VolumesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 3;
  tempPath.update(getFeaturePhasesArrayPath().getDataContainerName(), getFeaturePhasesArrayPath().getAttributeMatrixName(), getFeatureEulerAnglesArrayName());
  m_FeatureEulerAnglesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>, AbstractFilter, float>(this,  tempPath, 0, cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_FeatureEulerAnglesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_FeatureEulerAngles = m_FeatureEulerAnglesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 4;
  tempPath.update(getFeaturePhasesArrayPath().getDataContainerName(), getFeaturePhasesArrayPath().getAttributeMatrixName(), getAvgQuatsArrayName());
  m_AvgQuatsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>, AbstractFilter, float>(this,  tempPath, 0, cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_AvgQuatsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_AvgQuats = m_AvgQuatsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  cDims[0] = 1;
  // Now we are going to get a "Pointer" to the NeighborList object out of the DataContainer
  m_NeighborList = getDataContainerArray()->getPrereqArrayFromPath<NeighborList<int>, AbstractFilter>(this, getNeighborListArrayPath(), cDims);
  m_SharedSurfaceAreaList = getDataContainerArray()->getPrereqArrayFromPath<NeighborList<float>, AbstractFilter>(this, getSharedSurfaceAreaListArrayPath(), cDims);

  // Ensemble Data
  m_NumFeaturesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this,  getNumFeaturesArrayPath(), cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_NumFeaturesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_NumFeatures = m_NumFeaturesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  m_StatsDataArray = getDataContainerArray()->getPrereqArrayFromPath<StatsDataArray, AbstractFilter>(this, getInputStatsArrayPath(), cDims);
  if(m_StatsDataArray.lock() == NULL)
  {
    QString ss = QObject::tr("Statistics array is not initialized correctly. The path is %1").arg(getInputStatsArrayPath().serialize());
    setErrorCondition(-308);
    notifyErrorMessage(getHumanLabel(), ss, -308);
  }

  m_CrystalStructuresPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<unsigned int>, AbstractFilter>(this,  getCrystalStructuresArrayPath(), cDims);
  if( NULL != m_CrystalStructuresPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  {m_CrystalStructures = m_CrystalStructuresPtr.lock()->getPointer(0);}

  m_PhaseTypesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<unsigned int>, AbstractFilter>(this,  getPhaseTypesArrayPath(), cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_PhaseTypesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_PhaseTypes = m_PhaseTypesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  // Now create the output CrystalStructures Array
  tempPath.update(getNumFeaturesArrayPath().getDataContainerName(), getNumFeaturesArrayPath().getAttributeMatrixName(), "CrystalStructures" );
  m_SyntheticCrystalStructuresPtr = getDataContainerArray()->createNonPrereqArrayFromPath<UInt32ArrayType, AbstractFilter>(this, tempPath, true, cDims); /* Assigns the shared_ptr<>(this, tempPath, true, dims); Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_SyntheticCrystalStructuresPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_SyntheticCrystalStructures = m_SyntheticCrystalStructuresPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::execute()
{
  setErrorCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  size_t totalEnsembles = m_CrystalStructuresPtr.lock()->getNumberOfTuples();

  QString ss;
  ss = QObject::tr("Determining Volumes");
  notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
  determine_volumes();
  if (getCancel() == true) { return; }

  ss = QObject::tr("Determining Boundary Areas");
  notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
  determine_boundary_areas();
  if (getCancel() == true) { return; }

  m_SyntheticCrystalStructures[0] = m_CrystalStructures[0];
  for (size_t i = 1; i < totalEnsembles; ++i)
  {
    if (m_PhaseTypes[i] == DREAM3D::PhaseType::PrimaryPhase ||  m_PhaseTypes[i] == DREAM3D::PhaseType::PrecipitatePhase)
    {
      ss = QObject::tr("Initializing Arrays of Phase %1").arg(i);
      notifyStatusMessage(getHumanLabel(), "Initializing Arrays");
      initializeArrays(i);
      if(getErrorCondition() < 0) { return; }
      if (getCancel() == true) { return; }

      ss = QObject::tr("Assigning Eulers to Phase %1").arg(i);
      notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
      assign_eulers(i);
      if(getErrorCondition() < 0) { return; }
      if (getCancel() == true) { return; }

      ss = QObject::tr("Measuring Misorientations of Phase %1").arg(i);
      notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
      measure_misorientations(i);
      if (getCancel() == true) { return; }

      ss = QObject::tr("Matching Crystallography of Phase %1").arg(i);
      notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
      matchCrystallography(i);
      if (getCancel() == true) { return; }
    }

    m_SyntheticCrystalStructures[i] = m_CrystalStructures[i]; // Copy over the crystal structures from the statsfile into the synthetic file
  }

  // If there is an error set this to something negative and also set a message
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::initializeArrays(size_t ensem)
{
  StatsDataArray& statsDataArray = *(m_StatsDataArray.lock());

  if (m_PhaseTypes[ensem] == DREAM3D::PhaseType::PrecipitatePhase)
  {
    PrecipitateStatsData* pp = PrecipitateStatsData::SafePointerDownCast(statsDataArray[ensem].get());
    if (NULL == pp)
    {
      QString ss = QObject::tr("Tried to cast a statsDataArray[%1].get() to a PrecipitateStatsData* "
                               "pointer but this resulted in a NULL pointer. The value at m_PhaseTypes[%2] = %3 does not match up "
                               "with the type of pointer stored in the StatsDataArray (PrecipitateStatsData)\n")
                   .arg(ensem).arg(ensem).arg(m_PhaseTypes[ensem]);
      notifyErrorMessage(getHumanLabel(), ss, -666);
      setErrorCondition(-666);
      return;
    }
    actualodf = pp->getODF();
    actualmdf = pp->getMisorientationBins();
  }
  else if (m_PhaseTypes[ensem] == DREAM3D::PhaseType::PrimaryPhase)
  {
    PrimaryStatsData* pp = PrimaryStatsData::SafePointerDownCast(statsDataArray[ensem].get());
    if (NULL == pp)
    {
      QString ss = QObject::tr("Tried to cast a statsDataArray[%1].get() to a PrimaryStatsData* "
                               "pointer but this resulted in a NULL pointer. The value at m_PhaseTypes[%2] = %3 does not match up "
                               "with the type of pointer stored in the StatsDataArray (PrimaryStatsData)\n")
                   .arg(ensem).arg(ensem).arg(m_PhaseTypes[ensem]);
      notifyErrorMessage(getHumanLabel(), ss, -666);
      setErrorCondition(-666);
      return;
    }
    actualodf = pp->getODF();
    actualmdf = pp->getMisorientationBins();
  }
  else
  {
    setErrorCondition(-55000);
    QString ss;
    ss = QObject::tr("Improper phase type (%1) for matching crystallography").arg(m_PhaseTypes[ensem]);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  simodf = FloatArrayType::CreateArray(actualodf->getSize(), DREAM3D::StringConstants::ODF);
  simmdf = FloatArrayType::CreateArray(actualmdf->getSize(), DREAM3D::StringConstants::MisorientationBins);
  for (size_t j = 0; j < simodf->getSize(); j++)
  {
    simodf->setValue(j, 0.0);
  }
  for (size_t j = 0; j < simmdf->getSize(); j++)
  {
    simmdf->setValue(j, 0.0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::determine_volumes()
{
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_FeatureIdsArrayPath.getDataContainerName());

  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  size_t totalFeatures = m_FeaturePhasesPtr.lock()->getNumberOfTuples();
  size_t totalEnsembles = m_CrystalStructuresPtr.lock()->getNumberOfTuples();

  unbiasedvol.resize(totalEnsembles);
  for (size_t i = 1; i < totalFeatures; i++)
  {
    m_Volumes[i] = 0.0;
  }
  for (size_t i = 0; i < totalPoints; i++)
  {
    m_Volumes[m_FeatureIds[i]]++;
  }
  float res_scalar = m->getGeometryAs<ImageGeom>()->getXRes() * m->getGeometryAs<ImageGeom>()->getYRes() * m->getGeometryAs<ImageGeom>()->getZRes();
  for (size_t i = 1; i < totalFeatures; i++)
  {
    m_Volumes[i] = m_Volumes[i] * res_scalar;
    if(m_SurfaceFeatures[i] == false)
    {
      unbiasedvol[m_FeaturePhases[i]] = unbiasedvol[m_FeaturePhases[i]] + m_Volumes[i];
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::determine_boundary_areas()
{
  NeighborList<int32_t>& neighborlist = *(m_NeighborList.lock());
  NeighborList<float>& neighborsurfacearealist = *(m_SharedSurfaceAreaList.lock() );

  size_t totalFeatures = m_FeaturePhasesPtr.lock()->getNumberOfTuples();
  size_t totalEnsembles = m_CrystalStructuresPtr.lock()->getNumberOfTuples();

  m_TotalSurfaceArea.assign(totalEnsembles, 0.0f);

  int32_t phase1 = 0, phase2 = 0;
  for (size_t i = 1; i < totalFeatures; i++)
  {
    phase1 = m_FeaturePhases[i];
    size_t size = 0;
    if (neighborlist[i].size() != 0 && neighborsurfacearealist[i].size() == neighborlist[i].size())
    {
      size = neighborlist[i].size();
    }

    for (size_t j = 0; j < size; j++)
    {
      int32_t nname = neighborlist[i][j];
      float neighsurfarea = neighborsurfacearealist[i][j];
      phase2 = m_FeaturePhases[nname];
      if (phase1 == phase2)
      {
        m_TotalSurfaceArea[phase1] = m_TotalSurfaceArea[phase1] + neighsurfarea;
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::assign_eulers(size_t ensem)
{
  DREAM3D_RANDOMNG_NEW()

  int32_t numbins = 0;
  QuatF* avgQuats = reinterpret_cast<QuatF*>(m_AvgQuats);
  float random = 0.0f;
  int32_t choose = 0, phase = 0;

  size_t totalFeatures = m_FeaturePhasesPtr.lock()->getNumberOfTuples();

  CubicOps cOps;
  HexagonalOps hOps;

  for (size_t i = 1; i < totalFeatures; i++)
  {
    phase = m_FeaturePhases[i];
    if (phase == ensem)
    {
      random = static_cast<float>( rg.genrand_res53() );

      if ( Ebsd::CrystalStructure::Cubic_High == m_CrystalStructures[phase] ) { numbins = cOps.getODFSize(); };
      if ( Ebsd::CrystalStructure::Hexagonal_High == m_CrystalStructures[phase] ) { numbins = hOps.getODFSize(); }

      // If we get to here and numbins is still zero, then an unknown or unsupported crystal structure
      // was used, so we bail
      if (numbins == 0)
      {
        QString ss = QObject::tr("Unkown crystal structure (%1) for phase %2").arg(m_CrystalStructures[phase]).arg(phase);
        notifyErrorMessage(getHumanLabel(), ss, -666);
        setErrorCondition(-666);
        return;
      }

      choose = pick_euler(random, numbins);

      FOrientArrayType eulers = m_OrientationOps[m_CrystalStructures[ensem]]->determineEulerAngles(choose);
      eulers = m_OrientationOps[m_CrystalStructures[ensem]]->randomizeEulerAngles(eulers);
      m_FeatureEulerAngles[3 * i] = eulers[0];
      m_FeatureEulerAngles[3 * i + 1] = eulers[1];
      m_FeatureEulerAngles[3 * i + 2] = eulers[2];

      FOrientArrayType q(4, 0.0);
      FOrientTransformsType::eu2qu(FOrientArrayType(&(m_FeatureEulerAngles[3 * i]), 3), q);
      QuaternionMathF::Copy(q.toQuaternion(), avgQuats[i]);
      if (m_SurfaceFeatures[i] == false)
      {
        simodf->setValue(choose, (simodf->getValue(choose) + m_Volumes[i] / unbiasedvol[ensem]));
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t MatchCrystallography::pick_euler(float random, int32_t numbins)
{
  int32_t choose = 0;
  float totaldensity = 0.0f;

  for (int32_t j = 0; j < numbins; j++)
  {
    float density = actualodf->getValue(j);
    float td1 = totaldensity;
    totaldensity = totaldensity + density;
    if (random < totaldensity && random >= td1) { choose = j; break; }
  }
  return choose;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::MC_LoopBody1(int32_t feature, size_t ensem, size_t j, float neighsurfarea, uint32_t sym, QuatF& q1, QuatF& q2)
{
  float w = 0.0f;
  float n1 = 0.0f, n2 = 0.0f, n3 = 0.0f;
  float curmiso1 = 0.0f, curmiso2 = 0.0f, curmiso3 = 0.0f;
  size_t curmisobin = 0, newmisobin = 0;


  curmiso1 = m_MisorientationLists[feature][3 * j];
  curmiso2 = m_MisorientationLists[feature][3 * j + 1];
  curmiso3 = m_MisorientationLists[feature][3 * j + 2];

  FOrientArrayType rod(curmiso1, curmiso2, curmiso3, 0.0f);
  float mag = sqrt(curmiso1*curmiso1 + curmiso2*curmiso2 + curmiso3*curmiso3);
  if(mag == 0.0f)
  {
    rod[3] = std::numeric_limits<float>::infinity();
  }
  else
  {
    rod[3] = mag;
    rod[0] = rod[0]/rod[3];
    rod[1] = rod[1]/rod[3];
    rod[2] = rod[2]/rod[3];
  }

  curmisobin = m_OrientationOps[sym]->getMisoBin(rod);
  w = m_OrientationOps[sym]->getMisoQuat(q1, q2, n1, n2, n3);

  FOrientTransformsType::ax2ro(FOrientArrayType(n1, n2, n3, w), rod);
  newmisobin = m_OrientationOps[sym]->getMisoBin(rod);
  mdfchange = mdfchange
              + (((actualmdf->getValue(curmisobin) - simmdf->getValue(curmisobin)) * (actualmdf->getValue(curmisobin) - simmdf->getValue(curmisobin)))
                 - ((actualmdf->getValue(curmisobin) - (simmdf->getValue(curmisobin) - (neighsurfarea / m_TotalSurfaceArea[ensem])))
                    * (actualmdf->getValue(curmisobin) - (simmdf->getValue(curmisobin) - (neighsurfarea / m_TotalSurfaceArea[ensem])))));
  mdfchange = mdfchange
              + (((actualmdf->getValue(newmisobin) - simmdf->getValue(newmisobin)) * (actualmdf->getValue(newmisobin) - simmdf->getValue(newmisobin)))
                 - ((actualmdf->getValue(newmisobin) - (simmdf->getValue(newmisobin) + (neighsurfarea / m_TotalSurfaceArea[ensem])))
                    * (actualmdf->getValue(newmisobin) - (simmdf->getValue(newmisobin) + (neighsurfarea / m_TotalSurfaceArea[ensem])))));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::MC_LoopBody2(int32_t feature, size_t ensem, size_t j, float neighsurfarea, uint32_t sym, QuatF& q1, QuatF& q2)
{
  float w = 0.0f;
  float n1 = 0.0f, n2 = 0.0f, n3 = 0.0f;
  float curmiso1 = 0.0f, curmiso2 = 0.0f, curmiso3 = 0.0f;
  size_t curmisobin = 0, newmisobin = 0;
  float miso1 = 0.0f, miso2 = 0.0f, miso3 = 0.0f;

  curmiso1 = m_MisorientationLists[feature][3 * j];
  curmiso2 = m_MisorientationLists[feature][3 * j + 1];
  curmiso3 = m_MisorientationLists[feature][3 * j + 2];

  FOrientArrayType rod(curmiso1, curmiso2, curmiso3, 0.0f);
  float mag = sqrt(curmiso1*curmiso1 + curmiso2*curmiso2 + curmiso3*curmiso3);
  if(mag == 0.0f)
  {
    rod[3] = std::numeric_limits<float>::infinity();
  }
  else
  {
    rod[3] = mag;
    rod[0] = rod[0]/rod[3];
    rod[1] = rod[1]/rod[3];
    rod[2] = rod[2]/rod[3];
  }

  curmisobin = m_OrientationOps[sym]->getMisoBin(rod);
  w = m_OrientationOps[sym]->getMisoQuat(q1, q2, n1, n2, n3);

  FOrientTransformsType::ax2ro(FOrientArrayType(n1, n2, n3, w), rod);
  newmisobin = m_OrientationOps[sym]->getMisoBin(rod);
  m_MisorientationLists[feature][3 * j] = miso1;
  m_MisorientationLists[feature][3 * j + 1] = miso2;
  m_MisorientationLists[feature][3 * j + 2] = miso3;
  simmdf->setValue(curmisobin, (simmdf->getValue(curmisobin) - (neighsurfarea / m_TotalSurfaceArea[ensem])));
  simmdf->setValue(newmisobin, (simmdf->getValue(newmisobin) + (neighsurfarea / m_TotalSurfaceArea[ensem])));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::matchCrystallography(size_t ensem)
{
  // But since a pointer is difficult to use operators with we will now create a
  // reference variable to the pointer with the correct variable name that allows
  // us to use the same syntax as the "vector of vectors"
  NeighborList<int32_t>& neighborlist = *(m_NeighborList.lock());
  NeighborList<float>& neighborsurfacearealist = *(m_SharedSurfaceAreaList.lock() );
  size_t totalPoints = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  size_t totalFeatures = m_FeaturePhasesPtr.lock()->getNumberOfTuples();

  DREAM3D_RANDOMNG_NEW()
  int32_t numbins = 0;
  int32_t iterations = 0, badtrycount = 0;
  float random = 0.0f;
  size_t counter = 0;

  QuatF q1;
  QuatF q2;
  QuatF* avgQuats = reinterpret_cast<QuatF*>(m_AvgQuats);

  float ea1 = 0.0f, ea2 = 0.0f, ea3 = 0.0f;
  float g1ea1 = 0.0f, g1ea2 = 0.0f, g1ea3 = 0.0f, g2ea1 = 0.0f, g2ea2 = 0.0f, g2ea3 = 0.0f;
  int32_t g1odfbin = 0, g2odfbin = 0;
  float deltaerror = 0.0f;
  float currentodferror = 0.0f, currentmdferror = 0.0f;
  int32_t selectedfeature1 = 0, selectedfeature2 = 0;
  iterations = 0;
  badtrycount = 0;
  if ( Ebsd::CrystalStructure::Cubic_High == m_CrystalStructures[ensem]) { numbins = 18 * 18 * 18; }
  if ( Ebsd::CrystalStructure::Hexagonal_High == m_CrystalStructures[ensem]) { numbins = 36 * 36 * 12; }

  uint64_t millis = QDateTime::currentMSecsSinceEpoch();
  uint64_t startMillis = millis;
  int32_t lastIteration = 0;
  while (badtrycount < (m_MaxIterations / 10) && iterations < m_MaxIterations)
  {
    uint64_t currentMillis = QDateTime::currentMSecsSinceEpoch();
    if (currentMillis - millis > 1000)
    {
      QString ss = QObject::tr("Swapping/Switching Orientations Iteration %1/%2").arg(iterations).arg(m_MaxIterations);
      float timeDiff = ((float)iterations / (float)(currentMillis - startMillis));
      float estimatedTime = (float)(m_MaxIterations - iterations) / timeDiff;

      ss += QObject::tr(" || Est. Time Remain: %1 || Iterations/Sec: %2").arg(DREAM3D::convertMillisToHrsMinSecs(estimatedTime)).arg(timeDiff * 1000);
      notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);

      millis = QDateTime::currentMSecsSinceEpoch();
      lastIteration = iterations;
    }
    currentodferror = 0;
    currentmdferror = 0;
    float* actualOdfPtr = actualodf->getPointer(0);
    float* simOdfPtr = simodf->getPointer(0);
    float delta = 0.0f;
    for (int32_t i = 0; i < numbins; i++)
    {
      delta = actualOdfPtr[i] - simOdfPtr[i];
      currentodferror = currentodferror + (delta * delta);
    }
    for (int32_t i = 0; i < (numbins); i++)
    {
      currentmdferror = currentmdferror + ((actualmdf->getValue(i) - simmdf->getValue(i)) * (actualmdf->getValue(i) - simmdf->getValue(i)));
    }
    iterations++;
    badtrycount++;
    random = static_cast<float>( rg.genrand_res53() );

    if (getCancel() == true) { return; }

    if (random < 0.5) // SwapOutOrientation
    {
      counter = 0;
      selectedfeature1 = int32_t(rg.genrand_res53() * totalFeatures);
      while ((m_SurfaceFeatures[selectedfeature1] == true || m_FeaturePhases[selectedfeature1] != static_cast<int32_t>(ensem)) && counter < totalFeatures)
      {
        if (selectedfeature1 >= totalFeatures) { selectedfeature1 = selectedfeature1 - totalFeatures; }
        selectedfeature1++;
        counter++;
      }
      if (counter == totalFeatures)
      {
        badtrycount = 10 * m_NumFeatures[ensem];
      }
      else
      {
        ea1 = m_FeatureEulerAngles[3 * selectedfeature1];
        ea2 = m_FeatureEulerAngles[3 * selectedfeature1 + 1];
        ea3 = m_FeatureEulerAngles[3 * selectedfeature1 + 2];
        FOrientArrayType rod(4, 0.0);
        FOrientTransformsType::eu2ro(FOrientArrayType(&(m_FeatureEulerAngles[3 * selectedfeature1]), 3), rod);

        g1odfbin = m_OrientationOps[m_CrystalStructures[ensem]]->getOdfBin(rod);
        random = static_cast<float>( rg.genrand_res53() );
        int32_t choose = 0;

        choose = pick_euler(random, numbins);

        FOrientArrayType g1ea = m_OrientationOps[m_CrystalStructures[ensem]]->determineEulerAngles(choose);
        g1ea = m_OrientationOps[m_CrystalStructures[ensem]]->randomizeEulerAngles(g1ea);
        FOrientArrayType quat(4, 0.0);
        FOrientTransformsType::eu2qu(g1ea, quat);
        q1 = quat.toQuaternion();

        odfchange = ((actualodf->getValue(choose) - simodf->getValue(choose)) * (actualodf->getValue(choose) - simodf->getValue(choose)))
                    - ((actualodf->getValue(choose) - (simodf->getValue(choose) + (m_Volumes[selectedfeature1] / unbiasedvol[ensem])))
                       * (actualodf->getValue(choose) - (simodf->getValue(choose) + (m_Volumes[selectedfeature1] / unbiasedvol[ensem]))));
        odfchange = odfchange
                    + (((actualodf->getValue(g1odfbin) - simodf->getValue(g1odfbin)) * (actualodf->getValue(g1odfbin) - simodf->getValue(g1odfbin)))
                       - ((actualodf->getValue(g1odfbin) - (simodf->getValue(g1odfbin) - (m_Volumes[selectedfeature1] / unbiasedvol[ensem])))
                          * (actualodf->getValue(g1odfbin) - (simodf->getValue(g1odfbin) - (m_Volumes[selectedfeature1] / unbiasedvol[ensem])))));

        mdfchange = 0;
        size_t size = 0;
        if (neighborlist[selectedfeature1].size() != 0) { size = neighborlist[selectedfeature1].size(); }
        for (size_t j = 0; j < size; j++)
        {
          int32_t neighbor = neighborlist[selectedfeature1][j];
          FOrientTransformsType::eu2qu(FOrientArrayType(&(m_FeatureEulerAngles[3 * neighbor]), 3), quat);
          q1 = quat.toQuaternion();
          float neighsurfarea = neighborsurfacearealist[selectedfeature1][j];
          MC_LoopBody1(selectedfeature1, ensem, j, neighsurfarea, m_CrystalStructures[ensem], q1, q2);
        }

        deltaerror = (odfchange / currentodferror) + (mdfchange / currentmdferror);
        if (deltaerror > 0)
        {
          badtrycount = 0;
          m_FeatureEulerAngles[3 * selectedfeature1] = g1ea1;
          m_FeatureEulerAngles[3 * selectedfeature1 + 1] = g1ea2;
          m_FeatureEulerAngles[3 * selectedfeature1 + 2] = g1ea3;
          QuaternionMathF::Copy(q1, avgQuats[selectedfeature1]);
          simodf->setValue(choose, (simodf->getValue(choose) + (m_Volumes[selectedfeature1] / unbiasedvol[ensem])));
          simodf->setValue(g1odfbin, (simodf->getValue(g1odfbin) - (m_Volumes[selectedfeature1] / unbiasedvol[ensem])));
          size = 0;
          if (neighborlist[selectedfeature1].size() != 0) { size = neighborlist[selectedfeature1].size(); }
          for (size_t j = 0; j < size; j++)
          {
            int neighbor = neighborlist[selectedfeature1][j];
            FOrientArrayType quat(4);
            FOrientTransformsType::eu2qu(FOrientArrayType(&(m_FeatureEulerAngles[3 * neighbor]), 3), quat);
            q2 = quat.toQuaternion();
            float neighsurfarea = neighborsurfacearealist[selectedfeature1][j];
            MC_LoopBody2(selectedfeature1, ensem, j, neighsurfarea, m_CrystalStructures[ensem], q1, q2);
          }
        }
      }
      if (getCancel() == true) { return; }
    }
    else // SwitchOrientation
    {
      counter = 0;
      selectedfeature1 = int32_t(rg.genrand_res53() * totalFeatures);
      while ((m_SurfaceFeatures[selectedfeature1] == true || m_FeaturePhases[selectedfeature1] != static_cast<int32_t>(ensem)) && counter < totalFeatures)
      {
        if (selectedfeature1 >= totalFeatures) { selectedfeature1 = selectedfeature1 - totalFeatures; }
        selectedfeature1++;
        counter++;
      }
      if (counter == totalFeatures)
      {
        badtrycount = 10 * m_NumFeatures[ensem];
      }
      else
      {
        counter = 0;
        selectedfeature2 = int32_t(rg.genrand_res53() * totalFeatures);
        while ((m_SurfaceFeatures[selectedfeature2] == true || m_FeaturePhases[selectedfeature2] != static_cast<int32_t>(ensem) || selectedfeature2 == selectedfeature1) && counter < totalFeatures)
        {
          if (selectedfeature2 >= totalFeatures) { selectedfeature2 = selectedfeature2 - totalFeatures; }
          selectedfeature2++;
          counter++;
        }
        if (counter == totalFeatures)
        {
          badtrycount = 10 * m_NumFeatures[ensem];
        }
        else
        {
          g1ea1 = m_FeatureEulerAngles[3 * selectedfeature1];
          g1ea2 = m_FeatureEulerAngles[3 * selectedfeature1 + 1];
          g1ea3 = m_FeatureEulerAngles[3 * selectedfeature1 + 2];
          g2ea1 = m_FeatureEulerAngles[3 * selectedfeature2];
          g2ea2 = m_FeatureEulerAngles[3 * selectedfeature2 + 1];
          g2ea3 = m_FeatureEulerAngles[3 * selectedfeature2 + 2];
          QuaternionMathF::Copy(q1, avgQuats[selectedfeature1]);
          FOrientArrayType rod(4);
          FOrientTransformsType::eu2ro(FOrientArrayType(&(m_FeatureEulerAngles[3 * selectedfeature1]), 3), rod);

          g1odfbin = m_OrientationOps[m_CrystalStructures[ensem]]->getOdfBin(rod);
          QuaternionMathF::Copy(q1, avgQuats[selectedfeature2]);

          FOrientTransformsType::eu2ro(FOrientArrayType(&(m_FeatureEulerAngles[3 * selectedfeature2]), 3), rod);
          g2odfbin = m_OrientationOps[m_CrystalStructures[ensem]]->getOdfBin(rod);

          odfchange = ((actualodf->getValue(g1odfbin) - simodf->getValue(g1odfbin)) * (actualodf->getValue(g1odfbin) - simodf->getValue(g1odfbin)))
                      - ((actualodf->getValue(g1odfbin)
                          - (simodf->getValue(g1odfbin) - (m_Volumes[selectedfeature1] / unbiasedvol[ensem])
                             + (m_Volumes[selectedfeature2] / unbiasedvol[ensem])))
                         * (actualodf->getValue(g1odfbin)
                            - (simodf->getValue(g1odfbin) - (m_Volumes[selectedfeature1] / unbiasedvol[ensem])
                               + (m_Volumes[selectedfeature2] / unbiasedvol[ensem]))));
          odfchange = odfchange
                      + (((actualodf->getValue(g2odfbin) - simodf->getValue(g2odfbin)) * (actualodf->getValue(g2odfbin) - simodf->getValue(g2odfbin)))
                         - ((actualodf->getValue(g2odfbin)
                             - (simodf->getValue(g2odfbin) - (m_Volumes[selectedfeature2] / unbiasedvol[ensem])
                                + (m_Volumes[selectedfeature1] / unbiasedvol[ensem])))
                            * (actualodf->getValue(g2odfbin)
                               - (simodf->getValue(g2odfbin) - (m_Volumes[selectedfeature2] / unbiasedvol[ensem])
                                  + (m_Volumes[selectedfeature1] / unbiasedvol[ensem])))));

          mdfchange = 0;

          FOrientArrayType quat(4);
          FOrientTransformsType::eu2qu(FOrientArrayType(g2ea1, g2ea2, g2ea3), quat);
          q1 = quat.toQuaternion();

          size_t size = 0;
          if (neighborlist[selectedfeature1].size() != 0) { size = neighborlist[selectedfeature1].size(); }
          for (size_t j = 0; j < size; j++)
          {
            int32_t neighbor = neighborlist[selectedfeature1][j];
            FOrientTransformsType::eu2qu(FOrientArrayType(&(m_FeatureEulerAngles[3 * neighbor]), 3), quat);
            q2 = quat.toQuaternion();
            float neighsurfarea = neighborsurfacearealist[selectedfeature1][j];
            if(neighbor != static_cast<int>(selectedfeature2) )
            {
              MC_LoopBody1(selectedfeature1, ensem, j, neighsurfarea, m_CrystalStructures[ensem], q1, q2);
            }
          }

          FOrientTransformsType::eu2qu(FOrientArrayType(g1ea1, g1ea2, g1ea3), quat);
          q1 = quat.toQuaternion();
          size = 0;
          if (neighborlist[selectedfeature2].size() != 0) { size = neighborlist[selectedfeature2].size(); }
          for (size_t j = 0; j < size; j++)
          {
            size_t neighbor = neighborlist[selectedfeature2][j];

            FOrientTransformsType::eu2qu(FOrientArrayType(&(m_FeatureEulerAngles[3 * neighbor]), 3), quat);
            q2 = quat.toQuaternion();
            float neighsurfarea = neighborsurfacearealist[selectedfeature2][j];
            if (neighbor != selectedfeature1)
            {
              MC_LoopBody1(selectedfeature2, ensem, j, neighsurfarea, m_CrystalStructures[ensem], q1, q2);
            }
          }

          deltaerror = (odfchange / currentodferror) + (mdfchange / currentmdferror);
          if (deltaerror > 0)
          {

            badtrycount = 0;
            m_FeatureEulerAngles[3 * selectedfeature1] = g2ea1;
            m_FeatureEulerAngles[3 * selectedfeature1 + 1] = g2ea2;
            m_FeatureEulerAngles[3 * selectedfeature1 + 2] = g2ea3;
            m_FeatureEulerAngles[3 * selectedfeature2] = g1ea1;
            m_FeatureEulerAngles[3 * selectedfeature2 + 1] = g1ea2;
            m_FeatureEulerAngles[3 * selectedfeature2 + 2] = g1ea3;
            simodf->setValue(g1odfbin, (simodf->getValue(g1odfbin) + (m_Volumes[selectedfeature2] / unbiasedvol[ensem])
                                        - (m_Volumes[selectedfeature1] / unbiasedvol[ensem])));
            simodf->setValue(g2odfbin, (simodf->getValue(g2odfbin) + (m_Volumes[selectedfeature1] / unbiasedvol[ensem])
                                        - (m_Volumes[selectedfeature2] / unbiasedvol[ensem])));

            FOrientTransformsType::eu2qu(FOrientArrayType(g1ea1, g1ea2, g1ea3), quat);
            q1 = quat.toQuaternion();
            QuaternionMathF::Copy(q1, avgQuats[selectedfeature1]);
            size = 0;
            if (neighborlist[selectedfeature1].size() != 0) { size = neighborlist[selectedfeature1].size(); }
            for (size_t j = 0; j < size; j++)
            {
              size_t neighbor = neighborlist[selectedfeature1][j];
              ea1 = m_FeatureEulerAngles[3 * neighbor];
              ea2 = m_FeatureEulerAngles[3 * neighbor + 1];
              ea3 = m_FeatureEulerAngles[3 * neighbor + 2];
              FOrientTransformsType::eu2qu(FOrientArrayType(ea1, ea2, ea3), quat);
              q2 = quat.toQuaternion();
              float neighsurfarea = neighborsurfacearealist[selectedfeature1][j];
              if (neighbor != selectedfeature2)
              {
                MC_LoopBody2(selectedfeature1, ensem, j, neighsurfarea, m_CrystalStructures[ensem], q1, q2);
              }
            }

            FOrientTransformsType::eu2qu(FOrientArrayType(g1ea1, g1ea2, g1ea3), quat);
            q1 = quat.toQuaternion();
            QuaternionMathF::Copy(q1, avgQuats[selectedfeature2]);
            size = 0;
            if (neighborlist[selectedfeature2].size() != 0) { size = neighborlist[selectedfeature2].size(); }
            for (size_t j = 0; j < size; j++)
            {
              size_t neighbor = neighborlist[selectedfeature2][j];
              ea1 = m_FeatureEulerAngles[3 * neighbor];
              ea2 = m_FeatureEulerAngles[3 * neighbor + 1];
              ea3 = m_FeatureEulerAngles[3 * neighbor + 2];
              FOrientTransformsType::eu2qu(FOrientArrayType(ea1, ea2, ea3), quat);
              q2 = quat.toQuaternion();
              float neighsurfarea = neighborsurfacearealist[selectedfeature2][j];
              if (neighbor != selectedfeature1)
              {
                MC_LoopBody2(selectedfeature2, ensem, j, neighsurfarea, m_CrystalStructures[ensem], q1, q2);
              }
            }
          }
        }
      }
    }
    if (getCancel() == true) { return; }
  }

  if (getCancel() == true) { return; }

  for (size_t i = 0; i < totalPoints; i++)
  {
    m_CellEulerAngles[3 * i] = m_FeatureEulerAngles[3 * m_FeatureIds[i]];
    m_CellEulerAngles[3 * i + 1] = m_FeatureEulerAngles[3 * m_FeatureIds[i] + 1];
    m_CellEulerAngles[3 * i + 2] = m_FeatureEulerAngles[3 * m_FeatureIds[i] + 2];
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MatchCrystallography::measure_misorientations(size_t ensem)
{
  // But since a pointer is difficult to use operators with we will now create a
  // reference variable to the pointer with the correct variable name that allows
  // us to use the same syntax as the "vector of vectors"
  NeighborList<int32_t>& neighborlist = *(m_NeighborList.lock());
  NeighborList<float>& neighborsurfacearealist = *(m_SharedSurfaceAreaList.lock() );
  size_t totalFeatures = m_FeaturePhasesPtr.lock()->getNumberOfTuples();

  float w = 0.0f;
  float n1 = 0.0f, n2 = 0.0f, n3 = 0.0f;
  QuatF q1 = QuaternionMathF::New();
  QuatF q2 = QuaternionMathF::New();
  QuatF* avgQuats = reinterpret_cast<QuatF*>(m_AvgQuats);

  uint32_t crys1 = 0;
  int32_t mbin = 0;

  m_MisorientationLists.resize(totalFeatures);

  for (size_t i = 1; i < totalFeatures; i++)
  {
    if (m_FeaturePhases[i] == ensem)
    {
      if (m_MisorientationLists[i].size() != 0)
      {
        m_MisorientationLists[i].clear();
      }
      if (neighborlist[i].size() != 0)
      {
        m_MisorientationLists[i].assign(neighborlist[i].size() * 3, 0.0f);
      }
      QuaternionMathF::Copy(avgQuats[i], q1);
      crys1 = m_CrystalStructures[ensem];
      size_t size = 0;
      if (neighborlist[i].size() != 0 && neighborsurfacearealist[i].size() == neighborlist[i].size())
      {
        size = neighborlist[i].size();
      }

      for (size_t j = 0; j < size; j++)
      {
        int32_t nname = neighborlist[i][j];
        if (m_FeaturePhases[nname] == ensem)
        {
          w = 10000.0f;
          float neighsurfarea = neighborsurfacearealist[i][j];
          QuaternionMathF::Copy(avgQuats[nname], q2);
          w = m_OrientationOps[crys1]->getMisoQuat(q1, q2, n1, n2, n3);
          FOrientArrayType rod(4);
          FOrientTransformsType::ax2ro(FOrientArrayType(n1, n2, n3, w), rod);
          m_MisorientationLists[i][3 * j] = rod[0];
          m_MisorientationLists[i][3 * j + 1] = rod[1];
          m_MisorientationLists[i][3 * j + 2] = rod[2];
          mbin = m_OrientationOps[crys1]->getMisoBin(rod);
          if (m_SurfaceFeatures[i] == false && (nname > static_cast<int32_t>(i) || m_SurfaceFeatures[nname] == true))
          {
            simmdf->setValue(mbin, (simmdf->getValue(mbin) + (neighsurfarea / m_TotalSurfaceArea[m_FeaturePhases[i]])));
          }
        }
        else
        {
          m_MisorientationLists[i][3 * j] = -100;
          m_MisorientationLists[i][3 * j + 1] = -100;
          m_MisorientationLists[i][3 * j + 2] = -100;
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer MatchCrystallography::newFilterInstance(bool copyFilterParameters)
{
  MatchCrystallography::Pointer filter = MatchCrystallography::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString MatchCrystallography::getCompiledLibraryName()
{ return SyntheticBuildingConstants::SyntheticBuildingBaseName; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString MatchCrystallography::getGroupName()
{return DREAM3D::FilterGroups::SyntheticBuildingFilters;}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString MatchCrystallography::getSubGroupName()
{ return DREAM3D::FilterSubGroups::CrystallographyFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString MatchCrystallography::getHumanLabel()
{return "Match Crystallography";}
