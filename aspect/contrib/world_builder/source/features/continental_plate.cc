/*
  Copyright (C) 2018 - 2021 by the authors of the World Builder code.

  This file is part of the World Builder.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <world_builder/features/continental_plate.h>
#include <world_builder/features/continental_plate_models/temperature/interface.h>
#include <world_builder/features/continental_plate_models/composition/interface.h>

#include <world_builder/utilities.h>
#include <world_builder/assert.h>
#include <world_builder/nan.h>
#include <world_builder/parameters.h>

#include <world_builder/types/array.h>
#include <world_builder/types/double.h>
#include <world_builder/types/string.h>
#include <world_builder/types/object.h>
#include <world_builder/types/unsigned_int.h>
#include <world_builder/types/plugin_system.h>


namespace WorldBuilder
{
  using namespace Utilities;

  namespace Features
  {
    ContinentalPlate::ContinentalPlate(WorldBuilder::World *world_)
      :
      min_depth(NaN::DSNAN),
      max_depth(NaN::DSNAN)
    {
      this->world = world_;
      this->name = "continental plate";
    }

    ContinentalPlate::~ContinentalPlate()
      = default;


    void
    ContinentalPlate::declare_entries(Parameters &prm,
                                      const std::string & /*unused*/,
                                      const std::vector<std::string> &required_entries)
    {
      prm.declare_entry("", Types::Object(required_entries), "continental plate object");

      prm.declare_entry("min depth", Types::Double(0),
                        "The depth to which this feature is present");
      prm.declare_entry("max depth", Types::Double(std::numeric_limits<double>::max()),
                        "The depth to which this feature is present");
      prm.declare_entry("temperature models",
                        Types::PluginSystem("", Features::ContinentalPlateModels::Temperature::Interface::declare_entries, {"model"}),
                        "A list of temperature models.");
      prm.declare_entry("composition models",
                        Types::PluginSystem("", Features::ContinentalPlateModels::Composition::Interface::declare_entries, {"model"}),
                        "A list of composition models.");
      prm.declare_entry("grains models",
                        Types::PluginSystem("", Features::ContinentalPlateModels::Grains::Interface::declare_entries, {"model"}),
                        "A list of grains models.");
    }

    void
    ContinentalPlate::parse_entries(Parameters &prm)
    {
      const CoordinateSystem coordinate_system = prm.coordinate_system->natural_coordinate_system();

      this->name = prm.get<std::string>("name");
      this->get_coordinates("coordinates", prm, coordinate_system);

      min_depth = prm.get<double>("min depth");
      max_depth = prm.get<double>("max depth");


      prm.get_unique_pointers<Features::ContinentalPlateModels::Temperature::Interface>("temperature models", temperature_models);

      prm.enter_subsection("temperature models");
      {
        for (unsigned int i = 0; i < temperature_models.size(); ++i)
          {
            prm.enter_subsection(std::to_string(i));
            {
              temperature_models[i]->parse_entries(prm);
            }
            prm.leave_subsection();
          }
      }
      prm.leave_subsection();


      prm.get_unique_pointers<Features::ContinentalPlateModels::Composition::Interface>("composition models", composition_models);

      prm.enter_subsection("composition models");
      {
        for (unsigned int i = 0; i < composition_models.size(); ++i)
          {
            prm.enter_subsection(std::to_string(i));
            {
              composition_models[i]->parse_entries(prm);
            }
            prm.leave_subsection();
          }
      }
      prm.leave_subsection();


      prm.get_unique_pointers<Features::ContinentalPlateModels::Grains::Interface>("grains models", grains_models);

      prm.enter_subsection("grains models");
      {
        for (unsigned int i = 0; i < grains_models.size(); ++i)
          {
            prm.enter_subsection(std::to_string(i));
            {
              grains_models[i]->parse_entries(prm);
            }
            prm.leave_subsection();
          }
      }
      prm.leave_subsection();

    }


    double
    ContinentalPlate::temperature(const Point<3> &position,
                                  const double depth,
                                  const double gravity_norm,
                                  double temperature) const
    {
      WorldBuilder::Utilities::NaturalCoordinate natural_coordinate = WorldBuilder::Utilities::NaturalCoordinate(position,
                                                                      *(world->parameters.coordinate_system));
      if (depth <= max_depth && depth >= min_depth &&
          Utilities::polygon_contains_point(coordinates, Point<2>(natural_coordinate.get_surface_coordinates(),
                                                                  world->parameters.coordinate_system->natural_coordinate_system())))
        {
          for (auto &temperature_model: temperature_models)
            {
              temperature = temperature_model->get_temperature(position,
                                                               depth,
                                                               gravity_norm,
                                                               temperature,
                                                               min_depth,
                                                               max_depth);

              WBAssert(!std::isnan(temperature), "Temparture is not a number: " << temperature
                       << ", based on a temperature model with the name " << temperature_model->get_name());
              WBAssert(std::isfinite(temperature), "Temparture is not a finite: " << temperature
                       << ", based on a temperature model with the name " << temperature_model->get_name());

            }
        }

      return temperature;
    }

    double
    ContinentalPlate::composition(const Point<3> &position,
                                  const double depth,
                                  const unsigned int composition_number,
                                  double composition) const
    {
      WorldBuilder::Utilities::NaturalCoordinate natural_coordinate = WorldBuilder::Utilities::NaturalCoordinate(position,
                                                                      *(world->parameters.coordinate_system));

      if (depth <= max_depth && depth >= min_depth &&
          Utilities::polygon_contains_point(coordinates, Point<2>(natural_coordinate.get_surface_coordinates(),
                                                                  world->parameters.coordinate_system->natural_coordinate_system())))
        {
          for (auto &composition_model: composition_models)
            {
              composition = composition_model->get_composition(position,
                                                               depth,
                                                               composition_number,
                                                               composition,
                                                               min_depth,
                                                               max_depth);

              WBAssert(!std::isnan(composition), "Composition is not a number: " << composition
                       << ", based on a temperature model with the name " << composition_model->get_name());
              WBAssert(std::isfinite(composition), "Composition is not a finite: " << composition
                       << ", based on a temperature model with the name " << composition_model->get_name());

            }
        }

      return composition;
    }

    WorldBuilder::grains
    ContinentalPlate::grains(const Point<3> &position,
                             const double depth,
                             const unsigned int composition_number,
                             WorldBuilder::grains grains) const
    {
      WorldBuilder::Utilities::NaturalCoordinate natural_coordinate = WorldBuilder::Utilities::NaturalCoordinate(position,
                                                                      *(world->parameters.coordinate_system));

      if (depth <= max_depth && depth >= min_depth &&
          Utilities::polygon_contains_point(coordinates, Point<2>(natural_coordinate.get_surface_coordinates(),
                                                                  world->parameters.coordinate_system->natural_coordinate_system())))
        {
          for (auto &grains_model: grains_models)
            {
              grains = grains_model->get_grains(position,
                                                depth,
                                                composition_number,
                                                grains,
                                                min_depth,
                                                max_depth);

              /*WBAssert(!std::isnan(composition), "Composition is not a number: " << composition
                       << ", based on a temperature model with the name " << composition_model->get_name());
              WBAssert(std::isfinite(composition), "Composition is not a finite: " << composition
                       << ", based on a temperature model with the name " << composition_model->get_name());*/

            }
        }

      return grains;
    }

    WB_REGISTER_FEATURE(ContinentalPlate, continental plate)

  }
}
