#pragma once
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <functional>
#include <cassert>
#include <map>
#include <unordered_map>


#include "gbvfs_file.hpp"


//! Very simple and limited Virtual File System
namespace GBVFS
{
	class Directory;
    
    //! Structure representing a file path
	struct Path
	{
        //! Set to true if the path climbs upwards in the file hierarchy
		bool climbsUp = false;
        
        //! Generate the path from null terminated c string
		void fromCStr(const char* c_str, bool allowClimb = false)
		{
			components.clear();
			const char* start = c_str;
			for (; *c_str; ++c_str)
			{
				if (*c_str == '/')
				{
					components.push_back(std::string(start, c_str));
					start = c_str + 1;
				}
				
			}
			if (start != c_str)
			{
				components.push_back(std::string(start, c_str));
			}
			size_t i = 0;
			size_t totalLen = 0;
			//Canonicalize

			climbsUp = false;
			size_t elements = 0;
            size_t str_elementIndex = 0;
            
			while (i < components.size())
			{
				//Trim whitespace
				std::string& str = components[i];
                
                //str_elementIndex is always one past the real index
                str_elementIndex++;

                //Remove the implicit whitespace trimming

/*				auto fto = str.find_first_not_of(' ');

				if (fto != std::string::npos)
				{
					if (fto != 0)
						str.erase(0, fto);

					fto = str.find_last_not_of(' ');
					if (fto != std::string::npos || fto != str.size() - 1)
						str.erase(fto + 1, str.size());

				}
				else
				{
					components.erase(components.begin() + i);
					continue;
				}
*/
                if (str.size() == 0)
                {
                    //The first element is empty: path begins with /
                    //or is an empty path
                    
                    if ((str_elementIndex != 1))
                    {
                        components.erase(components.begin() + i);
                        continue;
                    }
                }

				if (str.size() == 1 && str[0] == '.')
				{
					components.erase(components.begin() + i);
					continue;
				}
				if (str == "..")
				{

					if (elements > 0)
					{
						components.erase(components.begin() + i - 1, components.begin() + i + 1);
						i -= 1;
						elements -= 1;
						continue;
					}
					else
					{
						if (allowClimb)
						{
							climbsUp = true;
						}
						else
						{
							components.erase(components.begin() + i);
							continue;
						}
					}
				}
				else
				{

					elements += 1;
				}

				totalLen += components[i].size() + 1;
				i+=1;
                
			}
			fullPath.clear();
			fullPath.reserve(totalLen);
			componentOffsets.clear();

			for (size_t i = 0; i < components.size(); i++)
			{
				if (i >= 1)
					fullPath += '/';
    
				componentOffsets.push_back(fullPath.size());
				fullPath += components[i];

			}
		}

		//! Constructor from C string
		Path(const char* c_str)
		{
			fromCStr(c_str);
		}

		//! Constructor from std::string
		Path(const std::string& str)
		{
			fromCStr(str.c_str());
		}

		//! Default constructor
		Path()
		{

		}

		const char* c_str() const
		{
			return fullPath.c_str();
		}
		
		//! The canonicalized path
		std::string fullPath;
        
        //! Individual path components
		std::vector<std::string> components;
        
        //! Offsets of different components in the fullPath
		std::vector<size_t> componentOffsets;
	};

    //! Structure representing a read-only view to Path.
	struct PathView
	{
        //! Get the component count in the PathView
		size_t getComponentCount() const
		{
			return path->components.size() - offset;
		}

		//! Get the component at index
		std::string getComponent(size_t index) const
		{
			return path->components.at(index + offset);
		}

		//! Get the C string of a component at index
		const char* getComponentCString(size_t index) const
		{
			return path->components.at(index + offset).c_str();
		}

		/*! \brief Construct from another PathView
         * 
         * \param pview the pathview
         * \param addedOffset how many path components should be skipped
         */
		PathView(const PathView& pview, size_t addedOffset = 1)
		{
			path = pview.path;

			offset = pview.offset + addedOffset;
			
			assert(offset <= path->components.size());
		}

		/*! \brief Construct from Path
         * 
         * \param ppath the path
         * \param addedOffset how many path components should be skipped
         */
		PathView(const Path& ppath, size_t addedOffset = 0)
		{
			path = &ppath;
			offset = addedOffset;
			
			assert(offset <= path->components.size());
		}

