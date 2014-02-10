/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
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
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
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
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef _RemoveArrays_H_
#define _RemoveArrays_H_

#include <string>
#include <set>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"
#include "DREAM3DLib/Common/AbstractFilter.h"


/**
 * @class RemoveArrays RemoveArrays.h /FilterCategoryFilters/RemoveArrays.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class DREAM3DLib_EXPORT RemoveArrays : public AbstractFilter
{
  public:
    DREAM3D_SHARED_POINTERS(RemoveArrays)
    DREAM3D_STATIC_NEW_MACRO(RemoveArrays)
    DREAM3D_TYPE_MACRO_SUPER(RemoveArrays, AbstractFilter)

    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedVoxelCellArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedVoxelFieldArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedVoxelEnsembleArrays)

    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSurfaceVertexArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSurfaceFaceArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSurfaceEdgeArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSurfaceFieldArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSurfaceEnsembleArrays)

    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSolidMeshVertexArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSolidMeshFaceArrays)
    DREAM3D_INSTANCE_PROPERTY(std::set<std::string>, SelectedSolidMeshEdgeArrays)

    virtual ~RemoveArrays();

    typedef std::set<std::string> ArrayList_t;

    virtual void setVoxelSelectedArrayNames(std::set<std::string> selectedCellArrays,
                                            std::set<std::string> selectedFieldArrays,
                                            std::set<std::string> selectedEnsembleArrays);
    virtual void setSurfaceSelectedArrayNames(std::set<std::string> selectedVertexArrays,
                                                  std::set<std::string> selectedFaceArrays,
                                                  std::set<std::string> selectedEdgeArrays,
                                                  std::set<std::string> selectedFieldArrays,
                                                  std::set<std::string> selectedEnsembleArrays);
    virtual void setSolidMeshSelectedArrayNames(std::set<std::string> selectedVertexArrays,
                                                std::set<std::string> selectedFaceArrays,
                                                std::set<std::string> selectedEdgeArrays);

    /**
    * @brief This returns the group that the filter belonds to. You can select
    * a different group if you want. The string returned here will be displayed
    * in the GUI for the filter
    */
    virtual const std::string getGroupName() { return DREAM3D::FilterGroups::GenericFilters; }
  virtual const std::string getSubGroupName() { return DREAM3D::FilterSubGroups::MemoryManagementFilters; }

    /**
    * @brief This returns a string that is displayed in the GUI. It should be readable
    * and understandable by humans.
    */
    virtual const std::string getHumanLabel() { return "Delete Arrays"; }

    /**
    * @brief This method will instantiate all the end user settable options/parameters
    * for this filter
    */
    virtual void setupFilterParameters();

    /**
    * @brief This method will write the options to a file
    * @param writer The writer that is used to write the options to a file
    */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    /**
    * @brief Reimplemented from @see AbstractFilter class
    */
    virtual void execute();

    /**
    * @brief This function runs some sanity checks on the DataContainer and inputs
    * in an attempt to ensure the filter can process the inputs.
    */
    virtual void preflight();

  protected:
    RemoveArrays();

    /**
    * @brief Checks for the appropriate parameter values and availability of
    * arrays in the data container
    * @param preflight
    * @param voxels The number of voxels
    * @param fields The number of fields
    * @param ensembles The number of ensembles
    */
    void dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles);


    RemoveArrays(const RemoveArrays&); // Copy Constructor Not Implemented
    void operator=(const RemoveArrays&); // Operator '=' Not Implemented
};

#endif /* _RemoveArrays_H_ */
