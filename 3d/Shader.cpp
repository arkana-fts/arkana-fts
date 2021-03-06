#include "Shader.h"

#include "3d/3d.h"
#include "3d/Math.h"
#include "logging/logger.h"
#include "main/Exception.h"
#include "utilities/NonCopyable.h"
#include "utilities/utilities.h"
#include "graphic/Color.h"
#include "dLib/dFile/dBrowse.h"
#include "dLib/dFile/dFile.h"

#include <iterator>
#include <map>
#include <vector>
#include <set>

using namespace FTS;

const String FTS::ShaderManager::DefaultVertexShader = "Default.vert";
const String FTS::ShaderManager::DefaultFragmentShader = "Default.frag";
const String FTS::ShaderManager::DefaultGeometryShader = "";
extern const char* sErrorVert;
extern const char* sErrorFrag;

#define D_SHADERS_DIRNAME "Shaders"

namespace FTS {
/// Abstract base-class that is used to independently either use native OpenGL
/// #include feature if available (GL_ARB_shading_language_include) or use a
/// workaround.
class ShaderIncludeManager {
public:
    /// default constructor
    ShaderIncludeManager() {};
    /// default destructor
    virtual ~ShaderIncludeManager() {};

    /// Adds a file to the list of files being searched for when an #include is
    /// met.
    ///
    /// \param in_sFileName The name of the file to add (and used in the
    ///                     #include directive in the shaders).
    /// \param in_sContent The content of the file.
    ///
    /// \return true if successful, false if not.
    /// \see NamedStringARB in the OpenGL extension registry.
    virtual bool addIncludeFile(const Path& in_sFileName, const String& in_sContent) = 0;

    /// This just calls the compile shader process. It might make some other
    /// stuff before or after the compilation, to get the includes done.
    ///
    /// \param in_id The id of the shader.
    /// \param in_sShaderName The name of the shader, for diagnostic purposes.
    /// \param in_sSrc The source code of the shader.
    ///
    /// \return An empty string if nothing special happened. You still have to
    ///         call the usual OpenGL diagnostic stuff. If the string is not
    ///         empty, an error occured in the pre/post processing and the
    ///         error message is returned.
    virtual String compileShader(const GLuint in_id, const String& in_sShaderName, String in_sSrc, const ShaderCompileFlags& in_flags) = 0;
};

std::size_t findVersionLineEnd(const String& code)
{
    // As the version string has to be the FIRST line, that's it!
    return code.find("\n");
}

String injectOptionDefines(const String& code, const ShaderCompileFlags& options)
{
    if(options.empty())
        return code;

    std::size_t endOfVersionLine = findVersionLineEnd(code);
    String realCode = code.left(endOfVersionLine + 1);
    realCode += join("#define ", options.flags().begin(), options.flags().end(), "\n");
    realCode += String(code, endOfVersionLine);
    return realCode;
}

String injectExtensionRequirement(const String& code, const String& name)
{
    if(name.empty())
        return code;

    std::size_t endOfVersionLine = findVersionLineEnd(code);
    String realCode = code.left(endOfVersionLine + 1);
    realCode += "#extension " + name + " : require\n";
    realCode += String(code, endOfVersionLine);
    return realCode;
}

/// This is the concrete ShaderIncludeManager that we want to use exclusively
/// in the future, as it uses the native OpenGL include extension.
class ShaderIncludeManagerNativeGL : public ShaderIncludeManager {
public:
    /// default constructor
    ShaderIncludeManagerNativeGL()
    {
        verifGL("ShaderIncludeManagerNativeGL::ShaderIncludeManagerNativeGL start");
        glNamedStringARB = (t_glNamedStringARB)FTS::glGetProcAddress("glNamedStringARB");
        glDeleteNamedStringARB = (t_glDeleteNamedStringARB)FTS::glGetProcAddress("glDeleteNamedStringARB");

        if(!glNamedStringARB || !glDeleteNamedStringARB || !String((const char *)glGetString(GL_EXTENSIONS)).contains("GL_ARB_shading_language_include"))
            throw NotExistException("GL_ARB_shading_language_include", "OpenGL extension");

        FTSMSGDBG("Using native GL include manager", 2);
        verifGL("ShaderIncludeManagerNativeGL::ShaderIncludeManagerNativeGL end");
    }

    /// default destructor
    virtual ~ShaderIncludeManagerNativeGL()
    {
        verifGL("ShaderIncludeManagerNativeGL::~ShaderIncludeManagerNativeGL start");
        for(std::list<Path>::iterator i = m_names.begin() ; i != m_names.end() ; ++i) {
            glDeleteNamedStringARB(-1, i->c_str());
        }
        verifGL("ShaderIncludeManagerNativeGL::~ShaderIncludeManagerNativeGL end");
    }

    virtual bool addIncludeFile(const Path& in_sFileName, const String& in_sContent)
    {
        // Clear it.
        glGetError();
        String name = "/" + in_sFileName;
        glNamedStringARB(SHADER_INCLUDE_ARB, -1, name.c_str(), -1, in_sContent.c_str());
        if(glGetError() != GL_NO_ERROR)
            return false;
        m_names.push_back(name);

        return true;
    }

