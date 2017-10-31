#include "shader.hpp"
#include "uniformMap.hpp"
#include "log.hpp"

#include "gfxFlags.hpp"
#include "fileOperations.hpp"
#include <gbvfs.hpp>

#include "oGL.hpp"
#include "stringUtil.hpp"

#include <sstream>
#include <unordered_set>

class ShaderPreProcessor
{
    class State
    {
    public:
        std::string file;
        int line;
    };
    std::vector<State> stateStack;
    std::unordered_set<std::string> filesInProcesing;
    std::unordered_set<std::string> once;
    std::string output;
    bool error = false;

    void printInFileMsg()
    {
        if (stateStack.size() != 0)
        {
            Log << stateStack.rbegin()->file << " line " << stateStack.rbegin()->line;
        }
        else
            Log << "(internal)";
    }
public:



    bool failed()
    {
        return error;
    }
    ShaderPreProcessor()
    {
        output.reserve(4096);
    }

    const char* getOutput()
    {
        return output.c_str();
    }

    void processFile(const std::string& fn, const std::vector<std::string>* prefix, const char* additionalPrefix, bool isFirst)
    {
        
        std::string cfn;
        if (stateStack.size() > 0)
        {
            cfn = GetPathRelativeTo(fn, stateStack.rbegin()->file);
            cfn = CanonicalizePath(cfn);
            if (!FileExists(cfn))
            {
                cfn = CanonicalizePath(fn);
            }
        }
        else
            cfn = CanonicalizePath(fn);
        
        if (once.find(cfn) != once.end())
            return;

        if (filesInProcesing.find(cfn) != filesInProcesing.end())
        {
            Log << "Circular file reference to "<< cfn << " at ";
            printInFileMsg();

            Log << Trace(CHash("Warning"));
            return;
        }
        

        filesInProcesing.insert(cfn);
        State st;
        st.file = cfn;
        st.line = 1;
        stateStack.push_back(st);

        State* state = &(*stateStack.rbegin());

        auto sfile = GetFileStream(cfn);
        if (!sfile)
        {
            error = true;
            Log << "Failed to load shader - could not open file '" << cfn << "'" << Trace(CHash("Warning"));
            return;
        }


        if (!isFirst)
        {
            output += "#line 1\n";
        }

        int lineNum = 0;
        while (!sfile->isEOF())
        {
            lineNum += 1;
            auto str = sfile->readLine();
            if (str.size() > 0 && str[0] == '#')
            {
                std::stringstream ss(str);
                std::string tag;
                ss >> tag;
                if (tag == "#version")
                {
                    if (isFirst)
                    {

                        output += str;
                        output += '\n';
                        if (additionalPrefix)
                        {
                            output += additionalPrefix;
                            output += '\n';
                        }

                        if (prefix)
                        for (auto& p : *prefix)
                        {
                            output += p;
                            output += '\n';
                        }
                        isFirst = false;
                    }
                    output += "#line " + std::to_string(lineNum + 1) +"\n";
                    continue;
                }
                else if (tag == "#pragma")
                {
                    ss >> tag;
                    if (tag == "once")
                    {
                        once.insert(cfn);
                        output += "#line " + std::to_string(lineNum + 1) + "\n";
                        continue;
                    }
                    else if (tag == "include")
                    {
                        std::string sec;
                        std::getline(ss, sec);
                        StringUtil::Trim(sec);
                        /*
                        auto pos = str.find(' ');
                        if (pos == str.npos)
                        {
                            Log << "Failed to load shader - Invalid #include in ";
                            printInFileMsg();
                            Log << Trace(CHash("Warning"));
                            error = true;
                            return;
                        }
                        std::string sec = str.substr(pos);
                        */
                        state->line = lineNum;
                        processFile(sec, nullptr, nullptr, false);
                        if (error)
                            return;

                        state = &(*stateStack.rbegin());

                        output += "#line " + std::to_string(lineNum + 1)+ "\n";
                        continue;
                    }
                }

            }

            output += str;
            filesInProcesing.erase(cfn);
            output += '\n';
        }
        stateStack.pop_back();
    }
};



void ShaderMap::initializeElement(Shader* s)
{

    s->load(settings);
}

void ShaderMap::deInitializeElement(Shader* s)
{
    s->unLoad();
}

int Shader::loadAndCompile(GFXFlags* settings, std::string fn, int type)
{
    ShaderPreProcessor spp;
    const char* additionalPrefix = nullptr;

    #ifndef COPPERY_HEADLESS
    if (type == GL_FRAGMENT_SHADER)
        additionalPrefix = "#define FRAGMENT";
    else if (type == GL_VERTEX_SHADER)
        additionalPrefix = "#define VERTEX";
    else if (type == GL_GEOMETRY_SHADER)
        additionalPrefix = "#define GEOMETRY";
    #endif
    
    spp.processFile(fn, &(settings->getShaderPrefix()), additionalPrefix, true);
    if (!spp.failed())
    {

        #ifndef COPPERY_HEADLESS
        unsigned int shader = glCreateShader(type);
        const char* e = spp.getOutput();
        glShaderSource(shader, 1, &e, 0);
        glCompileShader(shader);

        char* buf = new char[2560];

        int loglen;
        glGetShaderInfoLog(shader,2559,&loglen,buf);
        

        //If we received a message, print it
        if (loglen > 0)
        {
            //Ensure null terminated string
            buf[loglen] = 0;
            Log << "Shader "<< fn << ": \n" << buf << Trace(CHash("Warning"));
        }

        delete[] buf;
        return shader;

        #else
        return 1;
        #endif
        
    }
    return -1;
}

