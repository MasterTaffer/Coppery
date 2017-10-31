#include "sheet.hpp"
#include "log.hpp"
#include "texture.hpp"
#include <stb_image.h>

#include <cstring>
#include "fileOperations.hpp"

void TextureSheetMap::initializeElement(TextureSheet* t)
{
    t->load(assets);
}
void TextureSheetMap::deInitializeElement(TextureSheet* t)
{
    t->unLoad();
}
void TextureSheet::load(GraphicsData* assets)
{
    texture = new Texture();
    std::vector<char* > sdata;
    std::vector<char* > ddata;
    std::vector<char* > ndata;
    for (std::string fn : toAdd)
    {
        std::string d,s,n;
        s = GetTextureSpecularFileName(fn);
        d = GetTextureDiffuseFileName(fn);
        n = GetTextureNormalFileName(fn);
        char *sd, *dd, *nd;
        sd = dd = nd = 0;

        Vector2i ddim;
        int nk;
        if (FileExists(s))
        {
            size_t len;
            char* fdata = GetFileContentsCopy(s, &len);
            if (fdata)
            {
                sd = (char*) stbi_load_from_memory((unsigned char*)fdata, len, 
                    &ddim.x, &ddim.y, &nk, 4);
                delete[] fdata;
            }

            if (sd)
            {
                if (ddim != baseSize)
                {
                    stbi_image_free(sd);
                    sd = 0;
                }
            }
        }
        if (FileExists(d))
        {
            size_t len;
            char* fdata = GetFileContentsCopy(d, &len);
            if (fdata)
            {
                dd = (char*) stbi_load_from_memory((unsigned char*)fdata,len,&ddim.x,&ddim.y,&nk,4);
                delete[] fdata;
            }

            if (dd)
            {
                if (ddim != baseSize)
                {
                    Log << "Size mismatch at diffuse " << fn << " to sheet" << Trace(CHash("Warning"));
                    stbi_image_free(dd);
                    dd = 0;
                }
            }
        }
        if (FileExists(n))
        {
            size_t len;
            char* fdata = GetFileContentsCopy(n, &len);
            if (fdata)
            {
                nd = (char*) stbi_load_from_memory((unsigned char*)fdata,len,&ddim.x,&ddim.y,&nk,4);
                delete[] fdata;
            }

            if (nd)
            {
                if (ddim != baseSize)
                {
                    stbi_image_free(nd);
                    nd = 0;
                }
            }
        }

        sdata.push_back(sd);
        ddata.push_back(dd);
        ndata.push_back(nd);
    }
    for (size_t i = 0; i < toAddNames.size(); i++)
    {
        nameMap[Hash(toAddNames[i])] = i;
    }
    float dk = sqrt(ddata.size());
    size_t a = ceil(dk);
    size_t totcell = a*a;
    while (totcell < ddata.size())
    {
        a+=1;
        totcell = a*a;
    }

    // a = how many tiles fit per row and column

    Vector2i totDimensions = baseSize*a;

    unsigned int needSpace = totcell*baseSize.x*baseSize.y*4;

    char* texNSpace = new char[needSpace];
    char* texSSpace = new char[needSpace];
    char* texDSpace = new char[needSpace];

    char* texn = texNSpace;
    char* texs = texSSpace;
    char* texd = texDSpace;
    for (size_t i = 0; i < ddata.size(); i++)
    {
//        size_t s = baseSize.x*baseSize.y*4;

        size_t nextRow = a*baseSize.x*4;

        size_t rows = baseSize.x*4;
        size_t colc = baseSize.y;
        if (ndata[i])
        {
            for (size_t k = 0; k < colc; k++)
            {
                memcpy(texn+(nextRow*k),ndata[i]+k*rows,rows);
            }

            stbi_image_free(ndata[i]);
        }
        else
            for (size_t k = 0; k < colc; k++)
            {
                //Normal map neutral is not 0,0,0,0
                //The blue and alpha component must be 255, others must be 128
                char* nbase = texn+(nextRow*k);
                for (size_t i2 = 0; i2 < rows; i2++)
                {
                    if (i2 % 4 >= 2)
                        nbase[i2] = 255;
                    else
                        nbase[i2] = 128;
                }
                

            }

        if (ddata[i])
        {
            for (size_t k = 0; k < colc; k++)
            {
               memcpy(texd+(nextRow*k),ddata[i]+k*rows,rows);
            }
            stbi_image_free(ddata[i]);
        }
        else
            for (size_t k = 0; k < colc; k++)
            {
                memset(texd+(nextRow*k),0,rows);
            }

        if (sdata[i])
        {
            for (size_t k = 0; k < colc; k++)
            {
                memcpy(texs+(nextRow*k),sdata[i]+k*rows,rows);
            }
            stbi_image_free(sdata[i]);
        }
        else
            for (size_t k = 0; k < colc; k++)
            {
                memset(texs+(nextRow*k),16,rows);
            }

        int in = i+1;
        if (i/a != in/a)
        {
            texn += rows;
            texs += rows;
            texd += rows;

            texn += nextRow*(baseSize.y-1);
            texs += nextRow*(baseSize.y-1);
            texd += nextRow*(baseSize.y-1);
        }
        else
        {
            texn += rows;
            texs += rows;
            texd += rows;
        }

    }
    splits.x = a;
    splits.y = a;
    
    texture->setUseMipMaps(false);
    texture->loadFromMemory(assets,totDimensions,texDSpace,texNSpace,texSSpace);
    delete[] texDSpace;
    delete[] texNSpace;
    delete[] texSSpace;
}
void TextureSheet::unLoad()
{
    texture->unLoad();
    delete texture;
}

Vector2f TextureSheet::getUVSize()
{
    Vector2f tdim = texture->getDimensions();
    Vector2f ak = Vector2f(baseSize)/tdim;
    
    //A hack to remove bleeding, the UV coordinates are shifted
    //ever so slightly away from the neighboring tile coordinates
    //This is perhaps enough to remove bleeding on nearest interpolation
    Vector2f auxadd = {1.0f / splits.x, 1.0f / splits.y};
    auxadd /= 1000;
    ak -= auxadd;
    return ak;
}

Vector2f TextureSheet::getUVBase(int i)
{
    Vector2i base;
    base.y = i / splits.x;
    base.x = i % splits.x;

    Vector2f tk = base;
    tk =  tk / splits;
    
    //A hack to remove bleeding, see getUVSize
    Vector2f auxadd = {1.0f / splits.x, 1.0f / splits.y};
    auxadd /= 2000;
    tk += auxadd;
    return tk;
}

Texture* TextureSheet::getTexture()
{
    return texture;
}
