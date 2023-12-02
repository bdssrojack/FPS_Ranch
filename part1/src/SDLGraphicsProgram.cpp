#include "SDLGraphicsProgram.hpp"
#include "Camera.hpp"
#include "Terrain.hpp"
#include "Sphere.hpp"
#include "Texture.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <bits/stdc++.h>
#include <vector>

int SCREEN_WIDTH = 0, SCREEN_HEIGHT = 0;
int CROSSHAIR_WIDTH = 3, CROSSHAIR_HEIGHT = 3;
int RANCH_SCALE = 50;

void fire() {
    
}

void genTargets(SceneNode *root) {
    // Create targets
    std::vector<SceneNode*> children = root->GetChildren();
    for(int i = 0; i < children.size(); i++)
        delete children[i];
    
    for (int i = 0; i < 30; i++) {
        Object *sphere = new Sphere();
        sphere->LoadTexture("./assets/textures/sun.ppm");
        SceneNode *target = new SceneNode(sphere);
        target->GetLocalTransform().LoadIdentity();
        target->GetLocalTransform().Translate(rand()%RANCH_SCALE, rand()%10, rand()%RANCH_SCALE);
        target->GetLocalTransform().Scale(0.5f, 0.5f,0.5f);
        root->AddChild(target);
    }
}

void refreshTargets(SceneNode* root){
    std::vector<SceneNode*> children = root->GetChildren();
    for(int i = 0; i < children.size(); i++){
        SceneNode *target = children[i];
        target->GetLocalTransform().LoadIdentity();
        target->GetLocalTransform().Translate(rand()%RANCH_SCALE, rand()%10, rand()%RANCH_SCALE);
        target->GetLocalTransform().Scale(0.5f, 0.5f,0.5f);
    }
}

// Initialization function
// Returns a true or false value based on successful completion of setup.
// Takes in dimensions of window.
SDLGraphicsProgram::SDLGraphicsProgram() {
    // Initialization flag
    bool success = true;
    // String to hold any errors that occur.
    std::stringstream errorStream;
    // The window we'll be rendering to
    m_window = NULL;
    

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        errorStream << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        success = false;
    } else {
        //Use OpenGL 3.3 core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        // We want to request a double buffer for smooth updating.
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        // Get the screen width and height for full screen
        SDL_DisplayMode currentDisplayMode;
        if (SDL_GetDesktopDisplayMode(0, &currentDisplayMode) == 0) {
            SCREEN_WIDTH = currentDisplayMode.w;
            SCREEN_HEIGHT = currentDisplayMode.h;
        } else {
            // Fall back to a default resolution if getting display mode fails
            SCREEN_WIDTH = 1920; // Change to your desired width
            SCREEN_HEIGHT = 1080; // Change to your desired height
        }

        //Create window
        m_window = SDL_CreateWindow("Training Range",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SCREEN_WIDTH,
                                    SCREEN_HEIGHT,
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_INPUT_GRABBED);

        // Check if Window did not create.
        if (m_window == NULL) {
            errorStream << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
            success = false;
        }

        //Create an OpenGL Graphics Context
        m_openGLContext = SDL_GL_CreateContext(m_window);
        if (m_openGLContext == NULL) {
            errorStream << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
            success = false;
        }

        // Initialize GLAD Library
        if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
            errorStream << "Failed to iniitalize GLAD\n";
            success = false;
        }

        //Initialize OpenGL
        if (!InitGL()) {
            errorStream << "Unable to initialize OpenGL!\n";
            success = false;
        }
    }

    // If initialization did not work, then print out a list of errors in the constructor.
    if (!success) {
        errorStream << "SDLGraphicsProgram::SDLGraphicsProgram - Failed to initialize!\n";
        std::string errors = errorStream.str();
        SDL_Log("%s\n", errors.c_str());
    } else {
        SDL_Log("SDLGraphicsProgram::SDLGraphicsProgram - No SDL, GLAD, or OpenGL, errors detected during initialization\n\n");
    }

    // SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN); // Uncomment to enable extra debug support!
    GetOpenGLVersionInfo();

    // Set relative mode
    SDL_WarpMouseInWindow(m_window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Setup our Renderer
    m_renderer = new Renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
}


// Proper shutdown of SDL and destroy initialized objects
SDLGraphicsProgram::~SDLGraphicsProgram() {
    if (m_renderer != nullptr) {
        delete m_renderer;
    }
    //Destroy context
    SDL_GL_DeleteContext(m_openGLContext);
    //Destroy window
    SDL_DestroyWindow(m_window);
    // Point m_window to NULL to ensure it points to nothing.
    m_window = nullptr;
    //Quit SDL subsystems
    SDL_Quit();
}


// Initialize OpenGL
// Setup any of our shaders here.
bool SDLGraphicsProgram::InitGL() {
    //Success flag
    bool success = true;

    return success;
}

