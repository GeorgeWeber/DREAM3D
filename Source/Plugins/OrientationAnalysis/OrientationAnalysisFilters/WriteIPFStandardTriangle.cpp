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


#include "WriteIPFStandardTriangle.h"

#include <QtCore/QDir>

#include <QtGui/QPainter>

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/ChoiceFilterParameter.h"
#include "DREAM3DLib/FilterParameters/FileSystemFilterParameter.h"

#include "OrientationLib/SpaceGroupOps/CubicOps.h"

#include "OrientationAnalysis/OrientationAnalysisConstants.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
WriteIPFStandardTriangle::WriteIPFStandardTriangle() :
  AbstractFilter(),
  m_OutputFile(""),
  m_ImageSize(512)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
WriteIPFStandardTriangle::~WriteIPFStandardTriangle()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteIPFStandardTriangle::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FileSystemFilterParameter::New("Output File", "OutputFile", FilterParameterWidgetType::OutputFileWidget, getOutputFile(), FilterParameter::Parameter, "", "*.tif, *.bmp, *.png", "Image"));
  parameters.push_back(FilterParameter::New("Image Size", "ImageSize", FilterParameterWidgetType::IntWidget, getImageSize(), FilterParameter::Parameter, " Square Pixels"));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteIPFStandardTriangle::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setOutputFile( reader->readString("OutputFile", getOutputFile()));
  setImageSize( reader->readValue("ImageSize", getImageSize()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int WriteIPFStandardTriangle::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(FilterVersion)
  DREAM3D_FILTER_WRITE_PARAMETER(OutputFile)
  DREAM3D_FILTER_WRITE_PARAMETER(ImageSize)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteIPFStandardTriangle::dataCheck()
{
  setErrorCondition(0);

  QString ss;
  if (getOutputFile().isEmpty() == true)
  {
    ss = QObject::tr( "The output file must be set");
    notifyErrorMessage(getHumanLabel(), ss, -1);
    setErrorCondition(-1);
    return;
  }

  QFileInfo fi(getOutputFile());
  QDir parentPath = fi.path();
  QString ext = fi.completeSuffix();

  if (parentPath.exists() == false)
  {
    ss = QObject::tr( "The directory path for the output file does not exist. DREAM.3D will attempt to create this path during execution of the filter");
    notifyWarningMessage(getHumanLabel(), ss, -1);
  }

  if (ext.isEmpty())
  {
    ss = QObject::tr("The output file does not have an extension");
    notifyErrorMessage(getHumanLabel(), ss, -1003);
    setErrorCondition(-1004);
    return;
  }
  else if (ext != "tif" && ext != "bmp" && ext != "png")
  {
    ss = QObject::tr("The output file has an unsupported extension.  Please select a TIF, BMP, or PNG file");
    notifyErrorMessage(getHumanLabel(), ss, -1004);
    setErrorCondition(-1004);
    return;
  }

  if (m_ImageSize <= 0)
  {
    setErrorCondition(-1005);
    notifyErrorMessage(getHumanLabel(), "The size of the image must be positive", getErrorCondition());
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteIPFStandardTriangle::preflight()
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
QImage WriteIPFStandardTriangle::generateCubicHighTriangle()
{
  notifyStatusMessage(getHumanLabel(), "Generating Cubic IPF Triangle Legend");

  CubicOps ops;
  UInt8ArrayType::Pointer rgbaImage = ops.generateIPFTriangleLegend(getImageSize());
  QRgb* rgba = reinterpret_cast<QRgb*>(rgbaImage->getPointer(0));

  QImage image(getImageSize(), getImageSize(), QImage::Format_ARGB32_Premultiplied);

  int32_t xDim = getImageSize();
  int32_t yDim = getImageSize();
  size_t idx = 0;

  for (int32_t y = 0; y < yDim; ++y)
  {
    for (int32_t x = 0; x < xDim; ++x)
    {
      idx = (y * xDim) + x;
      image.setPixel(x, y, rgba[idx]);
    }
  }

  image = overlayCubicHighText(image);
  return image;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImage WriteIPFStandardTriangle::overlayCubicHighText(QImage image)
{
  QSize imageSize(getImageSize(), getImageSize());
  int32_t fontHeight = 0;
  int32_t fontWidth = 0;

  int32_t fontScale = 24 / 256 * getImageSize(); // At 256 Pixel Image, we want to use 24 Point font
  if (fontScale < 10) { fontScale = 10; } // Do not use fonts below 10Point.


  QFont font("Arial", fontScale, QFont::Bold);
  {
    QPainter painter;
    QImage pImage(100, 100, QImage::Format_ARGB32_Premultiplied);
    pImage.fill(0xFFFFFFFF); // All white background
    painter.begin(&pImage);

    painter.setFont(font);
    QFontMetrics metrics = painter.fontMetrics();
    fontHeight = metrics.height();
    fontWidth = metrics.width(QString("[0000]"));
    painter.end();
  }

  int32_t yMargin = 10;
  int32_t pImageWidth = imageSize.width() + yMargin;
  int32_t pImageHeight = imageSize.height() + fontHeight * 2;

  QImage pImage(pImageWidth, pImageHeight, QImage::Format_ARGB32_Premultiplied);
  pImage.fill(0xFFFFFFFF); // All white background

  // Create a Painter backed by a QImage to draw into
  QPainter painter;
  painter.begin(&pImage);
  painter.setRenderHint(QPainter::Antialiasing, true);

  painter.setFont(font);
  QFontMetrics metrics = painter.fontMetrics();

  // Draw the Figure into the upper left of the enlarged image so all the extra space is at the bottom
  QPoint point(yMargin / 2, 0);
  painter.drawImage(point, image);

  qint32 penWidth = 2;
  painter.setPen(QPen(QColor(0, 0, 0, 255), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  // Draw the [111] label in the Upper Right corner
  QString label("[111]");
  fontWidth = metrics.width(label);
  fontHeight = metrics.height();
  painter.drawText( pImageWidth - (fontWidth * 1.25), fontHeight * 1.10, label);

  label = QString("[101]");
  fontWidth = metrics.width(label);
  fontHeight = metrics.height();
  painter.drawText( pImageWidth - (fontWidth * 1.25), pImageHeight - fontHeight, label);

  label = QString("[001]");
  fontWidth = metrics.width(label);
  fontHeight = metrics.height();
  painter.drawText( 10, pImageHeight - fontHeight, label);

  label = QString("Cubic m-3m");
  fontWidth = metrics.width(label);
  fontHeight = metrics.height();
  painter.drawText( 10, fontHeight * 1.10, label);

  return pImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteIPFStandardTriangle::writeImage( QImage& image)
{

  QString ss = QObject::tr("Writing Image %1").arg(getOutputFile());
  notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);

  QFileInfo fi((m_OutputFile));
  QDir parent(fi.absolutePath());
  if (parent.exists() == false)
  {
    parent.mkpath(fi.absolutePath());
  }

  bool saved = image.save((m_OutputFile));
  if (!saved)
  {
    QString ss = QObject::tr("The Triangle image file '%1' was not saved").arg(getOutputFile());
    setErrorCondition(-90011);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void WriteIPFStandardTriangle::execute()
{
  setErrorCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  QImage image = generateCubicHighTriangle();
  writeImage(image);

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer WriteIPFStandardTriangle::newFilterInstance(bool copyFilterParameters)
{
  WriteIPFStandardTriangle::Pointer filter = WriteIPFStandardTriangle::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString WriteIPFStandardTriangle::getCompiledLibraryName()
{ return OrientationAnalysisConstants::OrientationAnalysisBaseName; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString WriteIPFStandardTriangle::getGroupName()
{ return DREAM3D::FilterGroups::IOFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString WriteIPFStandardTriangle::getSubGroupName()
{ return DREAM3D::FilterSubGroups::OutputFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString WriteIPFStandardTriangle::getHumanLabel()
{ return "Write IPF Triangle Legend (Cubic m-3m)"; }