    virtual String compileShader(const GLuint in_id, const String& in_sShaderName, String in_sSrc, const ShaderCompileFlags& in_flags)
    {
        verifGL("ShaderIncludeManagerNativeGL::compileShader start");

        // We split0 up the source into:
        //  - the version line
        //  - the extension specifications
        //  - the option defines
        //  - all the rest of the sourcecode
        String sRealSource = injectExtensionRequirement(in_sSrc, "GL_ARB_shading_language_include");
        String sRealSourceWithOptions = injectOptionDefines(sRealSource, in_flags);
        // Note that we *need* those two String objects above, if we'd make it
        // a one-liner, the memory src points at would've gone here.
        const char *src = sRealSourceWithOptions.c_str();
        FTSMSGDBG("Compiling shader " + in_sShaderName + " sourcecode:\n" + sRealSourceWithOptions.left(500) + " ...", 3);
        glShaderSource(in_id, 1, &src, NULL);
        glCompileShader(in_id);

        verifGL("ShaderIncludeManagerNativeGL::compileShader end");
        return String::EMPTY;
    }
private:
    std::list<Path> m_names;

    static const GLenum SHADER_INCLUDE_ARB = 0x8DAE;

    typedef void (APIENTRYP t_glNamedStringARB)(GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar* string);
    typedef void (APIENTRYP t_glDeleteNamedStringARB)(GLint namelen, const GLchar* name);
    t_glNamedStringARB glNamedStringARB;
    t_glDeleteNamedStringARB glDeleteNamedStringARB;
};

/// This is the concrete ShaderIncludeManager that we will use most of the time
/// right now (april 2010) as there is no graphics card driver out yet that
/// supports the native OpenGL include extension. This kind-of does it by hand:
/// it just replaces the #include line with the file's source, also adding #line
/// and #file preprocessors in order to still get valid compiler messages.
class ShaderIncludeManagerWorkaround : public ShaderIncludeManager {
public:
    /// default constructor
    ShaderIncludeManagerWorkaround() { FTSMSGDBG("Using workaround GL include manager", 2); };
    /// default destructor
    virtual ~ShaderIncludeManagerWorkaround() {};

    virtual bool addIncludeFile(const Path& in_sFileName, const String& in_sContent)
    {
        m_names[in_sFileName] = in_sContent;
        return true;
    }

    virtual String compileShader(const GLuint in_id, const String& in_sShaderName, String in_sSrc, const ShaderCompileFlags& in_flags)
    {
        verifGL("ShaderIncludeManagerWorkaround::compileShader start");

        try {
            size_t start = 0, end = 0, line = 0;
            String sName;

            std::set<String> namesDone;

            // We replace all #include lines until there is no more left.
            while(!(sName = this->findIncludeLine(in_sShaderName, in_sSrc, start, end, line)).empty()) {
                // Remove any leading slashes for this workaround.
                sName = sName.trimLeft("/\\");

                // Check if it is registered.
                if(m_names.find(sName) == m_names.end()) {
                    throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                        ": Can't find the include file \""+sName+"\""
                        "preprocessor: There should only be whitespace trailing the #include");
                }
                // As we do not preprocess the #ifdefs, we need to somehow avoid
                // cyclic (infinite) inclusion. We do this by keeping a list of
                // already included ones. This is not ideal but this is a huge
                // workaround anyways.
                if(namesDone.find(sName) != namesDone.end()) {
                    in_sSrc.replaceStr(start, end - start, "");
                    continue;
                }
                namesDone.insert(sName);

                // And now we gotta insert it into the source code string.
                String sCode = "#line 0";
                sCode += "\n" + m_names[sName] + "\n";
                //String sCode = "\n" + m_names[sName] + "\n";
                sCode += "#line " + String::nr(line);// + " " + String::nr(in_id);
                in_sSrc.replaceStr(start, end - start, sCode);
            }
        } catch(const ArkanaException& e) {
            return e.what();
        }

        // Note that we *need* this String object, if we'd make it
        // a one-liner, the memory src points at would disappear too fast.
        String source = injectOptionDefines(in_sSrc, in_flags);
        const char *src = source.c_str();
        FTSMSGDBG("Compiling shader " + in_sShaderName + " sourcecode:\n" + source.left(500) + " ...", 3);
        glShaderSource(in_id, 1, &src, NULL);
        glCompileShader(in_id);

        verifGL("ShaderIncludeManagerWorkaround::compileShader end");
        return String::EMPTY;
    }
private:
    std::map<Path, String> m_names;

    /// This method finds a line of the form #include "bla" in the source string.
    ///
    /// \param in_sShaderName The name of the shader being parsed, for diagnostic.
    /// \param in_sSrc The source string where to search the include line in.
    /// \param out_start The position in the string where the # sign is found.
    /// \param out_end The position in the string right before the next newline.
    /// \param out_line The line number of the found #include.
    ///
    /// \note Both output parameters are left unchanged if there is an error in
    ///       the include line or no include line has been found.
    ///
    /// \return The string between the "" of the include line. Empty string if
    ///         no include line was found.
    ///
    /// \throw CorruptDataException If something's wrong with the include line.
    String findIncludeLine(const String& in_sShaderName, const String& in_sSrc, size_t& out_start, size_t& out_end, size_t& out_line)
    {
        size_t pos = (size_t)-1, line = 0;
        do {
            line++;
            pos++; // Go behind the newline.
        //for(size_t line = 1 ; pos != std::string::npos ; line++, pos = in_sSrc.find("\n", pos)) {
            // We need the '#' to be the first char of the line.
            if(in_sSrc.getCharAt(pos) != '#') {
                continue;
            }

            size_t start = pos;

            // now we might skip whitespace after the #.
            pos++;
            while(in_sSrc.getCharAt(pos) == ' ' || in_sSrc.getCharAt(pos) == '\t') pos++;

            // check for the word "include".
            if(in_sSrc.mid(pos, 0).left(7) != "include")
                continue;

            // skip whitespace again.
            pos += 7;
            while(in_sSrc.getCharAt(pos) == ' ' || in_sSrc.getCharAt(pos) == '\t') pos++;

            // From now on, anything else is an error, because we detected #include already.

            // get the name string.
            char quote = in_sSrc.getCharAt(pos);
            if(quote != '"' && quote != '<') {
                throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                    " : line "+String::nr(line)+": Syntax error in #include "
                    "preprocessor: you must use the \" or < sign to quote");
            }

            // Find the end of the string.
            pos++;
            size_t eos = std::string::npos;
            if(quote == '\"') {
                eos = in_sSrc.find("\"", pos);
            } else {
                eos = in_sSrc.find(">", pos);
            }
            if(eos == std::string::npos) {
                throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                    ": line "+String::nr(line)+": Syntax error in #include "
                    "preprocessor: missing an ending \" or > sign");
            }

