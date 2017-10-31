#pragma once
#include <string>

namespace GBVFS
{
    //! File read access handle
	class File
	{

	public:
        //! Query the current position from the beginning of the file.
		virtual unsigned long tell() = 0;
        
        //! Seek to certain offset from the beggining of the file
		virtual void seek(unsigned long o) = 0;
        
        /*! \brief Read \p o amount of bytes to \p p
         * 
         * \return Bytes read
         */
		virtual size_t read(void* p, size_t o) = 0;
        
        //! Get the total size of file 
		virtual long getSize() = 0;
        
        //! Return if the end of file was reached
		virtual bool isEOF() = 0;
        
        //! Close the file 
		virtual void close() = 0;

        //! Utility for reading a single line
        std::string readLine()
        {
            std::string retVal;

            const size_t buflen = 16;
            char buf[buflen];
            size_t readlen = 0;
            while (!isEOF())
            {
                readlen = read((void*) buf, buflen);
                size_t eol = 0;
                for (; eol < readlen; eol++)
                {
                    if (buf[eol] == '\n')
                    {
                        break;
                    }
                }
                retVal.append(buf, eol);
                if (eol < readlen)
                {
                    seek(tell() - readlen + eol + 1);
                    return retVal;
                }
            }
            return retVal;
        }

        //! Virtual destructor
		virtual ~File() {};
	};


    //! File write access handle
    class FileWriter
    {

    public:
        
        //! Write \p o amoutn of bytes from \p p and return bytes written
        virtual size_t write(const void* p, size_t o) = 0;
        
        //! Close the handle
        virtual void close() = 0;

        //! Utility to write a single line
        void writeLine(const std::string& str)
        {
            write(static_cast<const void*>(str.c_str()), str.size());
            char endl = '\n';
            write(static_cast<const void*>(&endl), 1); 
        }

        //! Virtual destructor
        virtual ~FileWriter() {};
    };




}