//Loops forever!
void SDLGraphicsProgram::Loop() {

    srand(time(0));

    static float rotate = 0.0f;

    // Create crosshair
    Object *crosshair = new Sphere();
    SceneNode *crosshairNode = new SceneNode(crosshair);

    // Create floor
    Terrain *Floor = new Terrain(RANCH_SCALE, RANCH_SCALE, "./assets/textures/grass.ppm");
    SceneNode *FloorNode = new SceneNode(Floor);

    genTargets(FloorNode);

    // Render our scene starting from the sun.
    m_renderer->setRoot(FloorNode);

    // Set a default position for our camera
    m_renderer->GetCamera(0)->SetCameraEyePosition(RANCH_SCALE/2.0f, 0.5f, RANCH_SCALE/2.0f);
    m_renderer->GetCamera(0)->SetBoundary(RANCH_SCALE-1, RANCH_SCALE-1);

    // Main loop flag
    // If this is quit = 'true' then the program terminates.
    bool quit = false;
    // Event handler that handles various events in SDL
    // that are related to input and output
    SDL_Event e;
    // Enable text input
    SDL_StartTextInput();

    // Set the camera speed for how fast we move.
    float cameraSpeed = 0.25f;
    bool sprint = false;
    int mouseX = 0, mouseY = 0;

    // While application is running
    while (!quit) {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User posts an event to quit
            // An example is hitting the "x" in the corner of the window.
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
            // Handle keyboard input for the camera class
            if (e.type == SDL_MOUSEMOTION) {
                // Handle mouse movements
                mouseX += e.motion.xrel;
                mouseY += e.motion.yrel;
                m_renderer->GetCamera(0)->MouseLook(mouseX, mouseY);
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                // left click -- fire
                fire();
            }
        } // End SDL_PollEvent loop.

        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
        if (keyboardState[SDL_SCANCODE_W]) {
            m_renderer->GetCamera(0)->MoveForward(cameraSpeed);
        } else if (keyboardState[SDL_SCANCODE_S]) {
            m_renderer->GetCamera(0)->MoveBackward(cameraSpeed);
        }

        if (keyboardState[SDL_SCANCODE_A]) {
            m_renderer->GetCamera(0)->MoveLeft(cameraSpeed);
        } else if (keyboardState[SDL_SCANCODE_D]) {
            m_renderer->GetCamera(0)->MoveRight(cameraSpeed);
        }

        if (keyboardState[SDL_SCANCODE_SPACE]) {
            //TODO: jump
        }
        if (keyboardState[SDL_SCANCODE_LSHIFT]) {
            //TODO: sprint
        }
        if (keyboardState[SDL_SCANCODE_LCTRL]) {
            //TODO: crouch
        }
        if(keyboardState[SDL_SCANCODE_R]) {
            SDL_Delay(100);
            refreshTargets(FloorNode);
        }

        FloorNode->GetLocalTransform().LoadIdentity();

        // Update our scene through our renderer
        m_renderer->Update();
        // Render our scene using our selected renderer
        m_renderer->Render();
        // Delay to slow things down just a bit!
        // SDL_Delay(25);  // TODO: You can change this or implement a frame
        // independent movement method if you like.
        DrawCrosshair(crosshairNode);
        //Update screen of our specified window
        SDL_GL_SwapWindow(GetSDLWindow());
        rotate += 0.02f;
    }
    //Disable text input
    SDL_StopTextInput();
    //Free memory
    delete FloorNode;
    delete this;
}


// Get Pointer to Window
SDL_Window *SDLGraphicsProgram::GetSDLWindow() {
    return m_window;
}

// Helper Function to get OpenGL Version Information
void SDLGraphicsProgram::GetOpenGLVersionInfo() {
    SDL_Log("(Note: If you have two GPU's, make sure the correct one is selected)");
    SDL_Log("Vendor: %s", (const char *) glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", (const char *) glGetString(GL_RENDERER));
    SDL_Log("Version: %s", (const char *) glGetString(GL_VERSION));
    SDL_Log("Shading language: %s", (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void SDLGraphicsProgram::DrawCrosshair(SceneNode *crosshairNode) {
    glDisable(GL_DEPTH_TEST);

    float scale_factor = 5.0f;

    crosshairNode->GetLocalTransform().LoadIdentity();

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),((float)SCREEN_WIDTH)/((float)SCREEN_HEIGHT),0.0f,512.0f);
    crosshairNode->Update(projectionMatrix, m_renderer->GetCamera(0));

    glm::mat4 identityMatrix = glm::mat4(1.0f);
    crosshairNode->m_shader.SetUniformMatrix4fv("view", &identityMatrix[0][0]);
    crosshairNode->m_shader.SetUniformMatrix4fv("model", &identityMatrix[0][0]);
    crosshairNode->m_shader.SetUniformMatrix4fv("projection", &glm::ortho(-(SCREEN_WIDTH / scale_factor), SCREEN_WIDTH / scale_factor, SCREEN_HEIGHT / scale_factor, -(SCREEN_HEIGHT / scale_factor), -1000.0f, 1000.0f)[0][0]);
    crosshairNode->m_shader.SetUniform3f("lightColor",0.0f,0.0f,0.0f);
    crosshairNode->Draw();
    
    glEnable(GL_DEPTH_TEST);
}