            // Extract the include string.
            String name = in_sSrc.mid(pos, in_sSrc.len()-eos);
            pos = eos;

            // Check if there are only trailing whitespace but nothing else.
            pos++;
            while(in_sSrc.getCharAt(pos) == ' ' || in_sSrc.getCharAt(pos) == '\t') pos++;

            if(in_sSrc.getCharAt(pos) != '\r' && in_sSrc.getCharAt(pos) != '\n') {
                throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                    ": line "+String::nr(line)+": Syntax error in #include "
                    "preprocessor: There should only be whitespace trailing the #include");
            }

            out_start = start;
            out_end = pos;
            out_line = line;
            return name;
        } while((pos = in_sSrc.find("\n", pos)) != std::string::npos);

        return String::EMPTY;
    }

};

/// This is an internal class only to be used by the ShaderManager and nobody
/// else. The ShaderManager keeps a collection of these CompiledShader and
/// mixes them together (by linking) however the options state to.\n
class CompiledShader : public NonCopyable {
public:
    /// This compiles the shader of type \a in_type using the given sourcecode
    /// \a in_sSourceCode.
    ///
    /// \param in_sShaderName The name of the shader, for diagnostic purposes only.
    /// \param in_sSourceCode The source-code of the shader.
    /// \param in_type The type of the shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
    ///                GL_GEOMETRY_SHADER, ...)
    /// \param in_pIncMgr The include manager to use to compile the shader.
    /// \param in_flags The flags to use during shader compilation.
    ///
    /// \throws CorruptDataException When the shader cannot compile.
    CompiledShader(const String& in_sShaderName, const String& in_sSourceCode, GLuint in_type, ShaderIncludeManager* in_pIncMgr, const ShaderCompileFlags& in_flags)
    {
        verifGL("CompiledShader::CompiledShader start " + in_sShaderName);
        FTSMSGDBG("CompiledShader::CompiledShader: Preparing to compile " + in_sShaderName, 2);
        m_id = glCreateShader(in_type);
        String sErr = in_pIncMgr->compileShader(m_id, in_sShaderName, in_sSourceCode, in_flags);

        if(!sErr.empty()) {
            throw CorruptDataException(in_sShaderName, sErr);
        }

        // We always get the info log, it might contain some useful warnings!
        GLint loglen = 0;
        glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &loglen);
        std::vector<GLchar> pszLog(loglen);     // Deleted automatically.
        glGetShaderInfoLog(m_id, loglen, NULL, &pszLog[0]);
        m_sLog = &pszLog[0];

        if(!m_sLog.empty()) {
            FTSMSGDBG("Shader::CompiledShader: info-log of " + in_sShaderName + ":\n" + m_sLog, 2);
        }

        // If there was an error, we throw it as an exception.
        GLint bCompiled = GL_FALSE;
        glGetShaderiv(m_id, GL_COMPILE_STATUS, &bCompiled);
        if(bCompiled != GL_TRUE || m_id == 0) {
            throw CorruptDataException(in_sShaderName, m_sLog);
        }

        FTSMSGDBG("CompiledShader::CompiledShader: done with " + in_sShaderName + "; id = " + String::nr(m_id), 2);
        verifGL("CompiledShader::CompiledShader end " + in_sShaderName);
    };

    virtual ~CompiledShader()
    {
        if(m_id != 0) {
            verifGL("CompiledShader::~CompiledShader start");
            FTSMSGDBG("CompiledShader::CompiledShader: deleting " + String::nr(m_id), 2);
            glDeleteShader(m_id);
            verifGL("CompiledShader::~CompiledShader end");
        }
    };

    GLuint id() const {return m_id;};
private:
    GLuint m_id;   ///< Contains the OpenGL ID of the compiled shader.
    String m_sLog; ///< Might contain infos/warnings/errors about the shader.
};

} // namespace FTS

FTS::Program::Program(const CompiledShader& in_vert, const CompiledShader& in_frag, const String& in_sShaderName)
{
    verifGL("Program::Program: start of " + in_sShaderName);
    FTSMSGDBG("Program::Program: Preparing to link " + in_sShaderName + " using: " + String::nr(in_vert.id()) + ", " + String::nr(in_frag.id()), 2);

    m_id = glCreateProgram();
    glAttachShader(this->id(), in_vert.id());
    glAttachShader(this->id(), in_frag.id());
    glLinkProgram(this->id());

    verifGL("Program::Program: end of " + in_sShaderName);
    this->init(in_sShaderName);
}

