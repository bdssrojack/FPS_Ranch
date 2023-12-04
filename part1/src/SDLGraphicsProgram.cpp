#include "SDLGraphicsProgram.hpp"
#include "Camera.hpp"
#include "Terrain.hpp"
#include "Sphere.hpp"
#include "Texture.hpp"
#include "Sound.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <bits/stdc++.h>
#include <vector>

int SCREEN_WIDTH = 0, SCREEN_HEIGHT = 0;
int RANCH_SCALE = 50, CEILING_HEIGHT = 10;
int TARGET_NUM = 30;
float CAMERA_SPEED = 0.1f;
float STANDARD_EYE_HEIGHT = .5f, MAX_JUMP_HEIGHT = 1.8f, JUMP_VELOCITY = 0.08, CROUCHING_HEIGHT = -0.5f, FLOOR_Y_POSITION = -1.0f;

bool isSprinting = false, isCrouching = false, isJumping = false, isAscending = false;

int hitTarget = 0;

Camera *camera;
Sound *gunshot;

void resetSpeed() {
    CAMERA_SPEED = 0.1f;
}

void resetFigure() {
    resetSpeed();
    camera->SetCameraEyePosition(camera->GetEyeXPosition(), STANDARD_EYE_HEIGHT, camera->GetEyeZPosition());
    isSprinting = false;
    isCrouching = false;
}


void randomTranslate(SceneNode* node) {
    node->GetLocalTransform().LoadIdentity();
    float centerX = rand()%RANCH_SCALE, centerY = rand()%CEILING_HEIGHT, centerZ = rand()%RANCH_SCALE;
    node->m_centerCoord = glm::vec3(centerX, centerY, centerZ);
    node->GetLocalTransform().Translate(centerX, centerY, centerZ);
}

void genTargets(SceneNode *root) {
    // Create targets
    std::vector<SceneNode*> children = root->GetChildren();
    for(int i = 0; i < children.size(); i++)
        delete children[i];
    
    for (int i = 0; i < TARGET_NUM; i++) {
        Object *sphere = new Sphere();
        sphere->LoadTexture("./assets/textures/sun.ppm");
        SceneNode *target = new SceneNode(sphere);
        randomTranslate(target);
        root->AddChild(target);
    }
}

void refreshTargets(SceneNode* root){
    std::vector<SceneNode*> children = root->GetChildren();
    for(int i = 0; i < children.size(); i++){
        randomTranslate(children[i]);
    }
    hitTarget = 0;
}

