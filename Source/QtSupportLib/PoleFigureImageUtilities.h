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

#ifndef _PoleFigureImageUtilities_H_
#define _PoleFigureImageUtilities_H_

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtGui/QImage>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/DataArrays/DataArray.hpp"

#include "OrientationLib/Utilities/PoleFigureUtilities.h"

#include "QtSupportLib/QtSupportLib.h"
#include "QtSupportLib/PoleFigureData.h"

/**
 * @class ColorPoleFigure ColorPoleFigure.h StatsGenerator/ColorPoleFigure.h
 * @brief
 *
 * @date Nov 4, 2011
 * @version 1.0
 */
class QtSupportLib_EXPORT PoleFigureImageUtilities
{
  public:
    PoleFigureImageUtilities();
    virtual ~PoleFigureImageUtilities();



    int countPixelNeighbors(int imageWidth, int imageHeight, int pX, int pY,
                            QVector<qint32>& data, QVector<qint32>& counts,
                            int kX, int kY, bool genmask = false);

    void generateKernelWeigths(int kernelWidth, int kernelHeight);

    static QImage GenerateScalarBar(int imageWidth, int imageHeight, PoleFigureConfiguration_t& config);


    static QImage PaintPoleFigureOverlay(int imageWidth, int imageHeight, QString label, QImage image);

    static QImage CreateQImageFromRgbaArray(UInt8ArrayType* poleFigurePtr, int imageDimension, bool includeOverlay);
    /**
     * @brief Create3ImagePoleFigure
     * @param i0
     * @param i1
     * @param i2
     * @param config
     * @return
     */
    static QImage Create3ImagePoleFigure(UInt8ArrayType* i0, UInt8ArrayType* i1, UInt8ArrayType* i2,
                                         PoleFigureConfiguration_t& config, int32_t layout = DREAM3D::Layout::Square);
  private:

    QVector<qint32> m_KernelWeights;
    bool m_KernelWeightsInited;


    PoleFigureImageUtilities(const PoleFigureImageUtilities&); // Copy Constructor Not Implemented
    void operator=(const PoleFigureImageUtilities&); // Operator '=' Not Implemented
};


#endif /* _PoleFigureImageUtilities_H_ */