FTS::Program::Program(const CompiledShader& in_vert, const CompiledShader& in_frag, const CompiledShader& in_geom, const String& in_sShaderName)
{
    verifGL("Program::Program: start of " + in_sShaderName);
    FTSMSGDBG("Program::Program: Preparing to link " + in_sShaderName + " using: " + String::nr(in_vert.id()) + ", " + String::nr(in_frag.id()) + ", " + String::nr(in_geom.id()), 2);

    m_id = glCreateProgram();
    glAttachShader(this->id(), in_vert.id());
    glAttachShader(this->id(), in_frag.id());
    glAttachShader(this->id(), in_geom.id());
    glLinkProgram(this->id());

    verifGL("Program::Program: end of " + in_sShaderName);
    this->init(in_sShaderName);
}

void FTS::Program::init(const String& in_sShaderName)
{
    verifGL("Program::init: start of " + in_sShaderName);

    // We always get the info log, it might contain some useful warnings!
    GLint loglen = 0;
    glGetProgramiv(this->id(), GL_INFO_LOG_LENGTH, &loglen);
    std::vector<GLchar> pszLog(loglen);     // Deleted automatically.
    glGetProgramInfoLog(this->id(), loglen, NULL, &pszLog[0]);
    m_sLog = &pszLog[0];

    if(!m_sLog.empty()) {
        FTSMSGDBG("Program::init: info-log of " + in_sShaderName + ":\n" + m_sLog, 2);
    }

    // If there was an error, we throw it as an exception.
    GLint bLinked = GL_FALSE;
    glGetProgramiv(this->id(), GL_LINK_STATUS, &bLinked);
    if(bLinked != GL_TRUE || this->id() == 0) {
        throw CorruptDataException(in_sShaderName, m_sLog);
    }

    // Say that the fragment shader "out" variable "Color" is the output to the screen (0).
    glBindFragDataLocation(this->id(), 0, "Color");

    verifGL("Program::init: link of " + in_sShaderName);
    FTSMSGDBG("Program::init: done with " + in_sShaderName + "; got id = " + String::nr(this->id()), 2);

    // If all that worked, we query all attributes and all uniforms that are
    // available in the program and store their informations.

    // First the attributes:
    GLint nAttribs = 0, nLongestAttrib = 0;
    glGetProgramiv(this->id(), GL_ACTIVE_ATTRIBUTES, &nAttribs);
    glGetProgramiv(this->id(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &nLongestAttrib);

    for(GLint i = 0 ; i < nAttribs ; ++i) {
        Attribute a;
        std::vector<GLchar> name(nLongestAttrib);
        glGetActiveAttrib(this->id(), i, nLongestAttrib, NULL, &a.size, &a.type, &name[0]);
        a.name = &name[0];

        // Wow, it took me hours to find out that the id isn't forcedly i!
        a.id = glGetAttribLocation(this->id(), a.name.c_str());

        m_attribs[a.name] = a;
        FTSMSGDBG("Program::init: found attrib " + a.name + " id = " + String::nr(a.id), 3);
    }

    // Then the uniforms:
    GLint nUniforms = 0, nLongestUniform = 0;
    glGetProgramiv(this->id(), GL_ACTIVE_UNIFORMS, &nUniforms);
    glGetProgramiv(this->id(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &nLongestUniform);

    for(GLint i = 0 ; i < nUniforms ; ++i) {
        GLsizei iActualLen = 0;
        Uniform u;
        std::vector<GLchar> name(nLongestUniform);
        glGetActiveUniform(this->id(), i, nLongestUniform, &iActualLen, &u.size, &u.type, &name[0]);
        u.name = &name[0];

        // Same story here as for the attribs... Crazy shit!
        u.id = glGetUniformLocation(this->id(), u.name.c_str());

        // In case it is an array, we get the id of every single array entry.
        // The name that was returned is defined to include the [0] by the standard.
        // 7.3.1.1 Naming Active Resources.
        if(u.size > 1 || u.name.right(3) == "[0]") {
            u.arrayIds.push_back(u.id);
            u.name = u.name.mid(0, 3);
            for(GLint i = 1 ; i < u.size ; ++i) {
                u.arrayIds.push_back(glGetUniformLocation(this->id(), (u.name + "[" + String::nr(i) + "]").c_str()));
            }
        }

        m_uniforms[u.name] = u;
        FTSMSGDBG("Program::init: found uniform " + u.name + " id = " + String::nr(u.id), 3);
    }

    verifGL("Program::init: end of " + in_sShaderName);
}

const FTS::Program::Attribute& FTS::Program::vertexAttribute(const String& in_sAttribName) const
{
    auto attrib = m_attribs.find(in_sAttribName);
    if(attrib == m_attribs.end())
        throw NotExistException("Attribute " + in_sAttribName, "Program no " + String::nr(m_id));

    return attrib->second;
}

bool FTS::Program::hasVertexAttribute(const String& in_sAttribName) const
{
    return m_attribs.find(in_sAttribName) != m_attribs.end();
}

/// This method binds a vertex buffer to a certain attribute of the shader.
/// \return true if the bind succeeded, false else.
bool FTS::Program::setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer)
{
    auto i = m_attribs.find(in_sAttribName);
    if(i == m_attribs.end())
        return false;

    verifGL("Program::setVertexAttribute("+in_sAttribName+") start");
    // Do some verifications
    if(in_buffer.type == GL_FLOAT) {
        switch(in_buffer.nComponents) {
        case 1:
            if(i->second.type != GL_FLOAT)
                return false;
            break;
        case 2:
            if(i->second.type != GL_FLOAT_VEC2)
                return false;
            break;
        case 3:
            if(i->second.type != GL_FLOAT_VEC3)
                return false;
            break;
        case 4:
            if(i->second.type != GL_FLOAT_VEC4)
                return false;
            break;
        default:
            return false;
        }
    }

    glEnableVertexAttribArray(i->second.id);
    in_buffer.bind();
    glVertexAttribPointer(i->second.id, in_buffer.nComponents, in_buffer.type, in_buffer.normalize, in_buffer.stride, NULL);

    /// \TODO: where to disable the vertex attribute?
    verifGL("Program::setVertexAttribute("+in_sAttribName+") end");
    return true;
}

bool FTS::Program::setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer, GLint in_nComponents, std::size_t in_offset)
{
    auto i = m_attribs.find(in_sAttribName);
    if(i == m_attribs.end())
        return false;

    verifGL("Program::setVertexAttribute(interleaved: "+in_sAttribName+") start");
    // Do some verifications
    if(in_buffer.type == GL_FLOAT) {
        switch(in_nComponents) {
        case 1:
            if(i->second.type != GL_FLOAT)
                return false;
            break;
        case 2:
            if(i->second.type != GL_FLOAT_VEC2)
                return false;
            break;
        case 3:
            if(i->second.type != GL_FLOAT_VEC3)
                return false;
            break;
        case 4:
            if(i->second.type != GL_FLOAT_VEC4)
                return false;
            break;
        default:
            return false;
        }
    }

    glEnableVertexAttribArray(i->second.id);
    in_buffer.bind();
    auto sizeof_ = in_buffer.stride/in_buffer.nComponents;
    glVertexAttribPointer(i->second.id, in_nComponents, in_buffer.type, in_buffer.normalize, in_buffer.stride, (const GLvoid*)(in_offset * sizeof_));

    /// \TODO: where to disable the vertex attribute?
    verifGL("Program::setVertexAttribute("+in_sAttribName+") end");
    return true;
}