		//! Get C string representation of the PathView
		const char* getCString() const
		{
			if (getComponentCount() == 0)
				return "";
			return path->fullPath.c_str() + path->componentOffsets.at(offset);
		}

	private:
		size_t offset;
		const Path* path;
	};

    //! Structure representing file information
	class FileEntry
	{
	public:
        //! Set to true if the file exists
		bool exists = false;
        //! Set to true if the file is in fact a directory
		bool directory = false;
        //! Set to true if the file is a read only file
		bool readOnly = true;

        //! Size of the file in bytes
		long fileSize = 0; 
        //! Last modified time stamp of the file in Unix time stamp format
		unsigned long lastModified = 0;
	};

	class VFS;

    /*! \brief Abstract interface for a virtual file system loader
     * 
     * Loads files and directories for the file system to use. The GBVFS
     * provides no inbuilt ways of loading files from the disk; the user must
     * provide their own Loader for that. 
     */
	class Loader
	{
	public:
        
        //! The callback function used for iterateEntries
		typedef std::function<void(const char*, const FileEntry&)> FileIteratorCallback;

        //! Iterates the entries found in the directory behind PathView
		virtual void iterateEntries(const PathView&, FileIteratorCallback fic) = 0;

        //! Gets a FileEntry for the file pointed by PathView
		virtual FileEntry getFileEntry(const PathView&) = 0;
        
        //! Gets a file handle for the file pointed by PathView
		virtual std::unique_ptr<File> openFile(const PathView&) = 0;
        
        //! Gets a file write access handle for the file pointed by PathView
		virtual std::unique_ptr<FileWriter> openFileWrite(const PathView&) = 0;

        //! Virtual destructor
		virtual ~Loader() {};
	};

	/*
	class Directory
	{
		std::map<std::string, FileEntry> 
	public:
		class DirectoryIterator
		{
			Directory* dir = nullptr;
			size_t offset = 0;
		public:
			DirectoryIterator& operator++()
			{
				++offset;
				return *this;
			}

			std::pair<std::string, FileEntry> operator*() const
			{
				return std::pair<std::string, FileEntry>();
			}

			bool operator==(const DirectoryIterator& other) const
			{
				return (dir == other.dir) && (offset == other.offset);
			}
		};

		void begin();
		void end();
	};
	*/
    
    //! The virtual filesystem
	class VFS
	{

		struct VFS_Directory
		{
			std::string name;
			std::vector< std::unique_ptr<VFS_Directory> > subDirectories;
			std::unordered_map<std::string, size_t> subDirectoriesByName;
            bool isReadOnly = true;

			std::vector<std::pair<int, Path>> loadersActive;
		};

		std::vector<std::unique_ptr<Loader>> loaders;

		VFS_Directory root;

		void forLoadersAtDirectory(const char* p, std::function<bool(Loader*, const PathView&, bool readOnly)>);
	public:
        
        //! Flags for mount()
        enum
        {
            WRITEACCESS = 1
        };
        
        
        //! Error and warning return values from mount()
        enum
        {
            ERROR_INVALID_LOADER = -10,
            ERROR_INVALID_LOADER_PATH = -11,
            ERROR_LOADER_PATH_NOT_DIRECTORY = -12,
            
            WARNING_LOADER_CONFLICTING_READ_ACCESS = 10
            
        };
        
        /*! \brief Get the real world path of the file based on registered loaders
         * 
         * The VFS works in virtual paths registered via mount function. When
         * the user queries a file from the VFS path, all the loaders mounted
         * to that directory will be checked for matches. The VFS translates
         * the virtual path to a loader specific path to do this.
         * 
         * This function returns the loader specific path for the \p path
         * specified. If the file doesn't exist in the virtual file system,
         * \p path is returned.
         */
		std::string getRealPath(const char* path);
        
        /*! \brief Register a filesystem loader to VFS
         * 
         * The loader ownership is transferred to VFS and it will be deleted
         * by VFS at exit. The return value is a unique identifier for the
         * loader that will be used in calls to \ref mount.
         * 
         * \return The unique identifier for that loader.
         */
		int registerLoader(std::unique_ptr<Loader>);
        
