/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#include "AddNoise.h"

#include <map>


#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/DREAM3DMath.h"
#include "DREAM3DLib/Common/DREAM3DRandom.h"
#include "DREAM3DLib/Common/DataContainerMacros.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddNoise::AddNoise() :
AbstractFilter(),
m_GBEuclideanDistancesArrayName(DREAM3D::CellData::GBEuclideanDistances),
m_GBEuclideanDistances(NULL)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddNoise::~AddNoise()
{
}
// -----------------------------------------------------------------------------
void AddNoise::writeFilterParameters(AbstractFilterParametersWriter* writer)
{
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddNoise::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);
  std::stringstream ss;
  VoxelDataContainer* m = getVoxelDataContainer();

  // Cell Data
  GET_PREREQ_DATA(m, DREAM3D, CellData, GBEuclideanDistances, ss, -300, float, FloatArrayType, voxels, 1)
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddNoise::preflight()
{
  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddNoise::execute()
{
  int err = 0;
  setErrorCondition(err);
  DREAM3D_RANDOMNG_NEW()
  VoxelDataContainer* m = getVoxelDataContainer();

  if(NULL == m)
  {
    setErrorCondition(-999);
    notifyErrorMessage("The DataContainer Object was NULL", -999);
    return;
  }

  int64_t totalPoints = m->getTotalPoints();
  size_t totalFields = m->getNumFieldTuples();

  dataCheck(false, totalPoints, totalFields, m->getNumEnsembleTuples());
  if (getErrorCondition() < 0)
  {
    return;
  }

  add_noise();

  // If there is an error set this to something negative and also set a message
 notifyStatusMessage("AddNoises Completed");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void  AddNoise::add_noise()
{
 notifyStatusMessage("Adding Noise");
  DREAM3D_RANDOMNG_NEW()

  VoxelDataContainer* m = getVoxelDataContainer();

  std::list<std::string> voxelArrayNames = m->getCellArrayNameList();

  int count1 = 0;
  int count2 = 0;
  float random = 0.0;
  int64_t totalPoints = m->getTotalPoints();
  for (size_t i = 0; i < static_cast<size_t>(totalPoints); ++i)
  {
	  if(m_GBEuclideanDistances[3*i] < 1)
	  {
		count1++;
		random = static_cast<float>( rg.genrand_res53() );
		if(random < 0.1)
		{
		  count2++;
          for(std::list<std::string>::iterator iter = voxelArrayNames.begin(); iter != voxelArrayNames.end(); ++iter)
          {
            std::string name = *iter;
            IDataArray::Pointer p = m->getCellData(*iter);
			p->InitializeTuple(i,0); 
          }
		}
	  }
  }
}