void Shader::load(GFXFlags* settings)
{
    Log << "Building shader program \"" << frag << "\", \"" << vert << "\" and \"" << geom << "\"" << Trace(CHash("ShaderInfo"));
    if (! (hasFrag && hasVert))
    {
        Log << "  Incomplete shader" << Trace(CHash("ShaderInfo"));
        return;
    }
    fid = gid = vid = -1;

    #ifdef COPPERY_HEADLESS
    #define GL_FRAGMENT_SHADER 0
    #define GL_VERTEX_SHADER 0
    #define GL_GEOMETRY_SHADER 0
    #endif
    fid = loadAndCompile(settings, frag, GL_FRAGMENT_SHADER);
    vid = loadAndCompile(settings, vert, GL_VERTEX_SHADER);
    if (hasGeom)
        gid = loadAndCompile(settings, geom, GL_GEOMETRY_SHADER);

    if (hasFrag && fid == -1) hasFrag = false;
    if (hasVert && vid == -1) hasVert = false;
    if (hasGeom && gid == -1) hasGeom = false;

    #ifndef COPPERY_HEADLESS
    pid = glCreateProgram();
    glAttachShader(pid,fid);
    glAttachShader(pid,vid);
    if (hasGeom)
        glAttachShader(pid,gid);
    #else
    pid = 1;
    #endif

    if (feedbackOptions.varyings.size() > 0)
    {
        Log << "  Transform Feedback Varyings" << Trace(CHash("ShaderInfo"));
        std::vector<const char*> temp;
        for (const std::string& str : feedbackOptions.varyings)
        {
            temp.push_back(str.c_str());
            Log << "    " << str << Trace(CHash("ShaderInfo"));
        }

        #ifndef COPPERY_HEADLESS
        glTransformFeedbackVaryings(pid, temp.size(), temp.data(), GL_INTERLEAVED_ATTRIBS);
        #endif
    }

    #ifndef COPPERY_HEADLESS
    glLinkProgram(pid);
    hasProgram = true;

    auto buf = new char[2560];
    int d;
    glGetProgramInfoLog(pid, 2559, &d, buf);
    buf[d] = 0;
    if (d != 0)
        Log << "Shader Program: \n" << buf << Trace(CHash("Warning"));
    
    delete[] buf;
    #endif

    uniformsByHash.clear();


    Log << "  FragmentId = " << fid << ", VertexId = " << vid << ", GeometryId = " << gid << ", ProgramId = " << pid << Trace(CHash("ShaderInfo"));


    #ifndef COPPERY_HEADLESS

    int count;
    glGetProgramiv(pid, GL_ACTIVE_UNIFORMS, &count);

    const int maxNameLength = 64;
    char name[maxNameLength];
    name[maxNameLength - 1] = 0;

    int len;
    int size;
    unsigned type;

    Log << "  GL_ACTIVE_UNIFORMS = " << count << Trace(CHash("ShaderInfo"));
    for (int i = 0; i < count; i++)
    {
        glGetActiveUniform(pid, i, maxNameLength - 1, &len, &size, &type, name);

        int location = glGetUniformLocation(pid, name);
        Log << "    " << name << ": type = " << type << ", size = " << size << ", location = " << location << Trace(CHash("ShaderInfo"));
        if (location < 0)
        {
            
            Log << "Failed to get uniform '" << name << "' from shader with '" << frag << "', '" << vert << "' and '" << geom << "'" << Trace(CHash("Warning"));
            continue;
        }
        Hash h = Hash(name);
        auto it = uniformsByHash.find(h);
        if (it == uniformsByHash.end())
        {
            uniformsByHash[h] = location;
        }
        else
        {
            Log << "Uniform name hash collision with '" << name << "' in shader with '" << frag << "', '" << vert << "' and '" << geom << "'" << Trace(CHash("Warning"));
        }
    }

    #define CheckUniform(a) uniforms.uloc ## a = glGetUniformLocation(pid, #a ); \
    uniforms.needs ## a = (uniforms.uloc ## a !=-1);

    CheckUniform(MatProjection);
    CheckUniform(Texture);
    CheckUniform(TextureNormal);
    CheckUniform(TextureSpecular);
    CheckUniform(TextureDepth);
    CheckUniform(TextureFOV);
    CheckUniform(Camera);
    CheckUniform(LightPos);
    CheckUniform(LightColor);
    CheckUniform(LightPower);
    CheckUniform(FadeColor);
    CheckUniform(FOVCaster);
    CheckUniform(FillAmount);
    CheckUniform(DefaultNormal);
    CheckUniform(LightFlags);
    CheckUniform(Size);
    CheckUniform(SpecularData);
    CheckUniform(WindowSize);

    #endif /* COPPERY_HEADLESS */

}
void Shader::unLoad()
{

    #ifndef COPPERY_HEADLESS
    if (hasFrag) glDeleteShader(fid);
    if (hasVert) glDeleteShader(vid);
    if (hasGeom) glDeleteShader(gid);
    if (hasProgram) glDeleteProgram(pid);

    #endif
}

void UniformMap::apply()
{
    for (auto& p : uniform1fs)
    {
        #ifndef COPPERY_HEADLESS
        glUniform1f(p.first, p.second);
        #endif
    }
}

int UniformMap::findUniform1f(Hash h, float f)
{
    if (forShader)
    {
        auto res = forShader->getUniform(h);
        if (res >= 0)
        {
            uniform1fs.push_back({res, f});
            return uniform1fs.size() - 1;
        }
    }
    return -1;
}

void UniformMap::setUniform1f(Hash h, float f)
{
    auto it = name_uniform1fs.find(h);
    if (it == name_uniform1fs.end())
    {
        int idx = findUniform1f(h, f);
        
        name_uniform1fs[h] = {idx, f};
    }
    else
    {
        it->second.second = f;
        if (it->second.first >= 0)
            uniform1fs[it->second.first].second = f;
    }
}