        /*! \brief Mounts a loader path to a virtual path
         * 
         * The path \p loaderDir will be canonicalized and passed to the 
         * Loader when querying the mounted directory. \p loader must be the
         * unique identifer returned by \ref registerLoader. \p virtualDir
         * is the path in the virtual file system that will be mounted to
         * point at \p loaderDir at the Loader.
         * 
         *```
         *  
         *  mount(1, "data/assets", "assets") 
         *  
         *```
         * 
         * creates a link from virtual folder "assets" to loader specific
         * folder "data/assets" for loader 1.
         * 
         * Mounting two loaders into the same folder can cause interesting
         * effects. If two loaders have been mounted to the same folder
         * and contain the same files, the loader mounted after the first
         * one will override the files in the VFS. Other files will work
         * as expected.
         * 
         * However when mounting a loader into a sub-directory within
         * an already mounted directory will cause undesired results.
         * The sub-directory mount point will be basically invisible to
         * calls such as getDirectory. Other interesting effects is that
         * the sub-directory loader will override the whole directory entry.
         * 
         *```
         *  
         *  mount(1, "", "data"); //mount loader 1 root to "data"
         *  
         *  mount(2, "", "data/assets"); //mount loader 2 root to "data/assets"
         *  
         *```
         * 
         * In the above case, if the loader 1 has an "assets" directory with
         * files in it, they will all be inaccessible due to loader 2 mount
         * point overriding the whole directory entry for VFS path
         * "data/assets".
         * 
         * In short, it is not wise to mount loaders to sub-directories
         * which are already mounted to something. If there is no such
         * nested mount points, all will work as desired, and colliding file
         * entries will be overridden by the last mount.
         * 
         * \param loader the value returned from \ref registerLoader
         * \param loaderDir the path where the mount points to
         * \param virtualDir the mount point path
         * \param flags used to mount the directory
         * \return 0 on success, negative values on failure.
         */
		int mount(int loader, const char* loaderDir, const char* virtualDir, int flags = 0);
        
        //! Get a file entry identified by virtual path \p path.
		FileEntry getFileEntry(const char* path);
        //! Open file identified by virtual path \p path for reading.
		std::unique_ptr<File> openFile(const char* path);
        //! Open file identified by virtual path \p path for writing
		std::unique_ptr<FileWriter> openFileWrite(const char* path);
        
        //! Get contents of a virtual filesystem directory
		std::map<std::string, FileEntry> getDirectory(const char* path);
        
        //! Gets a human readable description of mount() return code
        const char* getMountReturnCodeInformation(int i);
	};


	#ifdef GBVFS_IMPLEMENTATION

	int VFS::registerLoader(std::unique_ptr<Loader> l)
	{
		loaders.push_back(std::move(l));
		return loaders.size() - 1;
	}

	int VFS::mount(int loader, const char* loaderDir, const char* virtualDir, int flags)
	{
		if (loader < 0 || size_t(loader) >= loaders.size())
			return ERROR_INVALID_LOADER;
		Loader* ld = loaders[loader].get();

		Path loadp;
		//Load and allow climbing UP
		loadp.fromCStr(loaderDir, true);

		//Assert the existence of the directory
		FileEntry f = ld->getFileEntry(PathView(loadp));
		if (!f.exists)
			return ERROR_INVALID_LOADER_PATH;
		if (!f.directory)
			return ERROR_LOADER_PATH_NOT_DIRECTORY;
		
		Path virtp(virtualDir);

		VFS_Directory* target = &root;

        bool readOnly = true;
        if ((flags & WRITEACCESS) != 0)
            readOnly = false;
        
		//Traverse virtual file system for correct the path
		for (size_t i = 0; i < virtp.components.size(); i++)
		{

			auto it = target->subDirectoriesByName.find(virtp.components[i]);
			if (it == target->subDirectoriesByName.end())
			{

				//Non-existing directory, generate sub paths
				while (i < virtp.components.size())
				{
					target->subDirectoriesByName[virtp.components[i]] = target->subDirectories.size();
					VFS_Directory* newp = new VFS_Directory();
					target->subDirectories.push_back(std::unique_ptr<VFS_Directory>(newp));
					target = newp;
					++i;
				}
				break;
			}
			else
			{
				target = target->subDirectories[it->second].get();
			}
		}
		
		//If a loader has been already added to the directory
		if (target->loadersActive.size() > 0)
        {
            //If the old loader had a different read access
            if (target->isReadOnly != readOnly)
            {
                //restrict access
                target->isReadOnly = false;
                return WARNING_LOADER_CONFLICTING_READ_ACCESS;
            }
        }
        else
            target->isReadOnly = readOnly;
		target->loadersActive.push_back({loader, loadp});
		return 0;
	}