const FTS::Program::Uniform& FTS::Program::uniform(const String& in_sUniformName) const
{
    auto uniform = m_uniforms.find(in_sUniformName);
    if(uniform == m_uniforms.end())
        throw NotExistException("Uniform " + in_sUniformName, "Program no " + String::nr(m_id));

    return uniform->second;
}

bool FTS::Program::hasUniform(const String& in_sUniformName) const
{
    auto i = m_uniforms.find(in_sUniformName);
    return i != m_uniforms.end();
}

bool FTS::Program::setUniform(const String& in_sUniformName, float in_v)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniform("+in_sUniformName+") start");
    if(i->second.type == GL_FLOAT) {
        glUniform1f(i->second.id, in_v);
        verifGL("Program::setUniform("+in_sUniformName+") end");
        return true;
    } else {
        verifGL("Program::setUniform("+in_sUniformName+") badend");
        return false;
    }
}

bool FTS::Program::setUniform(const String& in_sUniformName, const Vector& in_v)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniform("+in_sUniformName+") start");
    switch(i->second.type) {
    case GL_FLOAT:
        glUniform1f(i->second.id, in_v.x());
        break;
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.id, 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.id, 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.id, 1, in_v.array4f());
        break;
    default:
        verifGL("Program::setUniform("+in_sUniformName+") badend");
        return false;
    };

    verifGL("Program::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Program::setUniform(const String& in_sUniformName, const Color& in_c)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniform("+in_sUniformName+") start");
    switch(i->second.type) {
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.id, 1, in_c.array3f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.id, 1, in_c.array3f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.id, 1, in_c.array4f());
        break;
    default:
        verifGL("Program::setUniform("+in_sUniformName+") badend");
        return false;
    };

    verifGL("Program::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Program::setUniform(const String& in_sUniformName, const Quaternion& in_q)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniform("+in_sUniformName+") start");
    switch(i->second.type) {
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.id, 1, in_q.array4f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.id, 1, in_q.array4f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.id, 1, in_q.array4f());
        break;
    default:
        verifGL("Program::setUniform("+in_sUniformName+") badend");
        return false;
    };

    verifGL("Program::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Program::setUniform(const String& in_sUniformName, const General4x4Matrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniform("+in_sUniformName+") start");
    if(i->second.type != GL_FLOAT_MAT4) {
        verifGL("Program::setUniform("+in_sUniformName+") badend");
        return false;
    }

    glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16f());
    verifGL("Program::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Program::setUniform(const String& in_sUniformName, const AffineMatrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniform("+in_sUniformName+") start");
    if(i->second.type == GL_FLOAT_MAT3) {
        glUniformMatrix3fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array9f());
    } else if(i->second.type == GL_FLOAT_MAT4) {
        glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16f());
    } else {
        verifGL("Program::setUniform("+in_sUniformName+") badend");
        return false;
    }

    verifGL("Program::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Program::setUniformInverse(const String& in_sUniformName, const AffineMatrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniformInv("+in_sUniformName+") start");
    if(i->second.type == GL_FLOAT_MAT3) {
        glUniformMatrix3fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array9fInverse());
    } else if(i->second.type == GL_FLOAT_MAT4) {
        glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16fInverse());
    } else {
        verifGL("Program::setUniformInv("+in_sUniformName+") badend");
        return false;
    }

    verifGL("Program::setUniformInv("+in_sUniformName+") end");
    return true;
}

bool FTS::Program::setUniformInverse(const String& in_sUniformName, const General4x4Matrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniformInv("+in_sUniformName+") start");
    if(i->second.type != GL_FLOAT_MAT4) {
        verifGL("Program::setUniformInv("+in_sUniformName+") badend");
        return false;
    }

    glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16fInverse());
    verifGL("Program::setUniformInv("+in_sUniformName+") end");
    return true;
}

