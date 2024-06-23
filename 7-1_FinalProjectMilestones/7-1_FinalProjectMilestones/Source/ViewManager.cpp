///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// declaration of the global variables and defines
namespace {
    // Variables for window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    // these variables are used for mouse movement processing
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // time between current frame and last frame
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager), m_pWindow(nullptr),
    Position(glm::vec3(0.0f, 5.0f, 12.0f)), Front(glm::vec3(0.0f, -0.5f, -2.0f)),
    Up(glm::vec3(0.0f, 1.0f, 0.0f)), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
    Target(glm::vec3(0.0f, 0.0f, 0.0f)),
    Yaw(-90.0f), Pitch(0.0f), MovementSpeed(2.5f), MouseSensitivity(0.1f), DistanceToTarget(10.0f),
    currentProjectionMode(PERSPECTIVE), deltaTime(0.0f), lastFrame(0.0f) {
    updateCameraVectors();
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager() {
    // free up allocated memory
    m_pShaderManager = nullptr;
    m_pWindow = nullptr;
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle) {
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, Mouse_Position_Callback);
    glfwSetScrollCallback(window, Mouse_Scroll_Callback);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_pWindow = window;
    glfwSetWindowUserPointer(window, this);
    return window;
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos) {
    ViewManager* viewManager = static_cast<ViewManager*>(glfwGetWindowUserPointer(window));
    if (viewManager) {
        if (gFirstMouse) {
            gLastX = xMousePos;
            gLastY = yMousePos;
            gFirstMouse = false;
        }

        float xOffset = xMousePos - gLastX;
        float yOffset = gLastY - yMousePos; // reversed since y-coordinates range from bottom to top

        gLastX = xMousePos;
        gLastY = yMousePos;

        viewManager->ProcessMouseMovement(xOffset, yOffset);
    }
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is moved.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset) {
    ViewManager* viewManager = static_cast<ViewManager*>(glfwGetWindowUserPointer(window));
    if (viewManager) {
        viewManager->ProcessMouseScroll(yOffset);
    }
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents() {
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_pWindow, true);
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        ProcessKeyboard(0, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        ProcessKeyboard(1, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        ProcessKeyboard(2, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        ProcessKeyboard(3, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
        ProcessKeyboard(4, gDeltaTime);
    if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
        ProcessKeyboard(5, gDeltaTime);

    if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
        SetProjectionMode(PERSPECTIVE);
    if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
        SetProjectionMode(ORTHOGRAPHIC);
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView() {
    glm::mat4 view = glm::lookAt(Position, Target, Up);
    glm::mat4 projection;

    float currentFrame = glfwGetTime();
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    ProcessKeyboardEvents();

    if (currentProjectionMode == PERSPECTIVE) {
        projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else if (currentProjectionMode == ORTHOGRAPHIC) {
        projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
    }

    if (m_pShaderManager) {
        m_pShaderManager->setMat4Value(g_ViewName, view);
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);
        m_pShaderManager->setVec3Value("viewPosition", Position);
    }
}

/***********************************************************
 *  ProcessKeyboard()
 *
 *  This method processes keyboard input for camera movement.
 ***********************************************************/
void ViewManager::ProcessKeyboard(int direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == 0)
        Target += Front * velocity;
    if (direction == 1)
        Target -= Front * velocity;
    if (direction == 2)
        Target -= Right * velocity;
    if (direction == 3)
        Target += Right * velocity;
    if (direction == 4)
        Target += Up * velocity;
    if (direction == 5)
        Target -= Up * velocity;

    updateCameraVectors();
}

/***********************************************************
 *  SetProjectionMode()
 *
 *  This method sets the projection mode to either perspective
 *  or orthographic.
 ***********************************************************/
void ViewManager::SetProjectionMode(ViewManager::ProjectionMode mode) {
    currentProjectionMode = mode;
}

/***********************************************************
 *  DeltaTime()
 *
 *  This method returns the time between frames.
 ***********************************************************/
float ViewManager::DeltaTime() const {
    return gDeltaTime;
}

/***********************************************************
 *  updateCameraVectors()
 *
 *  This method updates the camera vectors based on the
 *  current yaw and pitch values.
 ***********************************************************/
void ViewManager::updateCameraVectors() {
    // Calculate the new Front vector
    Position.x = Target.x + DistanceToTarget * cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position.y = Target.y + DistanceToTarget * sin(glm::radians(Pitch));
    Position.z = Target.z + DistanceToTarget * sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

/***********************************************************
 *  ProcessMouseMovement()
 *
 *  This method processes mouse movement to update the camera
 *  orientation.
 ***********************************************************/
void ViewManager::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    Yaw += xOffset;
    Pitch += yOffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

/***********************************************************
 *  ProcessMouseScroll()
 *
 *  This method processes mouse scroll to adjust the camera speed.
 ***********************************************************/
void ViewManager::ProcessMouseScroll(float yOffset) {
    MovementSpeed += yOffset * 0.1f;
    if (MovementSpeed < 1.0f)
        MovementSpeed = 1.0f;
    if (MovementSpeed > 10.0f)
        MovementSpeed = 10.0f;
}
