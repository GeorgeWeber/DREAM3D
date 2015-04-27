/* ============================================================================
* Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
* Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
* Copyright (c) 2012 Joseph B. Kleingers (Student Research Assistant)
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
* Neither the name of Michael A. Groeber, Michael A. Jackson, Joseph B. Kleingers,
* the US Air Force, BlueQuartz Software nor the names of its contributors may be
* used to endorse or promote products derived from this software without specific
* prior written permission.
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

#include "AddFilterParameter.h"

#include "PluginMaker/PluginMaker.h"
#include "PluginMaker/FilterMaker.h"

#include <iostream>


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFilterParameter::AddFilterParameter(QWidget* parent) :
QWidget(parent)
{
  setupUi(this);

  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFilterParameter::~AddFilterParameter()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterParameter::setupGui()
{
  // Populate the type combo box
  QList<QString> typeList = getTypeList();
  for (int i = 0; i < typeList.size(); i++)
  {
    type->insertItem(i, typeList[i]);
  }

  errorString->setText("");

  // Update the "Add" button
  addFilterParameterBtn->setEnabled(filledOutCheck());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterParameter::on_addFilterParameterBtn_clicked()
{
  /* SLOT: FilterMaker::addFilterParameterToTable()
     CONNECT: FilterMaker::on_addFilterParameterBtn_clicked() */
  emit addBtnPressed(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterParameter::on_varName_textChanged(const QString &text)
{
  // Update the "Add" button
  if (type->currentText() == "SeparatorWidget")
  {
    addFilterParameterBtn->setEnabled(filledOutCheck_NoVarName());
  }
  else
  {
    addFilterParameterBtn->setEnabled(filledOutCheck());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterParameter::on_humanName_textChanged(const QString &text)
{
  // Update the "Add" button
  if (type->currentText() == "SeparatorWidget")
  {
    addFilterParameterBtn->setEnabled(filledOutCheck_NoVarName());
  }
  else
  {
    addFilterParameterBtn->setEnabled(filledOutCheck());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterParameter::on_type_currentTextChanged(const QString &text)
{
  if (text == "SeparatorWidget")
  {
    varName->clear();
    initValue->clear();
    varName->setEnabled(false);
    initValue->setEnabled(false);
  }
  else
  {
    varName->setEnabled(true);
    initValue->setEnabled(true);
  }

  // Update the "Add" button
  if (type->currentText() == "SeparatorWidget")
  {
    addFilterParameterBtn->setEnabled(filledOutCheck_NoVarName());
  }
  else
  {
    addFilterParameterBtn->setEnabled(filledOutCheck());
  }
}

//// -----------------------------------------------------------------------------
////
//// -----------------------------------------------------------------------------
//void AddFilterParameter::on_initValue_textChanged(const QString &text)
//{
//  // Update the "Add" button
//  addFilterParameterBtn->setEnabled(filledOutCheck());
//}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterParameter::on_cancelBtn_clicked()
{
  // Close the widget
  close();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AddFilterParameter::getVariableName()
{
  return varName->text();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AddFilterParameter::getHumanName()
{
  return humanName->text();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AddFilterParameter::getType()
{
  return type->currentText();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<QString> AddFilterParameter::getTypeList()
{
#include "DREAM3DWidgetsLib/Filter_Parameter_TypeList.cpp"
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AddFilterParameter::getInitValue()
{
  return initValue->text();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool AddFilterParameter::filledOutCheck()
{
  if (type->currentIndex() >= 0 && varName->text().isEmpty() == false 
    && humanName->text().isEmpty() == false /* && initValue->text().isEmpty() == false */)
  {
    return true;
  }
  else
  {
    return false;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool AddFilterParameter::filledOutCheck_NoVarName()
{
  if (type->currentIndex() >= 0 && humanName->text().isEmpty() == false /* && initValue->text().isEmpty() == false */)
  {
    return true;
  }
  else
  {
    return false;
  }
}



