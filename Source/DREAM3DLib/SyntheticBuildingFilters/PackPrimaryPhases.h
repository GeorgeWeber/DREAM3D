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

#ifndef PACKPRIMARYPHASES_H_
#define PACKPRIMARYPHASES_H_

#include <vector>
#include <QtCore/QString>

#include <boost/shared_array.hpp>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"
#include "DREAM3DLib/DataArrays/StatsDataArray.h"
#include "DREAM3DLib/StatsData/StatsData.h"
#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/DataContainers/VolumeDataContainer.h"
#include "DREAM3DLib/ShapeOps/ShapeOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"

typedef struct
{
  float m_Volumes;
  float m_EquivalentDiameters;
  float m_AxisLengths[3];
  float m_AxisEulerAngles[3];
  float m_Omega3s;
  int m_FeaturePhases;
  int m_Neighborhoods;
} Feature;

/**
 * @class PackPrimaryPhases PackPrimaryPhases.h DREAM3DLib/SyntheticBuilderFilters/PackPrimaryPhases.h
 * @brief
 * @author
 * @date Nov 19, 2011
 * @version 1.0
 */
class DREAM3DLib_EXPORT PackPrimaryPhases : public AbstractFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    DREAM3D_SHARED_POINTERS(PackPrimaryPhases)
    DREAM3D_STATIC_NEW_MACRO(PackPrimaryPhases)
    DREAM3D_TYPE_MACRO_SUPER(PackPrimaryPhases, AbstractFilter)

    virtual ~PackPrimaryPhases();
    DREAM3D_INSTANCE_STRING_PROPERTY(DataContainerName)
    DREAM3D_INSTANCE_STRING_PROPERTY(CellAttributeMatrixName)
    DREAM3D_INSTANCE_STRING_PROPERTY(CellFeatureAttributeMatrixName)
    DREAM3D_INSTANCE_STRING_PROPERTY(CellEnsembleAttributeMatrixName)

    typedef boost::shared_array<float> SharedFloatArray;
    typedef boost::shared_array<int> SharedIntArray;

    virtual const QString getGroupName() { return DREAM3D::FilterGroups::SyntheticBuildingFilters; }
    virtual const QString getSubGroupName() { return DREAM3D::FilterSubGroups::PackingFilters; }
    virtual const QString getHumanLabel() { return "Pack Primary Phases"; }

    DREAM3D_INSTANCE_STRING_PROPERTY(ErrorOutputFile)
    DREAM3D_INSTANCE_STRING_PROPERTY(VtkOutputFile)
    DREAM3D_INSTANCE_STRING_PROPERTY(CsvOutputFile)
    Q_PROPERTY(QString CsvOutputFile READ getCsvOutputFile WRITE setCsvOutputFile NOTIFY parametersChanged)
    DREAM3D_INSTANCE_PROPERTY(bool, PeriodicBoundaries)
    Q_PROPERTY(bool PeriodicBoundaries READ getPeriodicBoundaries WRITE setPeriodicBoundaries NOTIFY parametersChanged)
    DREAM3D_INSTANCE_PROPERTY(bool, WriteGoalAttributes)
    Q_PROPERTY(bool WriteGoalAttributes READ getWriteGoalAttributes WRITE setWriteGoalAttributes NOTIFY parametersChanged)


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

    virtual void preflight();

    /**
     * @brief Reimplemented from @see AbstractFilter class
     */
    virtual void execute();


  signals:
    void parametersChanged();

  protected:
    PackPrimaryPhases();

    void initialize_packinggrid();

    void generate_feature(int phase, int Seed, Feature* feature, unsigned int shapeclass);

    void transfer_attributes(int gnum, Feature* feature);
    void insert_feature(size_t featureNum);

    void move_feature(size_t featureNum, float xc, float yc, float zc);

    float check_sizedisterror(Feature* feature);
    void determine_neighbors(size_t featureNum, int add);
    float check_neighborhooderror(int gadd, int gremove);

    float check_fillingerror(int gadd, int gremove, Int32ArrayType::Pointer featureOwnersPtr, BoolArrayType::Pointer exclusionZonesPtr);
    void assign_voxels();
    void assign_gaps_only();
    void cleanup_features();
    void write_goal_attributes();

    void compare_1Ddistributions(QVector<float>, QVector<float>, float& sqrerror);
    void compare_2Ddistributions(QVector<QVector<float> >, QVector<QVector<float> >, float& sqrerror);
    void compare_3Ddistributions(QVector<QVector<QVector<float> > >, QVector<QVector<QVector<float> > >, float& sqrerror);

    int writeVtkFile(int32_t* featureOwners, bool* exclusionZonesPtr);
    int estimate_numfeatures(int xpoints, int ypoints, int zpoints, float xres, float yres, float zres);


  private:
    int32_t* m_Neighbors;

    // Cell Data - make sure these are all initialized to NULL in the constructor
    DEFINE_PTR_WEAKPTR_DATAARRAY(int32_t, FeatureIds)
    DEFINE_PTR_WEAKPTR_DATAARRAY(int32_t, CellPhases)
    int8_t*  m_SurfaceVoxels;

    // Feature Data - make sure these are all initialized to NULL in the constructor
    DEFINE_PTR_WEAKPTR_DATAARRAY(int32_t, FeaturePhases)
    DEFINE_PTR_WEAKPTR_DATAARRAY(int32_t, Neighborhoods)
    DEFINE_PTR_WEAKPTR_DATAARRAY(float, Centroids)
    DEFINE_PTR_WEAKPTR_DATAARRAY(float, Volumes)
    DEFINE_PTR_WEAKPTR_DATAARRAY(float, AxisLengths)
    DEFINE_PTR_WEAKPTR_DATAARRAY(float, AxisEulerAngles)
    DEFINE_PTR_WEAKPTR_DATAARRAY(float, Omega3s)
    DEFINE_PTR_WEAKPTR_DATAARRAY(float, EquivalentDiameters)

    // Ensemble Data - make sure these are all initialized to NULL in the constructor
    DEFINE_PTR_WEAKPTR_DATAARRAY(uint32_t, PhaseTypes)
    DEFINE_PTR_WEAKPTR_DATAARRAY(uint32_t, ShapeTypes)
    StatsDataArray* m_StatsDataArray;

    // All other private variables
    QMap<unsigned int, ShapeOps*> m_ShapeOps;
    ShapeOps::Pointer m_UnknownShapeOps;
    ShapeOps::Pointer m_CubicOctohedronOps;
    ShapeOps::Pointer m_CylinderOps;
    ShapeOps::Pointer m_EllipsoidOps;
    ShapeOps::Pointer m_SuperEllipsoidOps;

    OrthoRhombicOps::Pointer m_OrthoOps;

    QVector<QVector<int> > columnlist;
    QVector<QVector<int> > rowlist;
    QVector<QVector<int> > planelist;
    QVector<QVector<float> > ellipfunclist;

    unsigned long long int m_Seed;

    int firstPrimaryFeature;

    float sizex;
    float sizey;
    float sizez;
    float totalvol;

    float m_HalfPackingRes[3];
    float m_OneOverPackingRes[3];
    float m_OneOverHalfPackingRes[3];

    float m_PackingRes[3];
    int m_PackingPoints[3];
    int m_TotalPackingPoints;

    QVector<QVector<float> > featuresizedist;
    QVector<QVector<float> > simfeaturesizedist;
    QVector<QVector<QVector<float> > > neighbordist;
    QVector<QVector<QVector<float> > > simneighbordist;

    QVector<float> featuresizediststep;
    QVector<float> neighbordiststep;

    QVector<int> newnames;
    QVector<int> packqualities;
    QVector<int> gsizes;

    QVector<int> primaryphases;
    QVector<float> primaryphasefractions;

    float fillingerror, oldfillingerror;
    float currentneighborhooderror, oldneighborhooderror;
    float currentsizedisterror, oldsizedisterror;

    void dataCheck();
    void updateCellInstancePointers();
    void updateFeatureInstancePointers();

    PackPrimaryPhases(const PackPrimaryPhases&); // Copy Constructor Not Implemented
    void operator=(const PackPrimaryPhases&); // Operator '=' Not Implemented
};

#endif /* PackPrimaryPhases_H_ */

