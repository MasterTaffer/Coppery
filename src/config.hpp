#pragma once

#include <vector>
#include <string>
#include <memory>

#include <map>
#include <unordered_map>
#include <gbvfs_file.hpp>


/*! \brief Collection of configuration key-value pairs
 * 
 */
class ConfigGroup
{
    std::unordered_map<std::string, size_t> configurationsByName;
    std::vector<std::pair<std::string, std::string>> configurations; 
public:
    
    /*! \brief Parse a single configuration string line
     *  
     * Configuration strings are typically of form:
     * 
     *   KeyOfConfig = ValueOfConfig
     * 
     * \param line the string to parse
     * \return true if a new configuration value was added, false otherwise
     */ 
    bool parseLine(const std::string& line);
    
    /*! \brief Parse all configuration lines from a file
     * 
     * \param file handle to the file
     */
    void parseFromFile(GBVFS::File* file);

    /*! \brief Write all configurations to a file
     * 
     * \param file handle to the file
     */
    void writeToFile(GBVFS::FileWriter* file);

    friend class ConfigCollection;
};


/*! \brief Collection of configuration data
 * 
 * Used to group and organize key-value based configuration values. The
 * individual configurations are in separate ConfigGroups which are
 * typically loaded from different files (graphics config, sound config)
 * 
 */
class ConfigCollection
{
    std::map<std::string, std::unique_ptr<ConfigGroup>> configGroups;
public:
    
    /*! \brief Creates a ConfigGroup with name
     * 
     * \param group name of the new ConfigGroup
     * \return pointer to the new ConfigGroup or nullptr on failure
     */
    ConfigGroup* createGroup(const char* group);

    /*! \brief Add configurations from file to a ConfigGroup
     * 
     * \param file handle to the file
     * \param group name of the group
     */
    void addFromFileToGroup(GBVFS::File* file, const char* group);
    
    /*! \brief Write configurations from a ConfigGroup to file
     * 
     * \param file handle to the file
     * \param group name of the group
     */
    void writeGroupToFile(GBVFS::FileWriter* file, const char* group);

    /*!  \brief Get a single configuration value from the collection
     * 
     * \param group name of the group where the desired config value is in
     * \param key key for the configuration value
     * \param out output string for the value of the configuration value
     * \return true if the value was found and written to out, false otherwise.
     */
    bool getConfiguration(const char* group, const char* key, std::string& out);
};
