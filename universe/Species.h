#ifndef _Species_h_
#define _Species_h_


#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/optional/optional.hpp>
#include "EnumsFwd.h"
#include "../util/Enum.h"
#include "../util/Export.h"
#include "../util/Pending.h"


namespace Condition {
    struct Condition;
}
namespace Effect {
    class EffectsGroup;
}

FO_COMMON_API extern const int ALL_EMPIRES;

//! Environmental suitability of planets for a particular Species
FO_ENUM(
    (PlanetEnvironment),
    ((INVALID_PLANET_ENVIRONMENT, -1))
    ((PE_UNINHABITABLE))
    ((PE_HOSTILE))
    ((PE_POOR))
    ((PE_ADEQUATE))
    ((PE_GOOD))
    ((NUM_PLANET_ENVIRONMENTS))
)


/** A setting that a ResourceCenter can be assigned to influence what it
  * produces.  Doesn't directly affect the ResourceCenter, but effectsgroups
  * can use activation or scope conditions that check whether a potential
  * target has a particular focus.  By this method, techs or buildings or
  * species can act on planets or other ResourceCenters depending what their
  * focus setting is. */
class FO_COMMON_API FocusType {
public:
    FocusType() = default;
    FocusType(std::string& name, std::string& description,
              std::unique_ptr<Condition::Condition>&& location,
              std::string& graphic);
    ~FocusType(); // needed due to forward-declared Condition held in unique_ptr

    const std::string&          Name() const        { return m_name; }          ///< returns the name for this focus type
    const std::string&          Description() const { return m_description; }   ///< returns a text description of this focus type
    const Condition::Condition* Location() const    { return m_location.get(); }///< returns the condition that determines whether an UniverseObject can use this FocusType
    const std::string&          Graphic() const     { return m_graphic; }       ///< returns the name of the grapic file for this focus type
    std::string                 Dump(unsigned short ntabs = 0) const;           ///< returns a data file format representation of this object

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;

private:
    std::string                                 m_name;
    std::string                                 m_description;
    std::shared_ptr<const Condition::Condition> m_location; // TODO: make unique_ptr - requires tweaking SpeciesParser to not require copies
    std::string                                 m_graphic;
};


/** A predefined type of population that can exist on a PopulationCenter.
  * Species have associated sets of EffectsGroups, and various other
  * properties that affect how the object on which they reside functions.
  * Each kind of Species must have a \a unique name string, by which it can be
  * looked up using GetSpecies(). */
class FO_COMMON_API Species {
public:
    Species(std::string&& name, std::string&& desc,
            std::string&& gameplay_desc, std::vector<FocusType>&& foci,
            std::string&& default_focus,
            std::map<PlanetType, PlanetEnvironment>&& planet_environments,
            std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
            std::unique_ptr<Condition::Condition>&& combat_targets,
            bool playable, bool native, bool can_colonize, bool can_produce_ships,
            const std::set<std::string>& tags,
            std::set<std::string>&& likes, std::set<std::string>&& dislikes,
            std::string&& graphic,
            double spawn_rate = 1.0, int spawn_limit = 99999);

    ~Species();

    const std::string&              Name() const                        { return m_name; }                  ///< returns the unique name for this type of species
    const std::string&              Description() const                 { return m_description; }           ///< returns a text description of this type of species
    /** returns a text description of this type of species */
    std::string                     GameplayDescription() const;

    const Condition::Condition*     Location() const                    { return m_location.get(); }        ///< returns the condition determining what planets on which this species may spawn
    const Condition::Condition*     CombatTargets() const               { return m_combat_targets.get(); }  ///< returns the condition for possible targets. may be nullptr if no condition was specified.

