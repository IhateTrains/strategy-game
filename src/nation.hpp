#ifndef NATION_H
#define NATION_H

#include <cstdint>
#include <queue>
#include <deque>
#include <set>
#include <string>
#include <vector>

#include "policy.hpp"
#include "province.hpp"
#include "diplomacy.hpp"
#include "event.hpp"
#include "ideology.hpp"

/**
* Defines a one side relation between a country
* This allows for cases where a country A hates country B, but country B loves country A
 */
class NationRelation {
public:
    float relation;
    // Interest of a nation on this nation
    float interest;

    // Whetever commercial operations are allowed on on the target country
    bool has_embargo;
    bool has_war;
    bool has_alliance;
    bool has_defensive_pact;
    bool has_truce;
    bool has_embassy;
    bool has_military_access;
    bool has_market_access;

    // A nation can have free supplies to feed their soldiers
    bool free_supplies;
};

// Hints for the client on how to display the nation
class NationClientHint {
public:
    uint32_t colour;

    // Alternate name, for example communist Russia would be called USSR
    std::string alt_name;

    // Ideology to which this hint applies to (nullptr = default fallback)
    Ideology* ideology;
};

class NationModifier {
public:
    using Id = uint16_t;

    std::string ref_name;
    std::string name;

    // Modifiers for a nation, which increases/decreases certain stuff
    // They should never be 0, a modifier of 1.0 is equal to no modifer at
    // all. And a modifier of 0.5 would cause a 1/2. Similar to a 2 which
    // would make a x2
    float industry_output_mod = 1.f;
    float industry_input_mod = 1.f;
    float workers_needed_mod = 1.f;
    float salary_paid_mod = 1.f;
    float delivery_cost_mod = 1.f;
    float literacy_learn_mod = 1.f;
    float reproduction_mod = 1.f;
    float death_mod = 1.f;
    float militancy_mod = 1.f;
    float consciousness_mod = 1.f;
    float life_needs_met_mod = 1.f;
    float everyday_needs_met_mod = 1.f;
    float luxury_needs_met_mod = 1.f;
};

class Nation {
    inline void do_diplomacy();
    inline bool can_do_diplomacy();
public:
    using Id = uint16_t;

    bool is_ally(const Nation& nation);
    bool is_enemy(const Nation& nation);
    bool exists(void);
    void increase_relation(Nation& target);
    void decrease_relation(Nation& target);
    void auto_relocate_capital(void);
    void set_policy(Policies& policies);
    bool is_accepted_culture(const Pop& pop) const;
    float get_tax(const Pop& pop) const;
    void give_province(World& world, Province& province);
    const NationClientHint& get_client_hint(void) const;

    // Whetever this nation is controlled by AI
    bool controlled_by_ai;

    // Default transleted/display name
    std::string name;

    // Default reference name
    std::string ref_name;

    // A list with relations with all other nations, mapped 1:1 to the Nation list in the world
    std::vector<NationRelation> relations;

    // Id of the nation that has us on their sphere of influence
    Nation::Id spherer_id;

    // Number of diplomacy points available
    float diplomacy_points;

    // Total number of prestige
    float prestige = 0.1f;

    // Base literacy applied to all pops
    float base_literacy;

    // Determines if the nation is civilized
    bool is_civilized;

    // Level of infamy
    float infamy = 0.f;

    // 3 key scores used to define a nation's minimum prestige, how willing would the AI
    // be to challenge this nations and other valuable stuff
    float military_score = 0.f;
    float naval_score = 0.f;
    float economy_score = 0.f;

    // Total budget of the nation (money in ark), this is not equal to GDP, the GDP is the total sum of the price
    // of all products in the nation, which are volatile unless they are sold
    float budget;

    // Total GDP of the nation
    float gdp = 0.f;

    // The capital of this nation (can be nullptr)
    Province* capital = nullptr;

    // Hints for the client on how to draw a nation :)
    std::vector<NationClientHint> client_hints;
    Ideology* ideology = nullptr;

    // Accepted cultures in this nation, the accepted cultures may have some bonuses on provinces *totally*
    // owned by this nation
    std::set<Culture*> accepted_cultures;

    // List of provinces which are owned by this nation (including partial ownership)
    std::set<Province*> owned_provinces;

    // List of neighbouring nations
    std::set<Nation*> neighbours;

    // A pointer to a class defining the current policy of this nation
    Policies current_policy;

    // Time until a diplomatic action can be done
    uint16_t diplomatic_timer;

    std::vector<NationModifier> modifiers;

    bool is_ai = true;

    // Inbox of the nation; events that require our attention / should be processed
    std::deque<Event> inbox;
};

#endif
