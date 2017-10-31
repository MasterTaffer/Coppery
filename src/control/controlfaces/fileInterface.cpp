#include "fileInterface.hpp"
#include "fileOperations.hpp"
#include "gbvfs_file.hpp"
#include "gbvfs.hpp"
#include "control/crossFunctional.hpp"

struct LuaFileEntry
{
	std::unique_ptr<GBVFS::File> handle; 
};

constexpr static const char* metaTableName = "FileInterface.File";



static int f_eof(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    if (!fp || !fp->handle || fp->handle->isEOF())
    {
        lua_pushboolean(lua, 1);
    }
    else
        lua_pushboolean(lua, 0);
    return 1;
}

static int f_close(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    if (!fp || !fp->handle)
    	return 0;
    fp->handle->close();
    return 0;
}

static int f_readLine(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    if (!fp || !fp->handle)
    	return 0;
    
    auto line = fp->handle->readLine();
    
	lua_pushlstring(lua, line.c_str(), line.size());
    return 1;
}
static int f_read(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    if (!fp || !fp->handle)
    	return 0;
    int64_t count = luaL_checkinteger(lua, 2);

    char* buffer = new(std::nothrow) char[count];

    if (buffer == nullptr)
    	luaL_error(lua, "Failed to allocate buffer memory");

    size_t cnt = fp->handle->read((void*) buffer, count);

	lua_pushlstring(lua, buffer, cnt);
    return 1;
}
static int f_getSize(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    if (!fp || !fp->handle)
    	return 0;
    
    lua_pushinteger(lua, fp->handle->getSize());
    return 1;
}


static int f_tell(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    if (!fp || !fp->handle)
    	return 0;
    
    lua_pushinteger(lua, fp->handle->tell());
    return 1;
}


static int f_seek(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    int64_t pos = luaL_checkinteger(lua, 2);
    if (!fp || !fp->handle)
    	return 0;

    fp->handle->seek(pos);
    return 0;
}


static int fileGC(lua_State* lua)
{
    LuaFileEntry* fp = (LuaFileEntry*)luaL_checkudata(lua, 1, metaTableName);
    if (fp)
    	fp->~LuaFileEntry();
    return 0;
}

static int f_open(lua_State* lua)
{
	const char* fname = luaL_checkstring(lua, 1);
	auto f = GetFileStream(fname);

	if (!f)
		return 0;

    LuaFileEntry* lfe = (LuaFileEntry*) lua_newuserdata(lua, sizeof (LuaFileEntry));

    lfe = new(lfe) LuaFileEntry();
	lfe->handle = std::move(f);

    luaL_getmetatable(lua, metaTableName);
    lua_setmetatable(lua, -2);

    return 1;
}


std::vector<std::string> f_dirEntries(const std::string& dir, const std::string& ext)
{
	std::vector<std::string> vec;
	GetFilesInDirectoryRecursive(dir, ext, vec);
	return vec;
}

void RegisterFileFunctions(lua_State* lua)
{

    luaL_newmetatable(lua, metaTableName);

    lua_pushliteral(lua, "__gc");
    lua_pushcfunction(lua, fileGC);
    lua_rawset(lua, -3);

    lua_pushliteral(lua, "__index");
    lua_newtable(lua);

    lua_pushliteral(lua, "isEOF");
    lua_pushcfunction(lua, f_eof);
    lua_settable(lua, -3);

    lua_pushliteral(lua, "close");
    lua_pushcfunction(lua, f_close);
    lua_settable(lua, -3);


    lua_pushliteral(lua, "read");
    lua_pushcfunction(lua, f_read);
    lua_settable(lua, -3);

    lua_pushliteral(lua, "readLine");
    lua_pushcfunction(lua, f_readLine);
    lua_settable(lua, -3);

    lua_pushliteral(lua, "seek");
    lua_pushcfunction(lua, f_seek);
    lua_settable(lua, -3);

    lua_pushliteral(lua, "tell");
    lua_pushcfunction(lua, f_tell);
    lua_settable(lua, -3);

    lua_pushliteral(lua, "getSize");
    lua_pushcfunction(lua, f_getSize);
    lua_settable(lua, -3);


    lua_rawset(lua, -3);

    lua_pop(lua, 1);
}

luaL_Reg file_functions[] =
{
    {"Open", f_open},
    {"DirEntries", LuaCWrap(f_dirEntries)},
    {"Exists", LuaCWrap(FileExists)},
    {0, 0}
};