///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <unordered_set>

class SceneManager {
public:
    // Constructor
    SceneManager(ShaderManager* pShaderManager);

    // Destructor
    ~SceneManager();

    // Methods to prepare, load, and render the scene
    void PrepareScene();
    void LoadSceneTextures();
    void RenderScene();

    // Struct to hold texture information
    struct TEXTURE_ID {
        unsigned int ID = 0;   // Initialize ID
        std::string tag;
    };

    // Struct to hold object material properties
    struct OBJECT_MATERIAL {
        std::string tag;  // Material tag
        glm::vec3 ambientColor = glm::vec3(0.0f);
        glm::vec3 diffuseColor = glm::vec3(0.0f);
        glm::vec3 specularColor = glm::vec3(0.0f);
        float ambientStrength = 0.0f;
        float shininess = 0.0f;
    };

    // Struct to hold light properties
    struct Light {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };

private:
    ShaderManager* m_pShaderManager;  // Shader manager pointer
    ShapeMeshes* m_basicMeshes;       // Basic shapes meshes
    int m_loadedTextures;         // Number of loaded textures
    TEXTURE_ID m_textureIDs[16];    // Array of texture information
    std::vector<OBJECT_MATERIAL> m_objectMaterials;  // Vector of object materials

    Light m_primaryLight;             // Primary light
    Light m_ambientLight;             // Ambient light

    // Helper methods for texture and shader operations
    bool CreateGLTexture(const char* filename, std::string tag);
    void BindGLTextures();
    void DestroyGLTextures();
    int FindTextureID(std::string tag);
    int FindTextureSlot(std::string tag);
    bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);
    void SetTransformations(glm::vec3 scaleXYZ, float XrotationDegrees, float YrotationDegrees, float ZrotationDegrees, glm::vec3 positionXYZ);
    void SetShaderColor(float redColorValue, float greenColorValue, float blueColorValue, float alphaValue);
    void SetShaderTexture(std::string textureTag);
    void SetTextureUVScale(float u, float v);
    void SetShaderMaterial(std::string materialTag);
    void SetLighting(); // Method to set lighting
};
