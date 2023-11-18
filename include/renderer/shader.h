#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "logger.h"

namespace smokey_bedrock_parser {
    class Shader {
    public:
        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------
        Shader(const char* vertex_path, const char* fragment_path) {
            std::string filepath = std::filesystem::current_path().string();
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertex_code;
            std::string fragment_code;
            std::ifstream vertex_shader_file;
            std::ifstream fragment_shader_file;
            // ensure ifstream objects can throw exceptions:
            vertex_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fragment_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try {
                // open files
                vertex_shader_file.open(filepath + "/" + vertex_path);
                fragment_shader_file.open(filepath + "/" + fragment_path);
                std::stringstream vShaderStream, fShaderStream;
                // read file's buffer contents into streams
                vShaderStream << vertex_shader_file.rdbuf();
                fShaderStream << fragment_shader_file.rdbuf();
                // close file handlers
                vertex_shader_file.close();
                fragment_shader_file.close();
                // convert stream into string
                vertex_code = vShaderStream.str();
                fragment_code = fShaderStream.str();
            }
            catch (std::ifstream::failure& e) {
                log::error("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: {}", e.what());
            }

            const char* vertex_shader_code = vertex_code.c_str();
            const char* fragment_shader_code = fragment_code.c_str();

            // 2. compile shaders
            uint32_t vertex, fragment;

            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vertex_shader_code, NULL);
            glCompileShader(vertex);

            CheckCompileErrors(vertex, "VERTEX");

            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fragment_shader_code, NULL);
            glCompileShader(fragment);

            CheckCompileErrors(fragment, "FRAGMENT");

            // shader Program
            renderer_id = glCreateProgram();
            glAttachShader(renderer_id, vertex);
            glAttachShader(renderer_id, fragment);
            glLinkProgram(renderer_id);

            CheckCompileErrors(renderer_id, "PROGRAM");

            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }

        // activate the shader
        void Bind() const {
            glUseProgram(renderer_id);
        }

        // utility uniform functions
        void SetBool(const std::string& name, bool value) const {
            glUniform1i(GetUniformLocation(name), (int)value);
        }

        void SetInt(const std::string& name, int value) const {
            glUniform1i(GetUniformLocation(name), value);
        }

        void SetFloat(const std::string& name, float value) const {
            glUniform1f(GetUniformLocation(name), value);
        }

        void SetVec2(const std::string& name, const glm::vec2& value) const {
            glUniform2fv(GetUniformLocation(name), 1, &value[0]);
        }
        void SetVec2(const std::string& name, float x, float y) const {
            glUniform2f(GetUniformLocation(name), x, y);
        }

        void SetVec3(const std::string& name, const glm::vec3& value) const {
            glUniform3fv(GetUniformLocation(name), 1, &value[0]);
        }
        void SetVec3(const std::string& name, float x, float y, float z) const {
            glUniform3f(GetUniformLocation(name), x, y, z);
        }

        void SetVec4(const std::string& name, const glm::vec4& value) const {
            glUniform4fv(GetUniformLocation(name), 1, &value[0]);
        }
        void SetVec4(const std::string& name, float x, float y, float z, float w) const
        {
            glUniform4f(GetUniformLocation(name), x, y, z, w);
        }

        void SetMat2(const std::string& name, const glm::mat2& mat) const {
            glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
        }

        void SetMat3(const std::string& name, const glm::mat3& mat) const {
            glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
        }

        void SetMat4(const std::string& name, const glm::mat4& mat) const {
            glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
        }
    private:
        uint32_t renderer_id;
        mutable std::unordered_map<std::string, GLint> uniform_cache;

        GLint GetUniformLocation(const std::string& name) const {
            if (uniform_cache.find(name) != uniform_cache.end())
                return uniform_cache[name];

            GLint location = glGetUniformLocation(renderer_id, name.c_str());
            uniform_cache[name] = location;

            return location;
        }
        // utility function for checking shader compilation/linking errors.
        void CheckCompileErrors(GLuint shader, std::string type) {
            GLint success;
            GLchar infoLog[1024];

            if (type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

                if (!success) {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    log::error("ERROR::SHADER_COMPILATION_ERROR of type: {}", type);
                    log::error(infoLog);
                }
            }
            else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);

                if (!success) {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    log::error("ERROR::PROGRAM_LINKING_ERROR of type: {}", type);
                    log::error(infoLog);
                }
            }
        }
    };
} // namespace smokey_bedrock_parser