bool FTS::Program::setUniformSampler(const String& in_sUniformName, uint8_t in_iTexUnit)
{
    String sUniformName = in_sUniformName + String::chr(convertTextNr(in_iTexUnit));
    auto i = m_uniforms.find(sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Program::setUniformSampler("+in_sUniformName+") start");
    if(i->second.type != GL_SAMPLER_2D){
        verifGL("Program::setUniformSampler("+in_sUniformName+", "+String::nr(in_iTexUnit)+") badend");
        return false;
    }

    glUniform1i(i->second.id, (GLint)in_iTexUnit);
    verifGL("Program::setUniformSampler("+in_sUniformName+", "+String::nr(in_iTexUnit)+") end");
    return true;
}

bool FTS::Program::setUniformArrayElement(const String& in_sUniformName, size_t in_iArrayIdx, const Vector& in_v)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    if((GLint)in_iArrayIdx >= i->second.size) {
        FTSMSG("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+"): index out of bounds (max is "+String::nr(i->second.size)+")\n", MsgType::WarningNoMB);
        return false;
    }

    verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") start");
    switch(i->second.type) {
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.arrayIds[in_iArrayIdx], 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.arrayIds[in_iArrayIdx], 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.arrayIds[in_iArrayIdx], 1, in_v.array4f());
        break;
    default:
        verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") badend");
        return false;
    };

    verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") end");
    return true;
}

bool FTS::Program::setUniformArrayElement(const String& in_sUniformName, size_t in_iArrayIdx, const AffineMatrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    if( (GLint)in_iArrayIdx >= i->second.size) {
        FTSMSG("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+"): index out of bounds (max is "+String::nr(i->second.size)+")\n", MsgType::WarningNoMB);
        return false;
    }

    verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") start");
    if(i->second.type == GL_FLOAT_MAT3) {
        glUniformMatrix3fv(i->second.arrayIds[in_iArrayIdx], 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array9f());
    } else if(i->second.type == GL_FLOAT_MAT4) {
        glUniformMatrix4fv(i->second.arrayIds[in_iArrayIdx], 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16f());
    } else {
        verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") badend");
        return false;
    }

    verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") end");
    return true;
}

bool FTS::Program::setUniformArrayElementInverse(const String& in_sUniformName, size_t in_iArrayIdx, const AffineMatrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    if((GLint)in_iArrayIdx >= i->second.size) {
        FTSMSG("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+"): index out of bounds (max is "+String::nr(i->second.size)+")\n", MsgType::WarningNoMB);
        return false;
    }

    verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") start");
    if(i->second.type == GL_FLOAT_MAT3) {
        glUniformMatrix3fv(i->second.arrayIds[in_iArrayIdx], 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array9fInverse());
    } else if(i->second.type == GL_FLOAT_MAT4) {
        glUniformMatrix4fv(i->second.arrayIds[in_iArrayIdx], 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16fInverse());
    } else {
        verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") badend");
        return false;
    }

    verifGL("Program::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") end");
    return true;
}

GLuint Program::m_uiCurrentlyBoundShaderId = (GLuint)-1;

void FTS::Program::bind()
{
    // Do not re-bind the same shader again.
    if(this->id() == m_uiCurrentlyBoundShaderId)
        return;

    verifGL("Program::bind("+String::nr(this->id())+") start");
    glUseProgram(this->id());
    m_uiCurrentlyBoundShaderId = this->id();
    verifGL("Program::bind("+String::nr(this->id())+") end");
}

void Program::unbind()
{
    glUseProgram(0);
    m_uiCurrentlyBoundShaderId = (GLuint)-1;
}

FTS::Program::~Program()
{
    if(m_id != 0) {
        verifGL("Program::~Shader("+String::nr(this->id())+") start");
        glDeleteProgram(this->id());
        verifGL("Program::~Shader("+String::nr(this->id())+") end");
    }
}

const FTS::ShaderCompileFlag FTS::ShaderCompileFlag::Lit("D_LIT_OPTION");
const FTS::ShaderCompileFlag FTS::ShaderCompileFlag::Textured("D_TEXTURED_OPTION");
const FTS::ShaderCompileFlag FTS::ShaderCompileFlag::SkeletalAnimated("D_SKELETAL_ANIMATION_OPTION");

FTS::ShaderCompileFlag::ShaderCompileFlag(const String& name)
    : m_flag(name)
{ }

FTS::ShaderCompileFlag::ShaderCompileFlag(const String& name, const String& value)
    : m_flag(name)
    , m_value(value)
{ }

FTS::ShaderCompileFlags FTS::ShaderCompileFlag::operator|(const FTS::ShaderCompileFlag& f) const
{
    return ShaderCompileFlags(m_flag, f.m_flag);
}

FTS::String FTS::ShaderCompileFlag::name() const
{
    return m_flag;
}

FTS::String FTS::ShaderCompileFlag::value() const
{
    return m_value;
}

FTS::ShaderCompileFlag::operator std::string() const
{
    return this->name().str() + " " + this->value().str();
}

FTS::ShaderCompileFlags::ShaderCompileFlags()
{ }

FTS::ShaderCompileFlags::ShaderCompileFlags(const FTS::ShaderCompileFlag& flag)
{
    m_flags.push_back(flag.name());
}