void fire(SceneNode *root) {
    gunshot->play();
    // hitscan
    std::vector<SceneNode*> children = root->GetChildren();
    for(int i = 0; i < children.size(); i++){
        SceneNode *target = children[i];
        if(target->m_centerCoord.y < 0)
            continue;
        
        if(target->isHit(camera)){
            SDL_Log("target %u is hit", i);
            hitTarget++;
            target->GetLocalTransform().LoadIdentity();
            target->GetLocalTransform().Translate(10.0f, -10.0f, 10.0f);
            target->m_centerCoord = glm::vec3(10.0f, -10.0f, 10.0f);
            if(hitTarget == children.size()){
                refreshTargets(root);
            }
        }
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
                                    // SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_INPUT_GRABBED);
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_GRABBED);

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

    // Setup gunfire sound
    gunshot = new Sound("./assets/sounds/gun-gunshot-01.wav");
    gunshot->setupDevice();
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
    camera = m_renderer->GetCamera(0);
    camera->SetCameraEyePosition(RANCH_SCALE/2.0f, STANDARD_EYE_HEIGHT, RANCH_SCALE/2.0f);
    camera->SetBoundary(RANCH_SCALE-1, RANCH_SCALE-1);

    // Main loop flag
    // If this is quit = 'true' then the program terminates.
    bool quit = false;
    // Event handler that handles various events in SDL
    // that are related to input and output
    SDL_Event e;
    // Enable text input
    SDL_StartTextInput();

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
                camera->MouseLook(mouseX, mouseY);
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                // left click -- fire
                fire(FloorNode);
            }
        } // End SDL_PollEvent loop.

        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
        if (keyboardState[SDL_SCANCODE_W]) {
            camera->MoveForward(CAMERA_SPEED);
        } else if (keyboardState[SDL_SCANCODE_S]) {
            camera->MoveBackward(CAMERA_SPEED);
        }

        if (keyboardState[SDL_SCANCODE_A]) {
            camera->MoveLeft(CAMERA_SPEED);
        } else if (keyboardState[SDL_SCANCODE_D]) {
            camera->MoveRight(CAMERA_SPEED);
        }

        if (keyboardState[SDL_SCANCODE_SPACE]) { // Jump
            if(!isJumping){ // only jump when not jumping
                if(isCrouching){
                    resetSpeed();
                }
                isJumping = true;
                isAscending = true;
            }
        }
        if (keyboardState[SDL_SCANCODE_LSHIFT]) { // Sprint
            SDL_Delay(100);
            if(isSprinting) {
                resetFigure();
            } else {
                isSprinting = true;
                camera->SetCameraEyePosition(camera->GetEyeXPosition(), STANDARD_EYE_HEIGHT, camera->GetEyeZPosition());
                CAMERA_SPEED = 0.3f;
            }
        }
        if (keyboardState[SDL_SCANCODE_LCTRL]) { // Crouch
            SDL_Delay(100);
            if(isCrouching) {
                resetFigure();
            } else {
                isCrouching = true;
                CAMERA_SPEED = 0.02f;
                camera->SetCameraEyePosition(camera->GetEyeXPosition(), CROUCHING_HEIGHT, camera->GetEyeZPosition());
            }
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
        SDL_Delay(16);  // You can change this or implement a frame independent movement method if you like.
        if(isJumping) {
            float currentYPosition = camera->GetEyeYPosition();
            
            if(isAscending && currentYPosition < MAX_JUMP_HEIGHT){
                camera->SetCameraEyePosition(camera->GetEyeXPosition(), currentYPosition + JUMP_VELOCITY, camera->GetEyeZPosition());
            }
            if(abs(currentYPosition - MAX_JUMP_HEIGHT) < JUMP_VELOCITY){
                isAscending = false;
            }
            
            if(!isAscending && currentYPosition > STANDARD_EYE_HEIGHT){
                camera->SetCameraEyePosition(camera->GetEyeXPosition(), currentYPosition - JUMP_VELOCITY, camera->GetEyeZPosition());
            }
            if(abs(currentYPosition - STANDARD_EYE_HEIGHT) < JUMP_VELOCITY){
                isJumping = false;
            }
            
        }
        DrawCrosshair(crosshairNode);
        //Update screen of our specified window
        SDL_GL_SwapWindow(GetSDLWindow());
        rotate += 0.02f;
    }
    //Disable text input
    SDL_StopTextInput();
    //Free memory
    delete FloorNode;
    delete gunshot;
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

// TODO: make this work for all HUD elements
void SDLGraphicsProgram::DrawCrosshair(SceneNode *crosshairNode) {
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);

    float scale_factor = 5.0f;

    crosshairNode->GetLocalTransform().LoadIdentity();

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),((float)SCREEN_WIDTH)/((float)SCREEN_HEIGHT),0.0f,512.0f);
    crosshairNode->Update(projectionMatrix, m_renderer->GetCamera(0));

    glm::mat4 identityMatrix = glm::mat4(1.0f);
    crosshairNode->m_shader.SetUniformMatrix4fv("view", &identityMatrix[0][0]);
    crosshairNode->m_shader.SetUniformMatrix4fv("projection", &glm::ortho(-(SCREEN_WIDTH / scale_factor), SCREEN_WIDTH / scale_factor, SCREEN_HEIGHT / scale_factor, -(SCREEN_HEIGHT / scale_factor), -1000.0f, 1000.0f)[0][0]);
    // crosshairNode->m_shader.SetUniform3f("lightColor",1.0f,0.0f,0.0f);
    crosshairNode->Draw();
    
    glEnable(GL_DEPTH_TEST);
}