    std::string                     Dump(unsigned short ntabs = 0) const;                                   ///< returns a data file format representation of this object
    const std::vector<FocusType>&   Foci() const                        { return m_foci; }                  ///< returns the focus types this species can use
    const std::string&              DefaultFocus() const                { return m_default_focus; }       ///< returns the name of the planetary focus this species prefers. Default for new colonies and may affect happiness if on a different focus?
    const std::map<PlanetType, PlanetEnvironment>& PlanetEnvironments() const { return m_planet_environments; } ///< returns a map from PlanetType to the PlanetEnvironment this Species has on that PlanetType
    PlanetEnvironment               GetPlanetEnvironment(PlanetType planet_type) const;                     ///< returns the PlanetEnvironment this species has on PlanetType \a planet_type
    PlanetType                      NextBetterPlanetType(PlanetType initial_planet_type) const;             ///< returns the next better PlanetType for this species from the \a initial_planet_type specified

    /** Returns the EffectsGroups that encapsulate the effects that species of
        this type have. */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    float                           SpawnRate() const       { return m_spawn_rate; }
    int                             SpawnLimit() const      { return m_spawn_limit; }
    bool                            Playable() const        { return m_playable; }          ///< returns whether this species is a suitable starting species for players
    bool                            Native() const          { return m_native; }            ///< returns whether this species is a suitable native species (for non player-controlled planets)
    bool                            CanColonize() const     { return m_can_colonize; }      ///< returns whether this species can colonize planets
    bool                            CanProduceShips() const { return m_can_produce_ships; } ///< returns whether this species can produce ships
    const std::set<std::string>&    Tags() const            { return m_tags; }
    const std::set<std::string>&    Likes() const           { return m_likes; }
    const std::set<std::string>&    Dislikes() const        { return m_dislikes; }
    const std::string&              Graphic() const         { return m_graphic; }           ///< returns the name of the grapic file for this species

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                    GetCheckSum() const;

private:
    void Init();

    std::string                             m_name;
    std::string                             m_description;
    std::string                             m_gameplay_description;

    std::vector<FocusType>                  m_foci;
    std::string                             m_default_focus;
    std::map<PlanetType, PlanetEnvironment> m_planet_environments;

    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::unique_ptr<Condition::Condition>               m_location;
    std::unique_ptr<Condition::Condition>               m_combat_targets;

    bool                                    m_playable = true;
    bool                                    m_native = true;
    bool                                    m_can_colonize = true;
    bool                                    m_can_produce_ships = true;
    float                                   m_spawn_rate = 1.0;
    int                                     m_spawn_limit = 99999;

    std::set<std::string>                   m_tags;
    std::set<std::string>                   m_likes;
    std::set<std::string>                   m_dislikes;
    std::string                             m_graphic;
};


/** Holds all FreeOrion species.  Types may be looked up by name. */
class FO_COMMON_API SpeciesManager {
private:
    struct FO_COMMON_API PlayableSpecies
    { bool operator()(const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const; };
    struct FO_COMMON_API NativeSpecies
    { bool operator()(const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const; };

public:
    using SpeciesTypeMap = std::map<std::string, std::unique_ptr<Species>>;
    using CensusOrder = std::vector<std::string>;
    using iterator = SpeciesTypeMap::const_iterator;
    typedef boost::filter_iterator<PlayableSpecies, iterator> playable_iterator;
    typedef boost::filter_iterator<NativeSpecies, iterator>   native_iterator;

    SpeciesManager() = default;

    /** returns the building type with the name \a name; you should use the
      * free function GetSpecies() instead, mainly to save some typing. */
    const Species*      GetSpecies(const std::string& name) const;
    Species*            GetSpecies(const std::string& name);

    /** iterators for all species */
    iterator            begin() const;
    iterator            end() const;

    /** iterators for playble species. */
    playable_iterator   playable_begin() const;
    playable_iterator   playable_end() const;

    /** iterators for native species. */
    native_iterator     native_begin() const;
    native_iterator     native_end() const;

    /** returns an ordered list of tags that should be considered for census listings. */
    const CensusOrder&  census_order() const;

