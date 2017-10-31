#pragma once

class Tilemap;
//! Functions for loading Tilemap from disk
class TilemapLoading
{
public:
    
    //! Loads a .tsmap file to a new Tilemap
    static Tilemap* LoadTSMap(const std::string& file);
};
