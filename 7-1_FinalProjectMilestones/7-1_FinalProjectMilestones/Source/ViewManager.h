///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class ViewManager {
public:
    // Enum for Projection Mode
    enum ProjectionMode { PERSPECTIVE, ORTHOGRAPHIC };

    // constructor
    ViewManager(ShaderManager* pShaderManager);
    // destructor
    ~ViewManager();

    // mouse position callback for mouse interaction with the 3D scene
    static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);
    static void Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset);

    // create the initial OpenGL display window
    GLFWwindow* CreateDisplayWindow(const char* windowTitle);

    // prepare the conversion from 3D object display to 2D scene display
    void PrepareSceneView();

    // process keyboard input for camera movement
    void ProcessKeyboard(int direction, float deltaTime);

    // set projection mode
    void SetProjectionMode(ProjectionMode mode);

    // get delta time
    float DeltaTime() const;

private:
    ProjectionMode currentProjectionMode;
    float deltaTime;
    float lastFrame;
    bool bOrthographicProjection;

    // pointer to shader manager object
    ShaderManager* m_pShaderManager;
    // active OpenGL display window
    GLFWwindow* m_pWindow;

    // Camera attributes and methods
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::vec3 Target;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float DistanceToTarget;

    void updateCameraVectors();
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yOffset);

    // process keyboard events for interaction with the 3D scene
    void ProcessKeyboardEvents();
};