#ifndef ROADNOISECATALOG_H
#define ROADNOISECATALOG_H

#include "roadnoisevehiclecategory.h"
#include "roadnoiseaux.h"

//using namespace CNOSSOS_ROADNOISE;

class RoadNoiseCatalog
{
public:
  RoadNoiseCatalog();
  ~RoadNoiseCatalog();

private:
  // array for categories
  RoadNoiseVehicleCategory *Category[MAX_SRC_CAT];

public:
  int		refTemp;
  double	refSpeed;
  double	minSpeed;
  double srcHeight;

  // Number of actual categories
  int numCategories;

  // Functions to load the different XML catalog files
  void LoadRoadParamsFromFile(const string fn);
  void LoadRoadSurfacesFromFile(const string fn);

  // Accessor methods for road vehicle categories
  RoadNoiseVehicleCategory* addCategory();
  int				 indexOfCategory(const string id);
  RoadNoiseVehicleCategory* getCategory(const string id);
  RoadNoiseVehicleCategory* getCategory(const int index);

  // array for road surface definitions
  list<RoadSurface> surfaces;

};

// static pointer containing the catalog object
static RoadNoiseCatalog* catalog;

#endif // ROADNOISECATALOG_H
