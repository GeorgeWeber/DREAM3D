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


#include "VisualizeGBCDPoleFigure.h"

#include <QtCore/QDir>

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Utilities/DREAM3DEndian.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/ChoiceFilterParameter.h"
#include "DREAM3DLib/FilterParameters/FileSystemFilterParameter.h"
#include "DREAM3DLib/FilterParameters/SeparatorFilterParameter.h"
#include "DREAM3DLib/Utilities/DREAM3DEndian.h"

#include "OrientationLib/OrientationMath/OrientationMath.h"

#include "IO/IOConstants.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VisualizeGBCDPoleFigure::VisualizeGBCDPoleFigure() :
  AbstractFilter(),
  m_OutputFile(""),
  m_CrystalStructure(Ebsd::CrystalStructure::UnknownCrystalStructure),
  m_GBCDArrayPath(DREAM3D::Defaults::TriangleDataContainerName, DREAM3D::Defaults::FaceEnsembleAttributeMatrixName, DREAM3D::EnsembleData::GBCD),
  m_GBCD(NULL)
{
  m_MisorientationRotation.angle = 0.0f;
  m_MisorientationRotation.h = 0.0f;
  m_MisorientationRotation.k = 0.0f;
  m_MisorientationRotation.l = 0.0f;

  m_OrientationOps = SpaceGroupOps::getOrientationOpsQVector();

  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VisualizeGBCDPoleFigure::~VisualizeGBCDPoleFigure()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VisualizeGBCDPoleFigure::setupFilterParameters()
{
  FilterParameterVector parameters;

  {
    ChoiceFilterParameter::Pointer option = ChoiceFilterParameter::New();
    option->setHumanLabel("Crystal Structure");
    option->setPropertyName("CrystalStructure");
    option->setWidgetType(FilterParameterWidgetType::ChoiceWidget);
    //option->setValueType("unsigned int");
    QVector<QString> choices;
    choices.push_back("Hexagonal-High 6/mmm");
    choices.push_back("Cubic-High m-3m");
    //choices.push_back("Hexagonal-Low 6/m");
    //choices.push_back("Cubic-Low m-3 (Tetrahedral)");
    //choices.push_back("TriClinic -1");
    //choices.push_back("Monoclinic 2/m");
    //choices.push_back("OrthoRhombic mmm");
    //choices.push_back("Tetragonal-Low 4/m");
    //choices.push_back("Tetragonal-High 4/mmm");
    //choices.push_back("Trigonal-Low -3");
    //choices.push_back("Trigonal-High -3m");
    option->setChoices(choices);
    option->setCategory(FilterParameter::Parameter);
    parameters.push_back(option);
  }
  parameters.push_back(FilterParameter::New("Misorientation Axis-Angle", "MisorientationRotation", FilterParameterWidgetType::AxisAngleWidget, getMisorientationRotation(), FilterParameter::Parameter));
  parameters.push_back(FileSystemFilterParameter::New("Output Regular Grid VTK File", "OutputFile", FilterParameterWidgetType::OutputFileWidget, getOutputFile(), FilterParameter::Parameter, "", "*.vtk", "VTK File"));
  parameters.push_back(SeparatorFilterParameter::New("Face Ensemble Data", FilterParameter::RequiredArray));
  parameters.push_back(FilterParameter::New("GBCD", "GBCDArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getGBCDArrayPath(), FilterParameter::RequiredArray, ""));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VisualizeGBCDPoleFigure::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setGBCDArrayPath(reader->readDataArrayPath("GBCDArrayPath", getGBCDArrayPath() ) );
  setOutputFile( reader->readString( "OutputFile", getOutputFile() ) );
  setMisorientationRotation(reader->readAxisAngle("MisorientationRotation", getMisorientationRotation(), -1) );
  setCrystalStructure(reader->readValue("CrystalStructure", getCrystalStructure() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int VisualizeGBCDPoleFigure::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(FilterVersion)
  DREAM3D_FILTER_WRITE_PARAMETER(GBCDArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(OutputFile)
  DREAM3D_FILTER_WRITE_PARAMETER(MisorientationRotation)
  DREAM3D_FILTER_WRITE_PARAMETER(CrystalStructure)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VisualizeGBCDPoleFigure::dataCheck()
{
  setErrorCondition(0);

  getDataContainerArray()->getPrereqGeometryFromDataContainer<TriangleGeom, AbstractFilter>(this, getGBCDArrayPath().getDataContainerName());

  if (getCrystalStructure() == Ebsd::CrystalStructure::UnknownCrystalStructure)
  {
    QString ss = QObject::tr("A valid crystal structure must be set").arg(ClassName());
    notifyErrorMessage(getHumanLabel(), ss, -1);
    setErrorCondition(-381);
  }

  if (getOutputFile().isEmpty() == true)
  {
    QString ss = QObject::tr( "The output file must be set");
    notifyErrorMessage(getHumanLabel(), ss, -1000);
    setErrorCondition(-1);
  }

  QFileInfo fi(getOutputFile());
  QDir parentPath = fi.path();
  if (parentPath.exists() == false)
  {
    QString ss = QObject::tr( "The directory path for the output file does not exist. DREAM.3D will attempt to create this path during execution of the filter");
    notifyWarningMessage(getHumanLabel(), ss, -1);
  }

  if (fi.suffix().compare("") == 0)
  {
    setOutputFile(getOutputFile().append(".vtk"));
  }

  IDataArray::Pointer tmpGBCDPtr = getDataContainerArray()->getPrereqIDataArrayFromPath<IDataArray, AbstractFilter>(this, getGBCDArrayPath());
  if(getErrorCondition() < 0) { return; }

  if (NULL != tmpGBCDPtr.get())
  {
    QVector<size_t> cDims = tmpGBCDPtr->getComponentDimensions();
    m_GBCDPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<double>, AbstractFilter>(this, getGBCDArrayPath(), cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
    if( NULL != m_GBCDPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
    { m_GBCD = m_GBCDPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void VisualizeGBCDPoleFigure::preflight()
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
void VisualizeGBCDPoleFigure::execute()
{
  setErrorCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  // Make sure any directory path is also available as the user may have just typed
  // in a path without actually creating the full path
  QFileInfo fi(getOutputFile());

  QDir dir(fi.path());
  if(!dir.mkpath("."))
  {
    QString ss;
    ss = QObject::tr("Error creating parent path '%1'").arg(dir.path());
    notifyErrorMessage(getHumanLabel(), ss, -1);
    setErrorCondition(-1);
    return;
  }

  QFile file(getOutputFile());
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QString ss = QObject::tr("Error opening output file '%1'").arg(getOutputFile());
    setErrorCondition(-100);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  FloatArrayType::Pointer gbcdDeltasArray = FloatArrayType::CreateArray(5, "GBCDDeltas");
  gbcdDeltasArray->initializeWithZeros();

  FloatArrayType::Pointer gbcdLimitsArray = FloatArrayType::CreateArray(10, "GBCDLimits");
  gbcdLimitsArray->initializeWithZeros();

  Int32ArrayType::Pointer gbcdSizesArray = Int32ArrayType::CreateArray(5, "GBCDSizes");
  gbcdSizesArray->initializeWithZeros();

  float* gbcdDeltas = gbcdDeltasArray->getPointer(0);
  int* gbcdSizes = gbcdSizesArray->getPointer(0);
  float* gbcdLimits = gbcdLimitsArray->getPointer(0);

  // Original Ranges from Dave R.
  //m_GBCDlimits[0] = 0.0f;
  //m_GBCDlimits[1] = cosf(1.0f*m_pi);
  //m_GBCDlimits[2] = 0.0f;
  //m_GBCDlimits[3] = 0.0f;
  //m_GBCDlimits[4] = cosf(1.0f*m_pi);
  //m_GBCDlimits[5] = 2.0f*m_pi;
  //m_GBCDlimits[6] = cosf(0.0f);
  //m_GBCDlimits[7] = 2.0f*m_pi;
  //m_GBCDlimits[8] = 2.0f*m_pi;
  //m_GBCDlimits[9] = cosf(0.0f);

  // Greg R. Ranges
  gbcdLimits[0] = 0.0f;
  gbcdLimits[1] = 0.0f;
  gbcdLimits[2] = 0.0f;
  gbcdLimits[3] = 0.0f;
  gbcdLimits[4] = 0.0f;
  gbcdLimits[5] = DREAM3D::Constants::k_PiOver2;
  gbcdLimits[6] = 1.0f;
  gbcdLimits[7] = DREAM3D::Constants::k_PiOver2;
  gbcdLimits[8] = 1.0f;
  gbcdLimits[9] = DREAM3D::Constants::k_2Pi;

  // reset the 3rd and 4th dimensions using the square grid approach
  gbcdLimits[3] = -sqrtf(DREAM3D::Constants::k_PiOver2);
  gbcdLimits[4] = -sqrtf(DREAM3D::Constants::k_PiOver2);
  gbcdLimits[8] = sqrtf(DREAM3D::Constants::k_PiOver2);
  gbcdLimits[9] = sqrtf(DREAM3D::Constants::k_PiOver2);

  // get num components of GBCD
  QVector<size_t> cDims = m_GBCDPtr.lock()->getComponentDimensions();

  gbcdSizes[0] = cDims[0];
  gbcdSizes[1] = cDims[1];
  gbcdSizes[2] = cDims[2];
  gbcdSizes[3] = cDims[3];
  gbcdSizes[4] = cDims[4];

  gbcdDeltas[0] = (gbcdLimits[5] - gbcdLimits[0]) / float(gbcdSizes[0]);
  gbcdDeltas[1] = (gbcdLimits[6] - gbcdLimits[1]) / float(gbcdSizes[1]);
  gbcdDeltas[2] = (gbcdLimits[7] - gbcdLimits[2]) / float(gbcdSizes[2]);
  gbcdDeltas[3] = (gbcdLimits[8] - gbcdLimits[3]) / float(gbcdSizes[3]);
  gbcdDeltas[4] = (gbcdLimits[9] - gbcdLimits[4]) / float(gbcdSizes[4]);

  float vec[3] = { 0.0f, 0.0f, 0.0f };
  float vec2[3] = { 0.0f, 0.0f, 0.0f };
  float rotNormal[3] = { 0.0f, 0.0f, 0.0f };
  float rotNormal2[3] = { 0.0f, 0.0f, 0.0f };
  float sqCoord[2] = { 0.0f, 0.0f };
  float dg[3][3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
  float dgt[3][3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
  float dg1[3][3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
  float dg2[3][3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
  float sym1[3][3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
  float sym2[3][3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
  float sym2t[3][3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
  float mis_euler1[3] = { 0.0f, 0.0f, 0.0f };

  float misAngle = m_MisorientationRotation.angle * DREAM3D::Constants::k_PiOver180;
  // convert axis angle to matrix representation of misorientation
  FOrientArrayType om(9, 0.0f);
  FOrientTransformsType::ax2om(FOrientArrayType( m_MisorientationRotation.h, m_MisorientationRotation.k, m_MisorientationRotation.l, misAngle), om);
  om.toGMatrix(dg);

  // take inverse of misorientation variable to use for switching symmetry
  MatrixMath::Transpose3x3(dg, dgt);

  // Get our SpaceGroupOps pointer for the selected crystal structure
  SpaceGroupOps::Pointer orientOps = m_OrientationOps[m_CrystalStructure];

  // get number of symmetry operators
  int32_t n_sym = orientOps->getNumSymOps();

  int32_t xpoints = 100;
  int32_t ypoints = 100;
  int32_t zpoints = 1;
  int32_t xpointshalf = xpoints / 2;
  int32_t ypointshalf = ypoints / 2;
  float xres = 2.0f / float(xpoints);
  float yres = 2.0f / float(ypoints);
  float zres = (xres + yres) / 2.0;
  float x = 0.0f, y = 0.0f;
  float sum = 0;
  int32_t count = 0;
  bool nhCheck = false;
  int32_t hemisphere = 0;

  int32_t shift1 = gbcdSizes[0];
  int32_t shift2 = gbcdSizes[0] * gbcdSizes[1];
  int32_t shift3 = gbcdSizes[0] * gbcdSizes[1] * gbcdSizes[2];
  int32_t shift4 = gbcdSizes[0] * gbcdSizes[1] * gbcdSizes[2] * gbcdSizes[3];

  QVector<size_t> dims(1, 1);
  DoubleArrayType::Pointer poleFigureArray = DoubleArrayType::NullPointer();
  poleFigureArray = DoubleArrayType::CreateArray(xpoints * ypoints, dims, "PoleFigure");
  poleFigureArray->initializeWithZeros();
  double* poleFigure = poleFigureArray->getPointer(0);

  for (int32_t k = 0; k < ypoints; k++)
  {
    for (int32_t l = 0; l < xpoints; l++)
    {
      // get (x,y) for stereographic projection pixel
      x = float(l - xpointshalf) * xres + (xres / 2.0);
      y = float(k - ypointshalf) * yres + (yres / 2.0);
      if ((x * x + y * y) <= 1.0)
      {
        sum = 0.0f;
        count = 0;
        vec[2] = -((x * x + y * y) - 1) / ((x * x + y * y) + 1);
        vec[0] = x * (1 + vec[2]);
        vec[1] = y * (1 + vec[2]);
        MatrixMath::Multiply3x3with3x1(dgt, vec, vec2);

        // Loop over all the symetry operators in the given cystal symmetry
        for (int32_t i = 0; i < n_sym; i++)
        {
          //get symmetry operator1
          orientOps->getMatSymOp(i, sym1);
          for (int32_t j = 0; j < n_sym; j++)
          {
            // get symmetry operator2
            orientOps->getMatSymOp(j, sym2);
            MatrixMath::Transpose3x3(sym2, sym2t);
            // calculate symmetric misorientation
            MatrixMath::Multiply3x3with3x3(dg, sym2t, dg1);
            MatrixMath::Multiply3x3with3x3(sym1, dg1, dg2);
            // convert to euler angle
            FOrientArrayType eu(mis_euler1, 3);
            FOrientTransformsType::om2eu(FOrientArrayType(dg2), eu);
            if (mis_euler1[0] < DREAM3D::Constants::k_PiOver2 && mis_euler1[1] < DREAM3D::Constants::k_PiOver2 && mis_euler1[2] < DREAM3D::Constants::k_PiOver2)
            {
              mis_euler1[1] = cosf(mis_euler1[1]);
              // find bins in GBCD
              int32_t location1 = int32_t((mis_euler1[0] - gbcdLimits[0]) / gbcdDeltas[0]);
              int32_t location2 = int32_t((mis_euler1[1] - gbcdLimits[1]) / gbcdDeltas[1]);
              int32_t location3 = int32_t((mis_euler1[2] - gbcdLimits[2]) / gbcdDeltas[2]);
              //find symmetric poles using the first symmetry operator
              MatrixMath::Multiply3x3with3x1(sym1, vec, rotNormal);
              //get coordinates in square projection of crystal normal parallel to boundary normal
              nhCheck = getSquareCoord(rotNormal, sqCoord);
              // Note the switch to have theta in the 4 slot and cos(Phi) int he 3 slot
              int32_t location4 = int32_t((sqCoord[0] - gbcdLimits[3]) / gbcdDeltas[3]);
              int32_t location5 = int32_t((sqCoord[1] - gbcdLimits[4]) / gbcdDeltas[4]);
              if (location1 >= 0 && location2 >= 0 && location3 >= 0 && location4 >= 0 && location5 >= 0 &&
                  location1 < gbcdSizes[0] && location2 < gbcdSizes[1] && location3 < gbcdSizes[2] && location4 < gbcdSizes[3] && location5 < gbcdSizes[4])
              {
                hemisphere = 0;
                if (nhCheck == false) { hemisphere = 1; }
                sum += m_GBCD[2 * ((location5 * shift4) + (location4 * shift3) + (location3 * shift2) + (location2 * shift1) + location1) + hemisphere];
                count++;
              }
            }

            // again in second crystal reference frame
            // calculate symmetric misorientation
            MatrixMath::Multiply3x3with3x3(dgt, sym2, dg1);
            MatrixMath::Multiply3x3with3x3(sym1, dg1, dg2);
            // convert to euler angle
            FOrientTransformsType::om2eu(FOrientArrayType(dg2), eu);
            if (mis_euler1[0] < DREAM3D::Constants::k_PiOver2 && mis_euler1[1] < DREAM3D::Constants::k_PiOver2 && mis_euler1[2] < DREAM3D::Constants::k_PiOver2)
            {
              mis_euler1[1] = cosf(mis_euler1[1]);
              // find bins in GBCD
              int32_t location1 = int32_t((mis_euler1[0] - gbcdLimits[0]) / gbcdDeltas[0]);
              int32_t location2 = int32_t((mis_euler1[1] - gbcdLimits[1]) / gbcdDeltas[1]);
              int32_t location3 = int32_t((mis_euler1[2] - gbcdLimits[2]) / gbcdDeltas[2]);
              // find symmetric poles using the first symmetry operator
              MatrixMath::Multiply3x3with3x1(sym1, vec2, rotNormal2);
              // get coordinates in square projection of crystal normal parallel to boundary normal
              nhCheck = getSquareCoord(rotNormal2, sqCoord);
              // Note the switch to have theta in the 4 slot and cos(Phi) int he 3 slot
              int32_t location4 = int32_t((sqCoord[0] - gbcdLimits[3]) / gbcdDeltas[3]);
              int32_t location5 = int32_t((sqCoord[1] - gbcdLimits[4]) / gbcdDeltas[4]);
              if (location1 >= 0 && location2 >= 0 && location3 >= 0 && location4 >= 0 && location5 >= 0 &&
                  location1 < gbcdSizes[0] && location2 < gbcdSizes[1] && location3 < gbcdSizes[2] && location4 < gbcdSizes[3] && location5 < gbcdSizes[4])
              {
                hemisphere = 0;
                if (nhCheck == false) { hemisphere = 1; }
                sum += m_GBCD[2 * ((location5 * shift4) + (location4 * shift3) + (location3 * shift2) + (location2 * shift1) + location1) + hemisphere];
                count++;
              }
            }
          }
        }
        if (count > 0)
        {
          poleFigure[(k * xpoints) + l] = sum / float(count);
        }
      }
    }
  }

  FILE* f = NULL;
  f = fopen(m_OutputFile.toLatin1().data(), "wb");
  if (NULL == f)
  {
    QString ss = QObject::tr("Error opening output file '%1'").arg(m_OutputFile);
    setErrorCondition(-1);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  // Write the correct header
  fprintf(f, "# vtk DataFile Version 2.0\n");
  fprintf(f, "data set from DREAM3D\n");
  fprintf(f, "BINARY");
  fprintf(f, "\n");
  fprintf(f, "DATASET RECTILINEAR_GRID\n");
  fprintf(f, "DIMENSIONS %d %d %d\n", xpoints + 1, ypoints + 1, zpoints + 1);

  // Write the Coords
  writeCoords(f, "X_COORDINATES", "float", xpoints + 1, (-float(xpoints)*xres / 2.0f), xres);
  writeCoords(f, "Y_COORDINATES", "float", ypoints + 1, (-float(ypoints)*yres / 2.0f), yres);
  writeCoords(f, "Z_COORDINATES", "float", zpoints + 1, (-float(zpoints)*zres / 2.0f), zres);

  int32_t total = xpoints * ypoints * zpoints;
  fprintf(f, "CELL_DATA %d\n", total);

  fprintf(f, "SCALARS %s %s 1\n", "Intensity", "float");
  fprintf(f, "LOOKUP_TABLE default\n");
  {
    float* gn = new float[total];
    float t;
    count = 0;
    for (int32_t j = 0; j < ypoints; j++)
    {
      for (int32_t i = 0; i < xpoints; i++)
      {
        t = float(poleFigure[(j * xpoints) + i]);
        DREAM3D::Endian::FromSystemToBig::convert(t);
        gn[count] = t;
        count++;
      }
    }
    size_t totalWritten = fwrite(gn, sizeof(float), (total), f);
    delete[] gn;
    if (totalWritten != (total))
    {
      QString ss = QObject::tr("Error writing binary VTK data to file '%1'").arg(m_OutputFile);
      setErrorCondition(-1);
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      fclose(f);
      return;
    }
  }
  fclose(f);

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool VisualizeGBCDPoleFigure::getSquareCoord(float* xstl1_norm1, float* sqCoord)
{
  bool nhCheck = false;
  float adjust = 1.0;
  if (xstl1_norm1[2] >= 0.0)
  {
    adjust = -1.0;
    nhCheck = true;
  }
  if (fabsf(xstl1_norm1[0]) >= fabsf(xstl1_norm1[1]))
  {
    sqCoord[0] = (xstl1_norm1[0] / fabsf(xstl1_norm1[0])) * sqrtf(2.0f * 1.0f * (1.0f + (xstl1_norm1[2] * adjust))) * (DREAM3D::Constants::k_SqrtPi / 2.0f);
    sqCoord[1] = (xstl1_norm1[0] / fabsf(xstl1_norm1[0])) * sqrtf(2.0f * 1.0f * (1.0f + (xstl1_norm1[2] * adjust))) * ((2.0f / DREAM3D::Constants::k_SqrtPi) * atanf(xstl1_norm1[1] / xstl1_norm1[0]));
  }
  else
  {
    sqCoord[0] = (xstl1_norm1[1] / fabsf(xstl1_norm1[1])) * sqrtf(2.0f * 1.0f * (1.0f + (xstl1_norm1[2] * adjust))) * ((2.0f / DREAM3D::Constants::k_SqrtPi) * atanf(xstl1_norm1[0] / xstl1_norm1[1]));
    sqCoord[1] = (xstl1_norm1[1] / fabsf(xstl1_norm1[1])) * sqrtf(2.0f * 1.0f * (1.0f + (xstl1_norm1[2] * adjust))) * (DREAM3D::Constants::k_SqrtPi / 2.0f);
  }
  return nhCheck;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32_t VisualizeGBCDPoleFigure::writeCoords(FILE* f, const char* axis, const char* type,
                                         int64_t npoints, float min, float step)
{
  int32_t err = 0;
  fprintf(f, "%s %lld %s\n", axis, npoints, type);
  float* data = new float[npoints];
  float d;
  for (int64_t idx = 0; idx < npoints; ++idx)
  {
    d = idx * step + min;
    DREAM3D::Endian::FromSystemToBig::convert(d);
    data[idx] = d;
  }
  size_t totalWritten = fwrite(static_cast<void*>(data), sizeof(float), static_cast<size_t>(npoints), f);
  delete[] data;
  if (totalWritten != static_cast<size_t>(npoints) )
  {
    QString ss = QObject::tr("Error writing binary VTK data to file '%1'").arg(m_OutputFile);
    setErrorCondition(-1);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    fclose(f);
    return getErrorCondition();
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer VisualizeGBCDPoleFigure::newFilterInstance(bool copyFilterParameters)
{
  VisualizeGBCDPoleFigure::Pointer filter = VisualizeGBCDPoleFigure::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VisualizeGBCDPoleFigure::getCompiledLibraryName()
{ return IOConstants::IOBaseName; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VisualizeGBCDPoleFigure::getGroupName()
{ return DREAM3D::FilterGroups::IOFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VisualizeGBCDPoleFigure::getSubGroupName()
{ return DREAM3D::FilterSubGroups::OutputFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString VisualizeGBCDPoleFigure::getHumanLabel()
{ return "Write GBCD Pole Figure (VTK)"; }
