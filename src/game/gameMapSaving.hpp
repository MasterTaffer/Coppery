
#include <map>
#include <string>
#include <vector>
#include <fstream>

#ifdef GMS_USE_NAMESPACE
namespace GMS {
#endif // GMS_USE_NAMESPACE

#ifndef GMS_CLASS_GUARD
#define GMS_CLASS_GUARD

class MapObject
{
public:
    int x,y;
    int id;
    std::string type;
    std::string name;
    std::vector< std::pair<std::string,std::string> > properties;
};

class MapData
{
public:
    bool failed;
    int width;
    int height;
    std::map<std::string, int> tileDefs;
    //used only on loading
    std::vector<std::string> tileNames;
    std::vector<short> fg;
    std::vector<short> tfg;
    std::vector<short> bg;
    std::vector<char> collision;
    std::vector<MapObject> objects;
};

#endif //GMS_CLASS_GUARD

void SaveMap(MapData&, std::string);
void LoadMap(MapData&, std::string);

#ifdef GMS_CREATE_IMPLEMENTATION


template <typename T>
void dwany(std::vector<char>&v, const T& i)
{
    char* dk = new char[sizeof(T)];
    T* p = (T*)dk;

    p[0]  = i;

    for (size_t a = 0; a < sizeof(T); ++a)
    {
        v.push_back(dk[a]);
    }
    delete[] dk;
}
#define dwInt dwany<int>
#define dwShort dwany<short>

void dwChar(std::vector<char>&v, const char& i)
{
    v.push_back(i);
}

void dwString(std::vector<char>&v, const std::string& s)
{
    dwInt(v,s.size());
    for (const char& k : s)
        dwChar(v,k);
}

#define MAGIC 13456556
#define VERSION 102
std::vector<char> createTileMapHeader(MapData& md)
{
    std::vector<char> data;
    dwInt(data,MAGIC);
    dwInt(data,VERSION);
    dwInt(data,md.width);
    dwInt(data,md.height);
    return data;
}


std::vector<char> createTileDefData(MapData& md)
{
    std::vector<char> data;
    md.tileDefs.size();
    std::map<int,std::string> is;
    for (auto p : md.tileDefs)
        is[p.second] = p.first;

    dwInt(data,is.size());
    for (auto p : is)
    {
        dwString(data,p.second);
    }
    return data;
}

std::vector<char> createTilesData(MapData& md)
{
    std::vector<char> data;
    for (short k : md.bg)
        dwShort(data,k);
    for (short k : md.fg)
        dwShort(data,k);
    for (short k : md.tfg)
        dwShort(data,k);
    for (char k : md.collision)
        dwChar(data,k);
    return data;
}

std::vector<char> createObjectData(MapData& md)
{
    std::vector<char> data;
    dwInt(data,md.objects.size());
    for (MapObject& o : md.objects)
    {
        dwString(data,o.type);
        dwString(data,o.name);
        dwInt(data,o.x);
        dwInt(data,o.y);
        dwInt(data,o.properties.size());
        for (auto& p : o.properties)
        {
            dwString(data,p.first);
            dwString(data,p.second);
        }
    }
    return data;
}



void SaveMap(MapData& md ,std::string s)
{
    std::vector<char> data;
    auto d = createTileMapHeader(md);
    data.insert(data.end(),d.begin(),d.end());

    d = createTileDefData(md);
    data.insert(data.end(),d.begin(),d.end());

    d = createTilesData(md);
    data.insert(data.end(),d.begin(),d.end());

    d = createObjectData(md);
    data.insert(data.end(),d.begin(),d.end());

    std::ofstream fs;
    fs.open(s,std::ofstream::binary|std::ofstream::out|std::ofstream::trunc);

    fs.write(data.data(),data.size());
}



template <typename T>
void drany(std::istream& f, T& i)
{
    char* dk = (char*)&i;
    f.read(dk, sizeof(T));
}
#define drInt drany<int>
#define drShort drany<short>

void drChar(std::istream& f, char& i)
{
    f.read(&i,1);
}

void drString(std::istream& f, std::string& n)
{
    int len;
    drInt(f,len);
    n.clear();
    for (int i = 0; i < len; i++)
    {
        char c;
        drChar(f,c);
        n.push_back(c);
    }
}

bool readTileMapHeader(MapData& md, std::istream& f)
{
    int magic,version;
    drInt(f,magic);
    drInt(f,version);
    drInt(f,md.width);
    drInt(f,md.height);
    return (magic != MAGIC || version != VERSION);
}


void readTileDefData(MapData& md, std::istream& f)
{
    int count;
    drInt(f,count);
    md.tileNames.resize(count+1);
    for (int i = 0; i < count; i++)
    {
        std::string name;
        drString(f,name);
        md.tileDefs[name] = i+1;
        md.tileNames[i+1] = name;
    }
}

void readTilesData(MapData& md, std::istream& f)
{
    int count = md.width*md.height;
    for (int i = 0; i < count; i++)
    {
        short tile;
        drShort(f, tile);
        md.bg.push_back(tile);
    }
    for (int i = 0; i < count; i++)
    {
        short tile;
        drShort(f, tile);
        md.fg.push_back(tile);
    }
    for (int i = 0; i < count; i++)
    {
        short tile;
        drShort(f, tile);
        md.tfg.push_back(tile);
    }
    for (int i = 0; i < count; i++)
    {
        char coll;
        drChar(f, coll);
        md.collision.push_back(coll);
    }
}

void readObjectData(MapData& md, std::istream& f)
{
    int ocount;
    drInt(f,ocount);
    for (int i = 0; i < ocount; i++)
    {
        MapObject o;
        drString(f,o.type);
        drString(f,o.name);
        drInt(f,o.x);
        drInt(f,o.y);
        int pcount;
        drInt(f,pcount);
        for (int j = 0; j < pcount; j++)
        {
            std::pair<std::string,std::string> p;
            drString(f,p.first);
            drString(f,p.second);
            o.properties.push_back(p);
        }
        o.id = i;
        md.objects.push_back(o);
    }
    return;
}




void LoadMap(MapData& md ,std::istream& fs)
{
    if (readTileMapHeader(md,fs))
    {
        md.failed = true;
        return;
    }
    readTileDefData(md,fs);
    readTilesData(md,fs);
    readObjectData(md,fs);
    md.failed = false;
}


#endif // GMS_CREATE_IMPLEMENTATION

#ifdef GMS_USE_NAMESPACE
}
#endif // GMS_USE_NAMESPACE
