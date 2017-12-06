#include "roadnoisevehiclecategory.h"


// -------------------------------------------------------------------------------------------------------------
// constructor for the vehicle category class
// -------------------------------------------------------------------------------------------------------------
RoadNoiseVehicleCategory::RoadNoiseVehicleCategory()
{
  this->gradientCorrection = new GradientCategory();
}

// -------------------------------------------------------------------------------------------------------------
// destructor for the vehicle category class
// -------------------------------------------------------------------------------------------------------------
RoadNoiseVehicleCategory::~RoadNoiseVehicleCategory()
{
  delete this->gradientCorrection;
  if (this->studdedProps != NULL)
    delete this->studdedProps;
}
