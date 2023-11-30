#include "SDLGraphicsProgram.hpp"
#include "Camera.hpp"
#include "Terrain.hpp"
#include "Sphere.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

// Initialization function
// Returns a true or false value based on successful completion of setup.
// Takes in dimensions of window.
SDLGraphicsProgram::SDLGraphicsProgram(int w, int h){
	// Initialization flag
	bool success = true;
	// String to hold any errors that occur.
	std::stringstream errorStream;
	// The window we'll be rendering to
	m_window = NULL;

	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO)< 0){
		errorStream << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
		success = false;
	}
	else{
		//Use OpenGL 3.3 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		// We want to request a double buffer for smooth updating.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		//Create window
		m_window = SDL_CreateWindow( "Lab",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                w,
                                h,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

		// Check if Window did not create.
		if( m_window == NULL ){
			errorStream << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
			success = false;
		}

		//Create an OpenGL Graphics Context
		m_openGLContext = SDL_GL_CreateContext( m_window );
		if( m_openGLContext == NULL){
			errorStream << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
			success = false;
		}

		// Initialize GLAD Library
		if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
			errorStream << "Failed to iniitalize GLAD\n";
			success = false;
		}

		//Initialize OpenGL
		if(!InitGL()){
			errorStream << "Unable to initialize OpenGL!\n";
			success = false;
		}
  	}

    // If initialization did not work, then print out a list of errors in the constructor.
    if(!success){
        errorStream << "SDLGraphicsProgram::SDLGraphicsProgram - Failed to initialize!\n";
        std::string errors=errorStream.str();
        SDL_Log("%s\n",errors.c_str());
    }else{
        SDL_Log("SDLGraphicsProgram::SDLGraphicsProgram - No SDL, GLAD, or OpenGL, errors detected during initialization\n\n");
    }

	// SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN); // Uncomment to enable extra debug support!
	GetOpenGLVersionInfo();


    // Setup our Renderer
    m_renderer = new Renderer(w,h);    
}


// Proper shutdown of SDL and destroy initialized objects
SDLGraphicsProgram::~SDLGraphicsProgram(){
    if(m_renderer!=nullptr){
        delete m_renderer;
    }


    //Destroy window
	SDL_DestroyWindow( m_window );
	// Point m_window to NULL to ensure it points to nothing.
	m_window = nullptr;
	//Quit SDL subsystems
	SDL_Quit();
}


// Initialize OpenGL
// Setup any of our shaders here.
bool SDLGraphicsProgram::InitGL(){
	//Success flag
	bool success = true;

	return success;
}

// Create the Sun
Object* sphere;
SceneNode* Sun;
// ====================== Create the planets =============

