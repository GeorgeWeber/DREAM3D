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


#ifndef _WriteImages_H_
#define _WriteImages_H_

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"

/**
 * @brief The WriteImages class. See [Filter documentation](@ref writeimages) for details.
 */
class WriteImages : public AbstractFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    DREAM3D_SHARED_POINTERS(WriteImages)
    DREAM3D_STATIC_NEW_MACRO(WriteImages)
    DREAM3D_TYPE_MACRO_SUPER(WriteImages, AbstractFilter)

    virtual ~WriteImages();

    DREAM3D_FILTER_PARAMETER(bool, FilePrefix)
    Q_PROPERTY(bool FilePrefix READ getFilePrefix WRITE setFilePrefix)

    DREAM3D_FILTER_PARAMETER(QString, ImagePrefix)
    Q_PROPERTY(QString ImagePrefix READ getImagePrefix WRITE setImagePrefix)

    DREAM3D_FILTER_PARAMETER(QString, OutputPath)
    Q_PROPERTY(QString OutputPath READ getOutputPath WRITE setOutputPath)

    DREAM3D_FILTER_PARAMETER(int, ImageFormat)
    Q_PROPERTY(int ImageFormat READ getImageFormat WRITE setImageFormat)

    DREAM3D_FILTER_PARAMETER(int, Plane)
    Q_PROPERTY(int Plane READ getPlane WRITE setPlane)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, ColorsArrayPath)
    Q_PROPERTY(DataArrayPath ColorsArrayPath READ getColorsArrayPath WRITE setColorsArrayPath)

    enum ImageFormatType
    {
      TifImageType = 0,
      BmpImageType = 1,
      PngImageType = 2,
      JpgImageType = 3
    };

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

    /**
     * @brief saveImage Saves the data to an image on the disk
     * @param slice The axis on which the slicing occurs
     * @param dA Dimensions of one axis of the plane
     * @param dB Dimensions of the second axis of the plane
     * @param dims Array of the 3 dimensions
     * @return Integer error values
     */
    int32_t saveImage(size_t slice, size_t dB, size_t dA, size_t* dims);

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
    WriteImages();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

  private:
    DEFINE_DATAARRAY_VARIABLE(uint8_t, Colors)

    WriteImages(const WriteImages&); // Copy Constructor Not Implemented
    void operator=(const WriteImages&); // Operator '=' Not Implemented
};

#endif /* _WriteImages_H_ */