	void VFS::forLoadersAtDirectory(const char* path, std::function<bool(Loader*, const PathView&, bool)> cb)
	{
		Path loadp(path);
		VFS_Directory* target = &root;
		size_t offset = 0;
		for (; offset < loadp.components.size(); ++offset)
		{
			auto it = target->subDirectoriesByName.find(loadp.components[offset]);
			if (it == target->subDirectoriesByName.end())
			{
				break;
			}
			else
			{
				target = target->subDirectories[it->second].get();
			}
		}
		if (target->loadersActive.size() == 0)
		{
			return;
		}
		bool readOnly = target->isReadOnly;

		PathView loadpview(loadp, offset);
		for (auto it = target->loadersActive.rbegin(); it != target->loadersActive.rend(); it++)
		{
			std::string p = it->second.fullPath;
			if (p.size() > 0)
				p = p + "/" + loadpview.getCString();
			else
				p = loadpview.getCString();
			Path pathp;
			pathp.fromCStr(p.c_str(), true);

			if (cb(loaders[it->first].get(), PathView(pathp), readOnly))
			{
				break;
			}
		}
	}

	FileEntry VFS::getFileEntry(const char* path)
	{
		FileEntry f;
		forLoadersAtDirectory(path, [&](Loader* l, const PathView& pv, bool readOnly)
		{
			f = l->getFileEntry(pv);
			if (f.exists)
			{
				return true;
			}
			return false;
		});
		return f;
	}
	
	std::unique_ptr<File> VFS::openFile(const char* path)
	{
		std::unique_ptr<File> f;
		forLoadersAtDirectory(path, [&](Loader* l, const PathView& pv, bool)
		{
			f = l->openFile(pv);
			if (f != nullptr)
			{
				return true;
			}
			return false;
		});
		return f;
	}
	
	
	std::unique_ptr<FileWriter> VFS::openFileWrite(const char* path)
	{
		std::unique_ptr<FileWriter> f;
		forLoadersAtDirectory(path, [&](Loader* l, const PathView& pv, bool readOnly)
		{
            if (readOnly)
                return false;
                
			f = l->openFileWrite(pv);
			if (f != nullptr)
			{
				return true;
			}
			return false;
		});
		return f;
	}

	std::string VFS::getRealPath(const char* path)
	{
		std::string ret = path;
		forLoadersAtDirectory(path, [&](Loader* l, const PathView& pv, bool)
		{
			auto f = l->getFileEntry(pv);
			if (f.exists)
			{
				ret = pv.getCString();
				return true;
			}
			return false;
		});
		return ret;
	}

	std::map<std::string, FileEntry> VFS::getDirectory(const char* path)
	{
		std::map<std::string, FileEntry> map;


		forLoadersAtDirectory(path, [&](Loader* l, const PathView& pv, bool)
		{
			
			l->iterateEntries(pv, [&](const char* fn, const FileEntry& entry)
				{
					std::string str = std::string(fn);
					if (map.find(str) == map.end())
						map[str] = entry;
				});
			return false;
		});
		return map;
	}
	
	const char* VFS::getMountReturnCodeInformation(int i)
    {
        switch (i)
        {
            case 0: return "Success";
            case ERROR_INVALID_LOADER: return "Invalid Loader identifier";
            case ERROR_INVALID_LOADER_PATH: return "Loader path is invalid or directory does not exist";
            case ERROR_LOADER_PATH_NOT_DIRECTORY: return "Loader path not a directory";
            case WARNING_LOADER_CONFLICTING_READ_ACCESS: return "Conflicting read access specifier: mount point falling back to read only";
            default:;
        }
        return "Invalid error code";
    }

	#endif

}
