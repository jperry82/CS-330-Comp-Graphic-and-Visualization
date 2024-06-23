///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "SceneManager.h"
#include <glm/gtx/transform.hpp>
#include <vector>

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager), m_basicMeshes(new ShapeMeshes()), m_loadedTextures(0) {
    // initialize the texture collection
    for (int i = 0; i < 16; i++) {
        m_textureIDs[i].tag = "";
        m_textureIDs[i].ID = -1;
    }

    // Initialize primary light (prominent orange glow from just above the plane)
    m_primaryLight.position = glm::vec3(0.0f, 2.0f, 0.0f);
    m_primaryLight.color = glm::vec3(1.0f, 0.55f, 0.0f);  // A more prominent orange
    m_primaryLight.intensity = 1.5f;

    // Initialize ambient light (soft white light)
    m_ambientLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    m_ambientLight.intensity = 0.5f;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager() {
    // Clean up any allocated resources
    if (m_basicMeshes) {
        delete m_basicMeshes;
        m_basicMeshes = nullptr;
    }

    // Additional cleanup if necessary
}

/***********************************************************
 *  SetLighting()
 *
 *  This method is used for passing the lighting values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetLighting() {
    if (NULL != m_pShaderManager) {
        m_pShaderManager->setVec3Value("primaryLight.position", m_primaryLight.position);
        m_pShaderManager->setVec3Value("primaryLight.color", m_primaryLight.color);
        m_pShaderManager->setFloatValue("primaryLight.intensity", m_primaryLight.intensity);

        m_pShaderManager->setVec3Value("ambientLight.color", m_ambientLight.color);
        m_pShaderManager->setFloatValue("ambientLight.intensity", m_ambientLight.intensity);
    }
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene() {
    // load the textures for the 3D scene
    LoadSceneTextures();

    // only one instance of a particular mesh needs to be
    // loaded in memory no matter how many times it is drawn
    // in the rendered 3D scene
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadConeMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadSphereMesh();
    m_basicMeshes->LoadTaperedCylinderMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadTorusMesh();
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::LoadSceneTextures() {
    bool bReturn = false;

    bReturn = CreateGLTexture("C:\\CS330Content\\Projects\\Utilities\\textures\\circular-brushed-gold-texture.jpg", "dome");
    bReturn = CreateGLTexture("C:\\CS330Content\\Projects\\Utilities\\textures\\stainless.jpg", "hull");
    bReturn = CreateGLTexture("C:\\CS330Content\\Utilities\\textures\\stainless_end.jpg", "shuttlebay");
    bReturn = CreateGLTexture("C:\\CS330Content\\Utilities\\textures\\abstract.jpg", "planet");

    BindGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag) {
    if (m_loadedTextures >= 16) {
        std::cerr << "Error: Too many textures loaded." << std::endl;
        return false;
    }

    int width = 0;
    int height = 0;
    int colorChannels = 0;
    GLuint textureID = 0;

    // indicate to always flip images vertically when loaded
    stbi_set_flip_vertically_on_load(true);

    // try to parse the image data from the specified image file
    unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);

    // if the image was successfully read from the image file
    if (image) {
        std::cout << "Successfully loaded image: " << filename << ", width: " << width << ", height: " << height << ", channels: " << colorChannels << std::endl;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // if the loaded image is in RGB format
        if (colorChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        // if the loaded image is in RGBA format - it supports transparency
        else if (colorChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else {
            std::cout << "Error: Not implemented to handle image with " << colorChannels << " channels" << std::endl;
            stbi_image_free(image);
            return false;
        }

        // generate the texture mipmaps for mapping textures to lower resolutions
        glGenerateMipmap(GL_TEXTURE_2D);

        // free the image data from local memory
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        // register the loaded texture and associate it with the special tag string
        m_textureIDs[m_loadedTextures].ID = textureID;
        m_textureIDs[m_loadedTextures].tag = tag;
        m_loadedTextures++;

        std::cout << "Texture registered: " << tag << " ID: " << textureID << std::endl;
        BindGLTextures(); // Bind textures after creation

        return true;
    }

    std::cout << "Error: Could not load image: " << filename << std::endl;

    // Error loading the image
    return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures() {
    for (int i = 0; i < m_loadedTextures; i++) {
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures() {
    for (int i = 0; i < m_loadedTextures; i++) {
        glDeleteTextures(1, &m_textureIDs[i].ID);
    }
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag) {
    int textureID = -1;
    int index = 0;
    bool bFound = false;

    while ((index < m_loadedTextures) && (bFound == false)) {
        if (m_textureIDs[index].tag.compare(tag) == 0) {
            textureID = m_textureIDs[index].ID;
            bFound = true;
        }
        else
            index++;
    }

    return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag) {
    static std::unordered_set<std::string> missingTextures;

    for (int i = 0; i < m_loadedTextures; ++i) {
        if (m_textureIDs[i].tag == tag) {
            missingTextures.erase(tag);  // Remove tag from missingTextures if found
            return i;
        }
    }

    if (missingTextures.find(tag) == missingTextures.end()) {
        std::cout << "Error: Could not find texture slot for tag: " << tag << std::endl;
        missingTextures.insert(tag);  // Add tag to missingTextures to prevent repeated logging
    }

    return -1;
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material) {
    if (m_objectMaterials.size() == 0) {
        return(false);
    }

    int index = 0;
    bool bFound = false;
    while ((index < static_cast<int>(m_objectMaterials.size())) && (bFound == false)) {
        if (m_objectMaterials[index].tag.compare(tag) == 0) {
            bFound = true;
            material.ambientColor = m_objectMaterials[index].ambientColor;
            material.ambientStrength = m_objectMaterials[index].ambientStrength;
            material.diffuseColor = m_objectMaterials[index].diffuseColor;
            material.specularColor = m_objectMaterials[index].specularColor;
            material.shininess = m_objectMaterials[index].shininess;
        }
        else {
            index++;
        }
    }

    return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
    glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ) {
    glm::mat4 modelView;
    glm::mat4 scale;
    glm::mat4 rotationX;
    glm::mat4 rotationY;
    glm::mat4 rotationZ;
    glm::mat4 translation;

    scale = glm::scale(scaleXYZ);
    rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    translation = glm::translate(positionXYZ);

    modelView = translation * rotationX * rotationY * rotationZ * scale;

    if (NULL != m_pShaderManager) {
        m_pShaderManager->setMat4Value("model", modelView);
    }
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
    float redColorValue,
    float greenColorValue,
    float blueColorValue,
    float alphaValue) {
    glm::vec4 currentColor;

    currentColor.r = redColorValue;
    currentColor.g = greenColorValue;
    currentColor.b = blueColorValue;
    currentColor.a = alphaValue;

    if (NULL != m_pShaderManager) {
        m_pShaderManager->setIntValue("bUseTexture", false);
        m_pShaderManager->setVec4Value("objectColor", currentColor);
    }
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(std::string textureTag) {
    if (NULL != m_pShaderManager) {
        int textureSlot = FindTextureSlot(textureTag);
        if (textureSlot != -1) {
            m_pShaderManager->setIntValue("bUseTexture", true);
            m_pShaderManager->setSampler2DValue("objectTexture", textureSlot);
        }
    }
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v) {
    if (NULL != m_pShaderManager) {
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
    }
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag) {
    if (m_objectMaterials.size() > 0) {
        OBJECT_MATERIAL material;
        bool bReturn = false;

        bReturn = FindMaterial(materialTag, material);
        if (bReturn == true) {
            m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
            m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
            m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
            m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
            m_pShaderManager->setFloatValue("material.shininess", material.shininess);
        }
    }
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene() {
    glm::vec3 scaleXYZ;
    float XrotationDegrees = 0.0f;
    float YrotationDegrees = 0.0f;
    float ZrotationDegrees = 0.0f;
    glm::vec3 positionXYZ;

    // Set lighting
    SetLighting();

    // Render plane
    scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
    positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderTexture("planet");
    m_basicMeshes->DrawPlaneMesh();

    // Render upper saucer module
    scaleXYZ = glm::vec3(4.0f, 0.2f, 4.0f);
    positionXYZ = glm::vec3(-3.0f, 4.0f, 0.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawCylinderMesh();

    // Render lower saucer module
    scaleXYZ = glm::vec3(4.0f, 0.2f, 4.0f);
    positionXYZ = glm::vec3(-3.0f, 4.0f, 0.0f);
    SetTransformations(scaleXYZ, 180.0f, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawTaperedCylinderMesh();

    // Render bridge & phaser array
    scaleXYZ = glm::vec3(0.5f, 1.0f, 1.0f);
    positionXYZ = glm::vec3(-3.0f, 4.1f, 0.0f);
    SetTransformations(scaleXYZ, 90.0f, 90.0f, ZrotationDegrees, positionXYZ);
    SetShaderTexture("dome");
    m_basicMeshes->DrawSphereMesh();

    // Render neck
    scaleXYZ = glm::vec3(1.25f, 1.5f, 0.5f);
    positionXYZ = glm::vec3(0.0f, 3.25f, 0.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, 15.0f, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawBoxMesh();

    // Render deflector cone at front of main hull
    scaleXYZ = glm::vec3(0.75f, 0.5f, 0.75f);
    positionXYZ = glm::vec3(-0.59f, 2.0f, 0.0f);
    SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, 90.0f, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawTaperedCylinderMesh();

    // Render main hull
    scaleXYZ = glm::vec3(0.74f, 4.5f, 0.74f);
    positionXYZ = glm::vec3(3.9f, 2.0f, 0.0f);
    SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, 90.0f, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawCylinderMesh();

    // Render shuttlebay
    scaleXYZ = glm::vec3(0.74f, 0.74f, 0.74f);
    positionXYZ = glm::vec3(3.9f, 2.0f, 0.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderTexture("shuttlebay");
    m_basicMeshes->DrawHalfSphereMesh();

    // Render shuttlebay floor
    scaleXYZ = glm::vec3(0.01f, 0.74f, 0.74f);
    positionXYZ = glm::vec3(3.9f, 2.0f, 0.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, 90.0f, positionXYZ);
    SetShaderTexture("hall");
    m_basicMeshes->DrawSphereMesh();

    // Render deflector dish
    scaleXYZ = glm::vec3(0.35f, 0.35f, 0.35f);
    positionXYZ = glm::vec3(-1.1f, 2.0f, 0.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, -90.0f, positionXYZ);
    SetShaderColor(0.35f, 0.65f, 0.80f, 1.0f);
    m_basicMeshes->DrawConeMesh();

    // Render left pylon
    scaleXYZ = glm::vec3(0.75f, 2.5f, 0.10f);
    positionXYZ = glm::vec3(3.5f, 3.4f, 1.0f);
    SetTransformations(scaleXYZ, 40.0f, YrotationDegrees, -20.0f, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawBoxMesh();

    // Render left nacelle
    scaleXYZ = glm::vec3(0.25f, 4.5f, 0.25f);
    positionXYZ = glm::vec3(6.5f, 4.25f, 1.75f);
    SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, 90.0f, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawCylinderMesh();

    // Render left buzzard ram scoop
    scaleXYZ = glm::vec3(0.25f, 0.25f, 0.25f);
    positionXYZ = glm::vec3(2.0f, 4.25f, 1.75f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
    m_basicMeshes->DrawSphereMesh();

    // Render right pylon
    scaleXYZ = glm::vec3(0.75f, 2.5f, 0.10f);
    positionXYZ = glm::vec3(3.5f, 3.4f, -1.0f);
    SetTransformations(scaleXYZ, -40.0f, YrotationDegrees, -20.0f, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawBoxMesh();

    // Render right nacelle
    scaleXYZ = glm::vec3(0.25f, 4.5f, 0.25f);
    positionXYZ = glm::vec3(6.5f, 4.25f, -1.75f);
    SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, 90.0f, positionXYZ);
    SetShaderTexture("hull");
    m_basicMeshes->DrawCylinderMesh();

    // Render right buzzard ram scoop
    scaleXYZ = glm::vec3(0.25f, 0.25f, 0.25f);
    positionXYZ = glm::vec3(2.0f, 4.25f, -1.75f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
    m_basicMeshes->DrawSphereMesh();
}
