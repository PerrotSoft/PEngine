#include "../../Include/gnu/OpenGL.h"

#define P_ENGEN_GLM_STUB_ACTIVE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tiny_obj_loader.h>
#include <stb_easy_font.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include <map> 
#include <cmath> 
#include "../../Include/cfg/log.h"
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h> 
#include <stb_easy_font.h> 
#ifdef _WIN32
#define SHADERS_BASE_PATH "PEngine\\shaders\\"
#define BASE_PATH_ICON_PE "PEngine\\data\\icon.png"
#else
#define SHADERS_BASE_PATH "PEngine/shaders/"
#define BASE_PATH_ICON_PE "PEngine/data/icon.png"
#endif
namespace gnu {
    struct ImageData {
        int width = 0;
        int height = 0;
        int components = 0;
        std::vector<unsigned char> data;
    };

    glm::mat4 PEGLMesh::get_transform() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = model * glm::toMat4(rotation);
        model = glm::scale(model, scale);
        return model;
    }
    void PEGLMesh::translate(const glm::vec3& offset) {
        position += offset;
    }

    void PEGLMesh::rotate(const glm::quat& delta_rotation) {
        rotation = delta_rotation * rotation;
    }

    void PEGLMesh::set_scale(const glm::vec3& new_scale) {
        scale = new_scale;
    }

    void PEGLMesh::delete_gpu_resources() {
        if (VAO != 0) glDeleteVertexArrays(1, &VAO);
        if (VBO != 0) glDeleteBuffers(1, &VBO);
        if (EBO != 0) glDeleteBuffers(1, &EBO);
        VAO = VBO = EBO = 0;
        indexCount = 0;
    }

    void PEGLDelete_Shader_Program(PEGLShaderProgram& program) {
        if (program.programID != 0) {
            glDeleteProgram(program.programID);
            program.programID = 0;
        }
    }

    void PEGLDelete_Model(PEGLModel& model) {
        for (auto& mesh : model.meshes) {
            if (mesh.VAO != 0) glDeleteVertexArrays(1, &mesh.VAO);
            if (mesh.VBO != 0) glDeleteBuffers(1, &mesh.VBO);
            if (mesh.EBO != 0) glDeleteBuffers(1, &mesh.EBO);

            mesh.VAO = mesh.VBO = mesh.EBO = mesh.textureID = 0;
            mesh.indexCount = 0;
        }
        model.meshes.clear();
    }

    static std::string Read_File(const std::string& filePath) {
        std::ifstream fileStream(filePath, std::ios::in);
        if (!fileStream.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }
        std::stringstream sstr;
        sstr << fileStream.rdbuf();
        fileStream.close();
        return sstr.str();
    }

    static GLuint Compile_Shader(GLuint type, const std::string& source) {
        GLuint shaderID = glCreateShader(type);
        const char* sourcePtr = source.c_str();
        glShaderSource(shaderID, 1, &sourcePtr, NULL);
        glCompileShader(shaderID);

        GLint result = GL_FALSE;
        int infoLogLength;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0 && result == GL_FALSE) {
            std::vector<char> errorMessage(infoLogLength + 1);
            glGetShaderInfoLog(shaderID, infoLogLength, NULL, &errorMessage[0]);
            glDeleteShader(shaderID);
            throw std::runtime_error(std::string(errorMessage.begin(), errorMessage.end()));
        }
        return shaderID;
    }

    PEGLShaderProgram PEGLCompile_and_Link_Shader(const std::string& vertexPath,
        const std::string& fragmentPath)
    {
        PEGLShaderProgram program;
        std::string fullVertPath = SHADERS_BASE_PATH + vertexPath;
        std::string fullFragPath = SHADERS_BASE_PATH + fragmentPath;

        try {
            std::string vertexCode = Read_File(fullVertPath);
            std::string fragmentCode = Read_File(fullFragPath);

            GLuint vertexID = Compile_Shader(GL_VERTEX_SHADER, vertexCode);
            GLuint fragmentID = Compile_Shader(GL_FRAGMENT_SHADER, fragmentCode);

            program.programID = glCreateProgram();
            glAttachShader(program.programID, vertexID);
            glAttachShader(program.programID, fragmentID);
            glLinkProgram(program.programID);

            GLint result = GL_FALSE;
            int infoLogLength;
            glGetProgramiv(program.programID, GL_LINK_STATUS, &result);
            glGetProgramiv(program.programID, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (infoLogLength > 0 && result == GL_FALSE) {
                std::vector<char> errorMessage(infoLogLength + 1);
                glGetProgramInfoLog(program.programID, infoLogLength, NULL, &errorMessage[0]);
                glDeleteProgram(program.programID);
                program.programID = 0;
                PElogger((std::string("Shader Linking Failed: ") + std::string(errorMessage.begin(), errorMessage.end())).c_str());
                throw std::runtime_error("Shader Linking Failed: " + std::string(errorMessage.begin(), errorMessage.end()));
            }

            glDetachShader(program.programID, vertexID);
            glDetachShader(program.programID, fragmentID);
            glDeleteShader(vertexID);
            glDeleteShader(fragmentID);
        }
        catch (const std::runtime_error& e) {
            std::cerr << "Shader Error: " << e.what() << std::endl;
            PElogger((std::string("Shader Error: ") + e.what()).c_str());
            program.programID = 0;
        }
        return program;
    }
    static void glfw_error_callback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }
    PEGLShaderProgram* PEGLAudo_Compile_and_Link_Shader(Level_graphics lg) {
        std::string graphicsbasePath;
        switch (lg)
        {
        case gnu::PEGL_GRAPHICS_LOW:
			graphicsbasePath = "Low";
            break;
        case gnu::PEGL_GRAPHICS_MEDIUM:
			graphicsbasePath = "Medium";
            break;
        case gnu::PEGL_GRAPHICS_HIGH:
			graphicsbasePath = "High";
            break;
        default:
			graphicsbasePath = "Medium";
            break;
        }

        PEGLShaderProgram modelShader = PEGLCompile_and_Link_Shader("default\\"+graphicsbasePath+"\\simple.vert", "default\\"+graphicsbasePath+"\\simple.frag");
        PEGLShaderProgram uiShader = PEGLCompile_and_Link_Shader("default\\ui.vert", "default\\ui.frag");
        PEGLShaderProgram textShader = PEGLCompile_and_Link_Shader("default\\text.vert", "default\\text.frag");
	    return new PEGLShaderProgram[3]{ modelShader, uiShader, textShader };
    }

    void PEGLShow_Loading_Screen(GLFWwindow* window,
        const PEGLShaderProgram& uiShader,
        const glm::mat4& orthoMatrix)
    {
        if (!window || uiShader.programID == 0) {
            PElogger("ERROR: Failed to initialize loading screen (Window or Shader is NULL).");
            return;
        }
        GLuint splashTextureID = PEGLLoad_Texture_From_File(BASE_PATH_ICON_PE);

        if (splashTextureID == 0) {
            PElogger((std::string("WARNING: Failed to load splash screen texture: ") + BASE_PATH_ICON_PE).c_str());
            return;
        }
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        UI::PEGLImage splashImage;
        static PEGLMesh staticQuadPEGLMesh = UI::PEGLCreate_Quad_Mesh();

        splashImage.textureID = splashTextureID;
        splashImage.position = glm::vec2(0.0f, 0.0f);
        splashImage.size = glm::vec2((float)windowWidth, (float)windowHeight);
        splashImage.layer = 1.0f;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        UI::PEGLDraw_Image(splashImage, uiShader, orthoMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    GLFWwindow* PEGLInit_OpenGL_Window(int width, int height, const std::string& title) {
        glfwSetErrorCallback(glfw_error_callback);

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            PElogger("Failed to initialize GLFW");
            return nullptr;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!window) {
            std::cerr << "Failed to open GLFW window." << std::endl;
            PElogger("Failed to open GLFW window.");
            glfwTerminate();
            return nullptr;
        }
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            PElogger("Failed to initialize GLAD");
            glfwDestroyWindow(window);
            glfwTerminate();
            return nullptr;
        }
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        return window;
    }
    GLuint PEGLLoad_Texture_From_File(const std::string& filePath) {
        GLuint textureID = 0;
        int width, height, comp;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &comp, 0);

        if (data) {
            GLenum format = GL_RGB;
            if (comp == 4) format = GL_RGBA;
            if (comp == 1) format = GL_RED;

            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }
        else {
            std::cerr << "ERROR: STB_IMAGE failed to load texture: " << filePath << std::endl;
		    PElogger(("ERROR: STB_IMAGE failed to load texture: " + filePath).c_str());
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        return textureID;
    }
    void PEGLPrepare_Mesh_For_GPU(PEGLMesh& mesh, const std::vector<PEGLVertex>& vertices, const std::vector<uint32_t>& indices) {
        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);

        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PEGLVertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PEGLVertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PEGLVertex), (void*)offsetof(PEGLVertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PEGLVertex), (void*)offsetof(PEGLVertex, texCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PEGLVertex), (void*)offsetof(PEGLVertex, tangent));

        mesh.indexCount = (uint32_t)indices.size();
        glBindVertexArray(0);
    }

    PEGLModel PEGLLoad_Model_From_File_OBJ(const std::string& filePath, const std::string& baseDir) {
        PEGLModel model;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err, warn;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str(), baseDir.c_str());

        if (!warn.empty()) PElogger(("tinyobjloader WARNING: " + warn).c_str());
        if (!err.empty()) throw std::runtime_error("tinyobjloader ERROR: " + err);
        if (!ret) throw std::runtime_error("Failed to load/parse OBJ file: " + filePath);

        for (const auto& shape : shapes) {
            std::map<tinyobj::index_t, uint32_t, PEGLtinyobj_index_cmp> unique_indices;
            std::vector<PEGLVertex> vertices;
            std::vector<uint32_t> indices;

            for (const auto& index : shape.mesh.indices) {
                if (unique_indices.count(index) == 0) {
                    PEGLVertex vertex{};
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };
                    if (index.normal_index >= 0) {
                        vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        };
                    }
                    if (index.texcoord_index >= 0) {
                        vertex.texCoords = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                        };
                    }

                    unique_indices[index] = (uint32_t)vertices.size();
                    indices.push_back((uint32_t)vertices.size());
                    vertices.push_back(vertex);
                }
                else indices.push_back(unique_indices[index]);
            }

            for (size_t i = 0; i < indices.size(); i += 3) {
                PEGLVertex& v0 = vertices[indices[i]];
                PEGLVertex& v1 = vertices[indices[i + 1]];
                PEGLVertex& v2 = vertices[indices[i + 2]];

                glm::vec3 edge1 = v1.position - v0.position;
                glm::vec3 edge2 = v2.position - v0.position;
                glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
                glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                glm::vec3 tangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                v0.tangent += tangent;
                v1.tangent += tangent;
                v2.tangent += tangent;
            }
            for (auto& v : vertices) v.tangent = glm::normalize(v.tangent);

            PEGLMesh mesh;
            PEGLPrepare_Mesh_For_GPU(mesh, vertices, indices);
            model.meshes.push_back(std::move(mesh));
        }
        return model;
    }

    PEGLModel PEGLCreate_Cube_Model() {
        PEGLModel model;
        std::vector<PEGLVertex> vertices = {
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}
        };
        std::vector<uint32_t> indices = {
            0, 1, 2, 0, 2, 3,       
            4, 5, 6, 4, 6, 7,       
            8, 9, 10, 8, 10, 11,    
            12, 13, 14, 12, 14, 15, 
            16, 17, 18, 16, 18, 19, 
            20, 21, 22, 20, 22, 23  
        };

        PEGLMesh cubePEGLMesh;
        PEGLPrepare_Mesh_For_GPU(cubePEGLMesh, vertices, indices);
        model.meshes.push_back(std::move(cubePEGLMesh));
        return model;
    }
    bool PEGLtinyobj_index_cmp::operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const {
        if (a.vertex_index != b.vertex_index) return a.vertex_index < b.vertex_index;
        if (a.normal_index != b.normal_index) return a.normal_index < b.normal_index;
        return a.texcoord_index < b.texcoord_index;
    }
    void PEGLDraw_Mesh(const PEGLMesh& mesh, const PEGLShaderProgram& shader,
        const glm::mat4& viewProjection, const std::vector<PEGLPointLight>& allLights,
        const glm::vec3& viewPos, const glm::mat4& modelMatrix) {

        if (mesh.VAO == 0) return;
        glUseProgram(shader.programID);

        glUniformMatrix4fv(glGetUniformLocation(shader.programID, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader.programID, "viewProjection"), 1, GL_FALSE, &viewProjection[0][0]);
        glUniform3f(glGetUniformLocation(shader.programID, "viewPos"), viewPos.x, viewPos.y, viewPos.z);

        int numLights = static_cast<int>(allLights.size());
        glUniform1i(glGetUniformLocation(shader.programID, "activeLightCount"), allLights.size());
        for (int i = 0; i < allLights.size(); i++) {
            std::string base = "lights[" + std::to_string(i) + "]";
            glUniform3f(glGetUniformLocation(shader.programID, (base + ".position").c_str()), allLights[i].pos.x, allLights[i].pos.y, allLights[i].pos.z);
            glUniform3f(glGetUniformLocation(shader.programID, (base + ".color").c_str()), allLights[i].color.x, allLights[i].color.y, allLights[i].color.z);
            glUniform1f(glGetUniformLocation(shader.programID, (base + ".intensity").c_str()), allLights[i].intensity);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.material.diffuseMap);
        glUniform1i(glGetUniformLocation(shader.programID, "diffuseMap"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mesh.material.normalMap);
        glUniform1i(glGetUniformLocation(shader.programID, "normalMap"), 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mesh.material.specularMap);
        glUniform1i(glGetUniformLocation(shader.programID, "specularMap"), 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mesh.material.shadowMap);
        glUniform1i(glGetUniformLocation(shader.programID, "shadowMap"), 3);
        glUniform1f(glGetUniformLocation(shader.programID, "opacity"), mesh.material.opacity);
        glUniform1f(glGetUniformLocation(shader.programID, "shininess"), mesh.material.shininess);

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void PEGLDraw_Model(const PEGLModel& model, const PEGLShaderProgram& shader,
        const glm::mat4& viewProjection, const std::vector<PEGLPointLight>& allLights,
        const glm::vec3& viewPos, const glm::mat4& modelMatrix) {

        for (const auto& mesh : model.meshes) {
            PEGLDraw_Mesh(mesh, shader, viewProjection, allLights, viewPos, modelMatrix);
        }
    }
    namespace UI {
        static GLuint textVAO = 0;
        static GLuint textVBO = 0;
        static char quad_buffer[100000];
        static char tri_buffer[200000];

        void PEGLSet_UI_Shader_Uniforms(const PEGLUIQuad& quad, const PEGLShaderProgram& shader, float roundness) {
            GLint sizeLoc = glGetUniformLocation(shader.programID, "size");
            GLint roundLoc = glGetUniformLocation(shader.programID, "roundness");
            if (sizeLoc != -1) glUniform2f(sizeLoc, quad.size.x, quad.size.y);
            if (roundLoc != -1) glUniform1f(roundLoc, roundness);
        }

        glm::mat4 gnu::UI::PEGLUIQuad::get_transform() const {
            float z_position = -this->layer * 0.01f;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(position.x, position.y, z_position));
            model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
            model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

            return model;
        }

        float PEGLprint_string_get_width(const char* text, float scale_x) {
            if (text == nullptr) return 0;
            return stb_easy_font_width((char*)text) * scale_x;
        }

        void PEGLprint_string(float x, float y, const char* text, float r, float g, float b, float scale_x, bool flip_y) {
            if (textVAO == 0) {
                glGenVertexArrays(1, &textVAO);
                glGenBuffers(1, &textVBO);
                glBindVertexArray(textVAO);
                glBindBuffer(GL_ARRAY_BUFFER, textVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(tri_buffer), NULL, GL_DYNAMIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 16, (void*)0);
                glBindVertexArray(0);
            }

            int num_quads = stb_easy_font_print(0, 0, (char*)text, NULL, quad_buffer, sizeof(quad_buffer));
            if (num_quads <= 0) return;

            float* src = (float*)quad_buffer;
            float* dst = (float*)tri_buffer;
            int total_vertices = 0;

            for (int i = 0; i < num_quads; ++i) {
                float V1[4], V2[4], V3[4], V4[4];
                memcpy(V1, src, 16); src += 4;
                memcpy(V2, src, 16); src += 4;
                memcpy(V3, src, 16); src += 4;
                memcpy(V4, src, 16); src += 4;

                float* v_ptr[] = { V1, V2, V3, V4 };
                for (int j = 0; j < 4; ++j) {
                    v_ptr[j][0] = x + v_ptr[j][0] * scale_x;
                    v_ptr[j][1] = flip_y ? (y - v_ptr[j][1] * scale_x) : (y + v_ptr[j][1] * scale_x);
                }

                memcpy(dst, V1, 16); dst += 4;
                memcpy(dst, V2, 16); dst += 4;
                memcpy(dst, V3, 16); dst += 4;
                memcpy(dst, V1, 16); dst += 4;
                memcpy(dst, V3, 16); dst += 4;
                memcpy(dst, V4, 16); dst += 4;
                total_vertices += 6;
            }

            glDisable(GL_DEPTH_TEST);
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, total_vertices * 16, tri_buffer);
            glBindVertexArray(textVAO);
            glDrawArrays(GL_TRIANGLES, 0, total_vertices);
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
        }

        PEGLMesh PEGLCreate_Quad_Mesh() {
            std::vector<PEGLVertex> vertices = {
                {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
            };
            std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

            PEGLMesh mesh;
            glGenVertexArrays(1, &mesh.VAO);
            glGenBuffers(1, &mesh.VBO);
            glGenBuffers(1, &mesh.EBO);

            glBindVertexArray(mesh.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PEGLVertex), vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PEGLVertex), (void*)offsetof(PEGLVertex, position));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PEGLVertex), (void*)offsetof(PEGLVertex, texCoords));
            glEnableVertexAttribArray(2);

            mesh.indexCount = (uint32_t)indices.size();
            glBindVertexArray(0);
            return mesh;
        }
        void gnu::UI::PEGLDraw_Quad_2D(const PEGLUIQuad& quad, const PEGLShaderProgram& shader, const glm::mat4& orthoMatrix, float roundness) {
            if (quad.mesh.VAO == 0 || shader.programID == 0) return;

            glUseProgram(shader.programID);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            glm::mat4 model = quad.get_transform();
            glm::mat4 MVP = orthoMatrix * model;

            glUniformMatrix4fv(glGetUniformLocation(shader.programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniform2f(glGetUniformLocation(shader.programID, "size"), quad.size.x, quad.size.y);
            glUniform1f(glGetUniformLocation(shader.programID, "roundness"), roundness);
            glUniform1f(glGetUniformLocation(shader.programID, "opacity"), quad.mesh.material.opacity);
            glUniform3f(glGetUniformLocation(shader.programID, "baseColor"),
                quad.mesh.material.baseColor.r,
                quad.mesh.material.baseColor.g,
                quad.mesh.material.baseColor.b);

            GLuint texID = 0;
            const PEGLImage* img = dynamic_cast<const PEGLImage*>(&quad);
            if (img && img->textureID != 0) {
                texID = img->textureID;
            }
            else {
                texID = quad.mesh.material.diffuseMap;
            }

            if (texID != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texID);
                glUniform1i(glGetUniformLocation(shader.programID, "useTexture"), 1);
                glUniform1i(glGetUniformLocation(shader.programID, "u_texture"), 0);
            }
            else {
                glUniform1i(glGetUniformLocation(shader.programID, "useTexture"), 0);
            }

            glBindVertexArray(quad.mesh.VAO);
            glDrawElements(GL_TRIANGLES, quad.mesh.indexCount, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glEnable(GL_DEPTH_TEST);
        }
        void PEGLDraw_Button(const PEGLButton& button, const PEGLShaderProgram& uiShader, const PEGLShaderProgram& textShader, const glm::mat4& orthoMatrix) {
            PEGLButton btnCopy = button;
            if (button.isHovered) btnCopy.mesh.material.baseColor *= 0.7f;

            PEGLDraw_Quad_2D(btnCopy, uiShader, orthoMatrix, 12.0f);

            glUseProgram(textShader.programID);
            glUniformMatrix4fv(glGetUniformLocation(textShader.programID, "orthoMatrix"), 1, GL_FALSE, &orthoMatrix[0][0]);

            float tw = PEGLprint_string_get_width(button.text.c_str(), 1.0f);
            float tx = button.position.x + (button.size.x - tw) / 2.0f;
            float ty = button.position.y + (button.size.y / 2.0f) - 4.0f;

            PEGLprint_string(tx, ty, button.text.c_str(), button.textColor.x, button.textColor.y, button.textColor.z, 1.0f, false);
        }

        void PEGLDraw_Checkbox(const PEGLCheckbox& checkbox, const PEGLShaderProgram& uiShader, const PEGLShaderProgram& textShader, const glm::mat4& orthoMatrix) {
            PEGLUIQuad box = checkbox;
            box.size = checkbox.boxSize;
            box.mesh.material.baseColor = glm::vec3(0.15f, 0.15f, 0.15f);
            PEGLDraw_Quad_2D(box, uiShader, orthoMatrix, 4.0f);

            if (checkbox.isChecked) {
                PEGLUIQuad mark = box;
                mark.size -= 8.0f;
                mark.position += 4.0f;
                mark.mesh.material.baseColor = glm::vec3(0.2f, 0.8f, 0.2f);
                PEGLDraw_Quad_2D(mark, uiShader, orthoMatrix, 2.0f);
            }

            glUseProgram(textShader.programID);
            glUniformMatrix4fv(glGetUniformLocation(textShader.programID, "orthoMatrix"), 1, GL_FALSE, &orthoMatrix[0][0]);
            PEGLprint_string(checkbox.position.x + checkbox.boxSize.x + 10.0f, checkbox.position.y + (checkbox.boxSize.y / 2) - 8.0f,
                checkbox.text.c_str(), checkbox.textColor[0], checkbox.textColor[1], checkbox.textColor[2], 1.0f, false);
        }

        void PEGLDraw_InputField(const PEGLInputField& input, const PEGLShaderProgram& uiShader, const PEGLShaderProgram& textShader, const glm::mat4& orthoMatrix) {
            PEGLUIQuad quad = input;
            quad.mesh.material.baseColor = input.isActive ? glm::vec3(0.2f, 0.2f, 0.25f) : glm::vec3(0.1f, 0.1f, 0.1f);

            PEGLDraw_Quad_2D(quad, uiShader, orthoMatrix, 8.0f);

            glUseProgram(textShader.programID);
            glUniformMatrix4fv(glGetUniformLocation(textShader.programID, "orthoMatrix"), 1, GL_FALSE, &orthoMatrix[0][0]);

            std::string to_render = input.text.empty() ? input.hintText : input.text;
            glm::vec3 col = input.text.empty() ? glm::vec3(0.5f) : input.textColor;
            if (input.isActive) to_render += "|";

            PEGLprint_string(input.position.x + 10.0f, input.position.y + (input.size.y / 2) - 8.0f,
                to_render.c_str(), col.x, col.y, col.z, 1.0f, false);
        }

        void PEGLDraw_Panel(const PEGLPanel& panel, const PEGLShaderProgram& uiShader, const glm::mat4& orthoMatrix) {
            PEGLDraw_Quad_2D(panel, uiShader, orthoMatrix, 15.0f);
        }

        void PEGLDraw_Image(const PEGLImage& image, const PEGLShaderProgram& uiShader, const glm::mat4& orthoMatrix) {
            PEGLDraw_Quad_2D(image, uiShader, orthoMatrix, 0.0f);
        }

        bool PEGLis_point_in_quad(float x, float y, const PEGLUIQuad& quad) {
            return x >= quad.position.x && x <= (quad.position.x + quad.size.x) &&
                y >= quad.position.y && y <= (quad.position.y + quad.size.y);
        }
    }
}