//Loops forever!
void SDLGraphicsProgram::Loop(){

    // ================== Initialize the planets ===============
    static float rotate = 0.0f;

    // Create the Sun
    sphere = new Sphere();
    sphere->LoadTexture("./assets/textures/sun.ppm");
    Sun = new SceneNode(sphere);

    // Create Earth and Moon
    for(int i = 0; i < 3; i++) {
        Object* sphereEarth = new Sphere();
        sphereEarth->LoadTexture("./assets/textures/earth.ppm");
        SceneNode* Earth = new SceneNode(sphereEarth);
        Sun->AddChild(Earth);

        for(int j = 0; j < 2; j++){
            Object* sphereMoon = new Sphere();
            sphereMoon->LoadTexture("./assets/textures/rock.ppm");
            // Create a new node using sphere3 as the geometry
            SceneNode* Moon = new SceneNode(sphereMoon);
            Earth->AddChild(Moon);
        }
    }

    // Render our scene starting from the sun.
    m_renderer->setRoot(Sun);

    // Set a default position for our camera
    m_renderer->GetCamera(0)->SetCameraEyePosition(0.0f,0.0f,20.0f);

    // Main loop flag
    // If this is quit = 'true' then the program terminates.
    bool quit = false;
    // Event handler that handles various events in SDL
    // that are related to input and output
    SDL_Event e;
    // Enable text input
    SDL_StartTextInput();

    // Set the camera speed for how fast we move.
    float cameraSpeed = 0.05f;
    bool sprint = false;
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    int mouseX = windowWidth/2, mouseY = windowHeight/2;
    // SDL_Cursor *cursor; /* Make this variable visible in the point
    //                    where you exit the program */
    // int32_t cursorData[2] = {0, 0};
    // cursor = SDL_CreateCursor((Uint8 *)cursorData, (Uint8 *)cursorData, 8, 8, 4, 4);
    // SDL_SetCursor(cursor);
    // SDL_SetWindowGrab(m_window, SDL_TRUE);
    // SDL_ShowCursor(SDL_DISABLE);
    // SDL_SetRelativeMouseMode(SDL_TRUE);

    // While application is running
    while(!quit){

        //Handle events on queue
        while(SDL_PollEvent( &e ) != 0){
            // User posts an event to quit
            // An example is hitting the "x" in the corner of the window.
            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){
                quit = true;
            }
            // Handle keyboard input for the camera class
            if(e.type==SDL_MOUSEMOTION){
                // Handle mouse movements
                 mouseX += e.motion.xrel;
                 mouseY += e.motion.yrel;
                m_renderer->GetCamera(0)->MouseLook(mouseX, mouseY);
                // SDL_WarpMouseInWindow(m_window, windowWidth/2, windowHeight/2);
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

        // ================== Use the planets ===============
        Sun->GetLocalTransform().LoadIdentity();
        // ... transform the Sun
        Sun->GetLocalTransform().Rotate(rotate, 0.0f, 0.0f, 1.0f);

        std::vector<SceneNode*> Earths = Sun->GetChildren();
        for(int i = 0; i < Earths.size(); i++){
            SceneNode* Earth = Earths[i];
            Earth->GetLocalTransform().LoadIdentity();
            // ... transform the Earth
            Earth->GetLocalTransform().Translate(5.0f * i + 5.0f,0.0f,0.0f);
            Earth->GetLocalTransform().Scale(0.5f, 0.5f, 0.5f);
            Earth->GetLocalTransform().Rotate(rotate, 0.0f, 0.0f, 1.0f);

            std::vector<SceneNode*> Moons = Earth->GetChildren();
            float p = -1.0f;
            for(int j = 0; j < Moons.size(); j++){
                SceneNode* Moon = Moons[j];
                Moon->GetLocalTransform().LoadIdentity();
                // ... transform the Moon
                Moon->GetLocalTransform().Translate(2.0f * p, 0.0f, 0.0f);
                Moon->GetLocalTransform().Scale(0.3f, 0.3f, 0.3f);
                Moon->GetLocalTransform().Rotate(rotate, 0.0f, 0.0f, 1.0f);
                p *= -1.0f;
            }
        }

        // Update our scene through our renderer
        m_renderer->Update();
        // Render our scene using our selected renderer
        m_renderer->Render();
        // Delay to slow things down just a bit!
        // SDL_Delay(25);  // TODO: You can change this or implement a frame
                        // independent movement method if you like.
      	//Update screen of our specified window
      	SDL_GL_SwapWindow(GetSDLWindow());
        rotate += 0.02f;
	}
    //Disable text input
    SDL_StopTextInput();
    //Free memory
    delete Sun;
    // SDL_FreeCursor(cursor);
}


// Get Pointer to Window
SDL_Window* SDLGraphicsProgram::GetSDLWindow(){
  return m_window;
}

// Helper Function to get OpenGL Version Information
void SDLGraphicsProgram::GetOpenGLVersionInfo(){
	SDL_Log("(Note: If you have two GPU's, make sure the correct one is selected)");
	SDL_Log("Vendor: %s",(const char*)glGetString(GL_VENDOR));
	SDL_Log("Renderer: %s",(const char*)glGetString(GL_RENDERER));
	SDL_Log("Version: %s",(const char*)glGetString(GL_VERSION));
	SDL_Log("Shading language: %s",(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
}
