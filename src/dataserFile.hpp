#pragma once
#include <string>
#include <iostream>
#include <dataser.hpp>


namespace Dataser
{


    /*! \brief Load a table from file
     *
     * Type will be determined from file extension
     *
     */
    Table* ReadFromFile(const std::string& file);
    
    //! Load a table from TOML file
    Table* ReadTOMLFromFile(const std::string& file);

    //! Load a table from XML file
    Table* ReadXMLFromFile(const std::string& file);
    
    //! Write a table to a file in XML format
    bool WriteXMLToFile(Table* t, const std::string& file);
};