FTS::ShaderCompileFlags::ShaderCompileFlags(const FTS::ShaderCompileFlag& flag, const FTS::ShaderCompileFlag& flag2)
{
    m_flags.push_back(flag.name());
    m_flags.push_back(flag2.name());
}

FTS::ShaderCompileFlags FTS::ShaderCompileFlags::operator|(const FTS::ShaderCompileFlag& f) const
{
    ShaderCompileFlags ret(*this);
    ret.m_flags.push_back(f);
    return ret;
}

FTS::ShaderCompileFlags FTS::ShaderCompileFlags::operator|(const FTS::ShaderCompileFlags& f) const
{
    ShaderCompileFlags ret(*this);
    ret.m_flags.insert(ret.m_flags.end(), f.m_flags.begin(), f.m_flags.end());
    return ret;
}

void FTS::ShaderCompileFlags::operator|=(const FTS::ShaderCompileFlag& f)
{
    m_flags.push_back(f);
}

void FTS::ShaderCompileFlags::operator|=(const FTS::ShaderCompileFlags& f)
{
    m_flags.insert(m_flags.end(), f.m_flags.begin(), f.m_flags.end());
}

const std::list<FTS::ShaderCompileFlag>& FTS::ShaderCompileFlags::flags() const
{
    return m_flags;
}

FTS::String FTS::ShaderCompileFlags::toString() const
{
    return join(m_flags.begin(), m_flags.end(), ":");
}

bool ShaderCompileFlags::empty() const
{
    return m_flags.empty();
}

FTS::ShaderManager::ShaderManager()
    : m_sep("|")
    , m_optSep(":")
{
    verifGL("ShaderManager::ShaderManager start");
    String sInfo;

    // Get some interesting informations.
    GLint i = 0, i2 = 0, i3 = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &i);
    sInfo += "\nMaximum vertex attributes: " + String::nr(i) + " (min: 16)";
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &i);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &i2);
    glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &i3);
    sInfo += "\nMaximum vertex/fragment/combined uniform components: " + String::nr(i) + "/" + String::nr(i2) + "/" + String::nr(i3) + " (min: 16, w00t)";

    FTSMSGDBG("ShaderManager::ShaderManager: some OpenGL informations: "+sInfo, 2);

    // We need that extension to work. This is only included in recent drivers!
    // Recent as of March/April 2010
    try {
        m_pInclManager = new ShaderIncludeManagerNativeGL();
    } catch(const ArkanaException&) {
        // If that extension is not available, we need to use our workaround.
        m_pInclManager = new ShaderIncludeManagerWorkaround();
    }

    // We need to preload all the include files we can find.
    std::vector<String> fileNames = dBrowse(Path::datadir(D_SHADERS_DIRNAME), "*.vertinc");
    for(auto&& sFile : fileNames) {
        this->loadShader(Path(sFile));
    }

    fileNames = dBrowse(Path::datadir(D_SHADERS_DIRNAME), "*.fraginc");
    for(auto&& sFile : fileNames) {
        this->loadShader(Path(sFile));
    }

    fileNames = dBrowse(Path::datadir(D_SHADERS_DIRNAME), "*.geominc");
    for(auto&& sFile : fileNames) {
        this->loadShader(Path(sFile));
    }

    // Already load, compile and link the default shader.
    this->loadShaderCode(DefaultVertexShader, sErrorVert);
    this->loadShaderCode(DefaultFragmentShader, sErrorFrag);
    CompiledShaderPtr pVert = this->compileShader(DefaultVertexShader);
    CompiledShaderPtr pFrag = this->compileShader(DefaultFragmentShader);
    String sDefaultShaderName = this->buildProgramName(DefaultVertexShader, DefaultFragmentShader, DefaultGeometryShader, ShaderCompileFlags());
    m_linkedShaders[sDefaultShaderName] = new Program(*pVert, *pFrag, sDefaultShaderName);
    verifGL("ShaderManager::ShaderManager end");
}

FTS::ShaderManager::~ShaderManager()
{
    verifGL("ShaderManager::~ShaderManager start");
    FTSMSGDBG("ShaderManager::~ShaderManager", 2);
    SAFE_DELETE(m_pInclManager);

    String sWarning = "The following shader programs haven't been unloaded:\n";

    for(auto i = m_linkedShaders.begin() ; i != m_linkedShaders.end() ; ++i) {
        // Do not delete anyone referencing the default program, as it may
        // be referenced by several entries, resulting in multi-deletes.
        if(i->second == this->getDefaultProgram())
            continue;

        sWarning += "    -> " + i->first + "\n";
        delete i->second;
    }

    if(m_linkedShaders.size() > 1) {
        FTSMSGDBG("ShaderManager::~ShaderManager: " + sWarning, 2);
    }

    // But don't forget to delete it anyways.
    delete getDefaultProgram();

    verifGL("ShaderManager::~ShaderManager end");
}

void ShaderManager::destroyProgramsUsing(const FTS::String& in_sShaderName)
{
    std::set<String> todestroy;

    // Collect the keys of all the programs to be destroyed.
    for(auto program = m_linkedShaders.begin() ; program != m_linkedShaders.end() ; ++program) {
        std::vector<String> names;
        program->first.split(std::inserter(names, names.begin()), m_sep);
        for(auto name = names.begin() ; name != names.end() ; ++name) {
            if(*name == in_sShaderName) {
                todestroy.insert(program->first);
            }
        }
    }

    // And then destroy them all.
    for(auto progName = todestroy.begin() ; progName != todestroy.end() ; ++progName) {
        delete m_linkedShaders[*progName];
        m_linkedShaders.erase(*progName);
    }
}

