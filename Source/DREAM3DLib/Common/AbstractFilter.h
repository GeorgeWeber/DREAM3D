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

#ifndef _AbstractFilter_H_
#define _AbstractFilter_H_

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/Common/Observable.h"
#include "DREAM3DLib/DataContainers/DataContainerArray.h"
#include "DREAM3DLib/FilterParameters/FilterParameter.h"
#include "DREAM3DLib/Common/PipelineMessage.h"


class AbstractFilterParametersReader;
class AbstractFilterParametersWriter;
class IDREAM3DPlugin;

/**
 * @class AbstractFilter AbstractFilter.h DREAM3DLib/Common/AbstractFilter.h
 * @brief This class is the basic class to subclass when creating a new Filter for
 * DREAM.3D. The subclass must implement at least the  execute method. If an
 * error occurs during the execution of the filter set the errorCondition to
 * a non zero value and optionally use the setErrorMessage() method to explain what the
 * error was. This class also inherits from Observable so that the filter can
 * provide updates to the user interface during execution.
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Nov 28, 2011
 * @version 1.0
 */
class DREAM3DLib_EXPORT AbstractFilter : public Observable
{
    Q_OBJECT
    Q_PROPERTY(int PipelineIndex READ getPipelineIndex WRITE setPipelineIndex)
    Q_PROPERTY(QString GroupName READ getGroupName CONSTANT)
    Q_PROPERTY(QString SubGroupName READ getSubGroupName CONSTANT)
    Q_PROPERTY(QString HumanLabel READ getHumanLabel CONSTANT)
    Q_PROPERTY(QString FilterVersion READ getFilterVersion CONSTANT)
    Q_PROPERTY(QString CompiledLibraryName READ getCompiledLibraryName CONSTANT)
    Q_PROPERTY(int Cancel READ getCancel WRITE setCancel)

  public:
    DREAM3D_SHARED_POINTERS(AbstractFilter)
    DREAM3D_STATIC_NEW_MACRO(AbstractFilter)
    DREAM3D_TYPE_MACRO_SUPER(AbstractFilter, Observable)

    virtual ~AbstractFilter();

    /**
     * @brief getGroupName Returns the group name for the filter, which determines its
     * top level sorting in the GUI
     * @return Group name
     */
    virtual const QString getGroupName();

    /**
     * @brief getSubGroupName Returns the subgroup name for the filter, which determines its
     * second level sorting in the GUI
     * @return Subgroup name
     */
    virtual const QString getSubGroupName();

    /**
     * @brief getHumanLabel Returns the human label for the filter, which determines its
     * primary labeling inthe GUI
     * @return Human lable
     */
    virtual const QString getHumanLabel();

    /**
     * @brief getBrandingString Returns the branding string for the filter, which is a tag
     * used to denote the filter's association with specific plugins
     * @return Branding string
     */
    virtual const QString getBrandingString();

    /**
     * @brief getCompiledLibraryName Returns the library name for the filter, which is the
     * actual C++ library in which the filter resides; this is usually the filter's parent
     * plugin
     * @return Compiled library name
     */
    virtual const QString getCompiledLibraryName();

    /**
     * @brief generateHtmlSummary Generates a brief HTML summary of the filter
     * @return HTML summary
     */
    virtual const QString generateHtmlSummary();

    /**
     * @brief setupFilterParameters Instantiates the filter parameters that are allowed
     * to be modified by the user
     */
    virtual void setupFilterParameters();

    /**
    * @brief writeFilterParameters Writes the filter parameters to a file
    * @param writer Writer that is used to write the parameters to a file
    * @param index Index the filter is in the pipeline that is being written
    */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
    * @brief readFilterParameters Reads the filter parameters from a file
    * @param reader Reader that is used to read the parameters from a file
    */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    /**
     * @brief execute Implements the main functionality of the filter
     */
    virtual void execute();

    /**
     * @brief preflight Communicates with the GUI to request user settings for the filter and
     * run any necessary sanity checks before execution
     */
    virtual void preflight();

    /**
     * @brief getPluginInstance Returns an instance of the filter's plugin
     * @return
     */
    IDREAM3DPlugin* getPluginInstance();

    /**
     * @brief newFilterInstance Instantiates a new instance of the filter
     * @param copyFilterParameters Whether to copy current filter parameters to the new instance
     * @return Shared pointer to the new filter instance
     */
    virtual Pointer newFilterInstance(bool copyFilterParameters);

    // ------------------------------
    // Standard methods for this class the are commonly used by subclasses.
    // ------------------------------
    virtual const QString getFilterVersion();

    DREAM3D_INSTANCE_PROPERTY(DataContainerArray::Pointer, DataContainerArray)

    DREAM3D_INSTANCE_PROPERTY(QVector<FilterParameter::Pointer>, FilterParameters)

    DREAM3D_INSTANCE_PROPERTY(QString, MessagePrefix)

    DREAM3D_INSTANCE_PROPERTY(int, ErrorCondition)

    DREAM3D_INSTANCE_PROPERTY(bool, InPreflight)

    // ------------------------------
    // These functions allow interogating the position the filter is in the pipeline and the previous and next filters
    // ------------------------------

    /**
    * @brief This property tells which index the filter is in a pipeline
    */
    DREAM3D_INSTANCE_PROPERTY(int, PipelineIndex)

    /**
    * @brief Returns the previous filter
    */
    DREAM3D_INSTANCE_PROPERTY(AbstractFilter::WeakPointer, PreviousFilter)
    /**
    * @brief Returns the next filter in the pipeline
    */
    DREAM3D_INSTANCE_PROPERTY(AbstractFilter::WeakPointer, NextFilter)

    /**
     * @brief doesPipelineContainFilterBeforeThis
     * @param name
     * @return
     */
    virtual bool doesPipelineContainFilterBeforeThis(const QString& name);

    /**
     * @brief doesPipelineContainFilterAfterThis
     * @param name
     * @return
     */
    virtual bool doesPipelineContainFilterAfterThis(const QString& name);


    virtual void printValues(std::ostream& out) {}

    /**
     * @brief getCancel Returns if the filter has been cancelled.
     * @return
     */
    bool getCancel();

    /**
     * @brief copyFilterParameterInstanceVariables
     * @param filter
     */
    virtual void copyFilterParameterInstanceVariables(AbstractFilter* filter);

  public slots:

    /**
      * @brief Cancel the operation
      */
    void setCancel(bool value);


#if 0
  signals:
    void updateFilterParameters(AbstractFilter* filter);
    void parametersChanged();
    void preflightAboutToExecute();
    void preflightExecuted();
#endif

  protected:
    AbstractFilter();



  private:
    bool m_Cancel;

    AbstractFilter(const AbstractFilter&); // Copy Constructor Not Implemented
    void operator=(const AbstractFilter&); // Operator '=' Not Implemented
};




#endif /* _AbstractFilter_H_  */