    /** returns true iff this SpeciesManager is empty. */
    bool                empty() const;

    /** returns the number of species stored in this manager. */
    int                 NumSpecies() const;
    int                 NumPlayableSpecies() const;
    int                 NumNativeSpecies() const;

    /** returns the name of a species in this manager, or an empty string if
      * this manager is empty. */
    const std::string&  RandomSpeciesName() const;

    /** returns the name of a playable species in this manager, or an empty
      * string if there are no playable species. */
    const std::string&  RandomPlayableSpeciesName() const;
    const std::string&  SequentialPlayableSpeciesName(int id) const;

    /** returns a map from species name to a set of object IDs that are the
      * homeworld(s) of that species in the current game. */
    std::map<std::string, std::set<int>>
        GetSpeciesHomeworldsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns a map from species name to a map from empire id to each the
      * species' opinion of the empire */
    const std::map<std::string, std::map<int, float>>&
        GetSpeciesEmpireOpinionsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns opinion of species with name \a species_name about empire with
      * id \a empire_id or 0.0 if there is no such opinion yet recorded. */
    float SpeciesEmpireOpinion(const std::string& species_name, int empire_id) const;

    /** returns a map from species name to a map from other species names to the
      * opinion of the first species about the other species. */
    const std::map<std::string, std::map<std::string, float>>&
        GetSpeciesSpeciesOpinionsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns opinion of species with name \a opinionated_species_name about
      * other species with name \a rated_species_name or 0.0 if there is no
      * such opinion yet recorded. */
    float SpeciesSpeciesOpinion(const std::string& opinionated_species_name,
                                const std::string& rated_species_name) const;

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;

    /** sets the opinions of species (indexed by name string) of empires (indexed
      * by id) as a double-valued number. */
    void SetSpeciesEmpireOpinions(std::map<std::string, std::map<int, float>>&& species_empire_opinions);
    void SetSpeciesEmpireOpinion(const std::string& species_name, int empire_id, float opinion);

    /** sets the opinions of species (indexed by name string) of other species
      * (indexed by name string) as a double-valued number. */
    void SetSpeciesSpeciesOpinions(std::map<std::string,
                                   std::map<std::string, float>>&& species_species_opinions);
    void SetSpeciesSpeciesOpinion(const std::string& opinionated_species,
                                  const std::string& rated_species, float opinion);
    void ClearSpeciesOpinions();

    void AddSpeciesHomeworld(std::string species, int homeworld_id);
    void RemoveSpeciesHomeworld(const std::string& species, int homeworld_id);
    void ClearSpeciesHomeworlds();

    void UpdatePopulationCounter();

    std::map<std::string, std::map<int, float>>&        SpeciesObjectPopulations(int encoding_empire = ALL_EMPIRES);
    std::map<std::string, std::map<std::string, int>>&  SpeciesShipsDestroyed(int encoding_empire = ALL_EMPIRES);

    /** Sets species types to the value of \p future. */
    void SetSpeciesTypes(Pending::Pending<std::pair<SpeciesTypeMap, CensusOrder>>&& future);

private:
    /** sets the homeworld ids of species in this SpeciesManager to those
      * specified in \a species_homeworld_ids */
    void SetSpeciesHomeworlds(std::map<std::string, std::set<int>>&& species_homeworld_ids);

    /** Assigns any m_pending_types to m_species. */
    static void CheckPendingSpeciesTypes();

    std::map<std::string, std::set<int>>                m_species_homeworlds;
    std::map<std::string, std::map<int, float>>         m_species_empire_opinions;
    std::map<std::string, std::map<std::string, float>> m_species_species_opinions;
    std::map<std::string, std::map<int, float>>         m_species_object_populations;
    std::map<std::string, std::map<std::string, int>>   m_species_species_ships_destroyed;

    template <typename Archive>
    friend void serialize(Archive&, SpeciesManager&, unsigned int const);
};

#endif
