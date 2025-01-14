#ifndef BUILDING_HPP
#define BUILDING_HPP
#include "unit.hpp"
#include "company.hpp"

// Type for military outposts
class BuildingType {
public:
    using Id = uint8_t;

    std::string name;
    std::string ref_name;

    bool is_plot_on_sea;
    bool is_plot_on_land;
    bool is_build_land_units;
    bool is_build_naval_units;

    // Defensive bonus given to units on the outpost
    float defense_bonus;

    // Is this a factory?
    bool is_factory;
    // List of goods required to create output
    std::vector<Good *> inputs;
    // List of goods that this factory type creates
    std::vector<Good *> outputs;

    // Required goods, first describes the id of the good and the second describes how many
    std::vector<std::pair<Good *, size_t>> req_goods;
};

// A military outpost, on land serves as a "spawn" place for units
// When adjacent to a water tile this serves as a shipyard for spawning naval units
class Building {
public:
    using Id = uint16_t;

    // Position of outpost
    size_t x;
    size_t y;

    BuildingType* type;

    // Owner of the outpost
    Nation* owner;

    // Unit that is currently being built here (nullptr indicates no unit)
    UnitType* working_unit_type;
    BoatType* working_boat_type;

    // Remaining ticks until the unit is built
    uint16_t build_time;

    // Required goods for building the working unit
    std::vector<std::pair<Good *, size_t>> req_goods_for_unit;
    std::vector<std::pair<Good *, size_t>> req_goods_for_boat;
    // Required goods for building this, or repairing this after a military attack
    std::vector<std::pair<Good *, size_t>> req_goods;

    bool can_do_output(const World& world);
    void add_to_stock(const World& world, const Good* good, size_t add);
    Province* get_province(const World& world);
    void create_factory(World& world);
    void delete_factory(World& world);

    // Corporate owner of this building
    Company* corporate_owner;
    // Total money that the factory has
    float budget = 0.f;
    // Days that the factory has not been operational
    size_t days_unoperational = 0;
    // Money needed to produce - helps determine the price of the output products
    float production_cost = 0.f;
    // Stockpile of inputs in the factory
    std::vector<size_t> stockpile;
    // Used for faster lookups
    std::vector<Product *> output_products;
    // The employees needed per output
    std::vector<size_t> employees_needed_per_output;
    // The desired quality of a product (otherwise it's not accepted)
    float min_quality = 0.f;
    // The pay we are willing to give
    size_t willing_payment = 0;
    // How many workers are in the industry
    size_t workers = 0;
};

#endif