GLuint guessShaderType(const Path& name)
{
    String ext = name.ext();
    if(ext == "vert") {
        return GL_VERTEX_SHADER;
    } else if(ext == "frag") {
        return GL_FRAGMENT_SHADER;
    } else if(ext == "geom") {
        return GL_GEOMETRY_SHADER;
    }

    throw InvalidCallException("guessShaderType with unknown type. Shader name is: " + name);
}

void FTS::ShaderManager::loadShader(const Path& in_sFile, String in_sShaderName)
{
    if(in_sShaderName.empty()) {
        in_sShaderName = in_sFile;
    }

    this->loadShaderCode(in_sShaderName, File::open(Path::datadir(D_SHADERS_DIRNAME) + in_sFile, File::Read)->readstr());
}

void FTS::ShaderManager::loadShaderCode(String in_sShaderName, String in_sShaderContent)
{
    // Treat any unknown shader type as shader include file.
    try {
        guessShaderType(in_sShaderName);
        m_ShaderSources[in_sShaderName] = in_sShaderContent;
    } catch(const InvalidCallException&) {
        String includeShaderName = in_sShaderName;
        includeShaderName.replaceStr(D_SHADERS_DIRNAME, "");
        m_pInclManager->addIncludeFile(includeShaderName, in_sShaderContent);
    }
}

bool FTS::ShaderManager::hasShader(const String& in_sShaderName)
{
    return m_ShaderSources.find(in_sShaderName) != m_ShaderSources.end();
}

void FTS::ShaderManager::unloadShader(const String& in_sShaderName)
{
    m_ShaderSources.erase(in_sShaderName);
}

CompiledShaderPtr FTS::ShaderManager::compileShader(const String& in_sShaderName, const ShaderCompileFlags& flags)
{
    if(in_sShaderName.empty())
        return CompiledShaderPtr();

    auto src = m_ShaderSources.find(in_sShaderName);

    // If it hasn't been loaded, try to load it from disk.
    if(src == m_ShaderSources.end()) {
        this->loadShader(in_sShaderName);
        src = m_ShaderSources.find(in_sShaderName);
    }

    return CompiledShaderPtr(new CompiledShader(in_sShaderName, src->second, guessShaderType(in_sShaderName), m_pInclManager, flags));
}

String FTS::ShaderManager::getCompiledShaderSource(const String& in_sShaderName, const ShaderCompileFlags& flags)
{
    verifGL("ShaderManager::getCompiledShaderSource(" + in_sShaderName + ", " + flags.toString() + ") start");
    CompiledShaderPtr shader = this->compileShader(in_sShaderName, flags);
    GLuint id = shader->id();
    GLint srclen = 0;
    glGetShaderiv(id, GL_SHADER_SOURCE_LENGTH, &srclen);
    GLint realsrclen = 0;
    std::vector<GLchar> pszSrc(srclen);
    glGetShaderSource(id, srclen, &realsrclen, &pszSrc[0]);
    verifGL("ShaderManager::getCompiledShaderSource(" + in_sShaderName + ", " + flags.toString() + ") end");
    return String(&pszSrc[0]);
}

FTS::Program* FTS::ShaderManager::getOrLinkProgram(const String& in_sVertexShader, const String& in_sFragmentShader, const String& in_sGeometryShader, const ShaderCompileFlags& flags)
{
    // Check if we got that one cached (linked) already?
    String sLinkedShaderName = this->buildProgramName(in_sVertexShader, in_sFragmentShader, in_sGeometryShader, flags);
    auto iCached = m_linkedShaders.find(sLinkedShaderName);
    if(iCached != m_linkedShaders.end())
        return iCached->second;

    try {
        // If it's not been linked in this combination yet, do this now!
        CompiledShaderPtr pVert = this->compileShader(in_sVertexShader, flags);
        CompiledShaderPtr pFrag = this->compileShader(in_sFragmentShader, flags);
        CompiledShaderPtr pGeom = this->compileShader(in_sGeometryShader, flags);

        // Link them. If they don't fit, replace it by the default ones linked.
        if(pGeom) {
            return m_linkedShaders[sLinkedShaderName] = new Program(*pVert, *pFrag, *pGeom, sLinkedShaderName);
        } else {
            return m_linkedShaders[sLinkedShaderName] = new Program(*pVert, *pFrag, sLinkedShaderName);
        }
    } catch(const ArkanaException& e) {
        // Use the all-default shader in case of failure.
        e.show();

        return m_linkedShaders[sLinkedShaderName] = this->getDefaultProgram();
    }
}

Program* ShaderManager::getDefaultProgram()
{
    String defaultName = this->buildProgramName(DefaultVertexShader, DefaultFragmentShader, DefaultGeometryShader, ShaderCompileFlags());
    return m_linkedShaders[defaultName];
}

String ShaderManager::buildShaderName(FTS::String baseName)
{
    // If the file is located within the shaders directory (or somewhere deeper)
    // we take out the whole shaders directory prefix so we kindo got "relative"
    // names to a kind of "include directory".
    return baseName.replaceStr(D_SHADERS_DIRNAME, "");
}

String ShaderManager::buildProgramName(const FTS::String& vtxBaseName, const FTS::String& fragBaseName, const FTS::String& geomBaseName, const FTS::ShaderCompileFlags& flags)
{
    return this->buildShaderName(vtxBaseName) + m_sep + this->buildShaderName(fragBaseName) + m_sep + this->buildShaderName(geomBaseName) + join(m_optSep, flags.flags().begin(), flags.flags().end());
}
