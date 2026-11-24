#include <iostream>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <vector>


//setting up callback error function so if the glfw failed to initiate we get to know what is the reason
void glfw_error_callback(int error, const char* description){
    std::cerr << "GLFW Error ( " << error << " ): " <<  description << std::endl;
}

//setting up the void canvas function that initiate the main canvas
void Canvas(){

    //using the glfw_error_callback function to show the reported error message on the terminal
    glfwSetErrorCallback(glfw_error_callback);

    //Initializing the glfw lib----
    if(!glfwInit()){
        std::cerr << "Error ocurred:: glfw initilization failed\n";
    }
    
    // Configuring the opengl version 
    /*glfwWinodwhint: its a fucntion used to tell GLFW what properties and version
    we want our window and its graphic context (such as OpenGL) to have before we actually create the window.
    
    In grpahic APIs like OpenGl, the version is split into two parts seprated by dotL
    . Major version (GLFW_Context_Version_Major): the first number represents a major architectural or breaking revision of the API
      (e.g., the 3 in OpenGL 3.3 or the 4 in OpenGL 4.6). changing the major version often means old functions are removed or the peipeline changes significantly.

    . Minor version (GLFW_Context_Version_Minor): the second number represents smaller feature additions, bug fixes, or extensions built on top of that 
      major version (e.g., the second 3 in OpenGL 3.3 or the 6 in  OpenGL 4.6).

    */ 

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Major Version 3 [0.1.4]
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Minor Version 3 (Together: OpenGL 3.3) [0.1.4]
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Mordern coere profile [0.1.4]

    // Creating the window Canvas and OpenGl Context
    /* here we used the glfwGetPrimaryMonitor() func, cause it returns pointer value
    to the object GLFWmonitor {which is basically an opaque C struct used by GLFW as an abstarct handle to represent a physical display connected to your computer}. 
    And we use the created vaiable 'monitor' to the store the returned pointer.*/
    GLFWmonitor* monitor = glfwGetPrimaryMonitor(); 

    /* GLFWvidmode is a typedef for a structure that describes a single video mode, including properties such as width, height, bit depths for red, green, and blue channels,
     and refresh rate in Hz. It is used in conjunction with functions like glfwGetVideoMode and glfwGetVideoModes to retrieve information about available video modes on a monitor.*/
    
     const GLFWvidmode* mode  = glfwGetVideoMode(monitor);

    

}