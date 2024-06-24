#include"windowManager.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

Window::Window() {
	window = nullptr;
}

void Window::init(const char *windowName) {
	initWindow(&window, windowName);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); // Disables vsync
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	glfwSetWindowPos(window, 1300, MonitorSize.y / 2 + WindowSize.y / 2);
}

void initWindow(GLFWwindow** window, const char *windowName) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_FLOATING, true);

	*window = glfwCreateWindow(WindowSize.x, WindowSize.y, windowName, NULL, NULL);
	if (*window == NULL) {
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window!");
	}
	glfwMakeContextCurrent(*window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwTerminate();
		throw std::runtime_error("Failed to initialize GLAD!");
	}

	//glGenFramebuffers(1, &FramebufferID);
	//glBindFramebuffer(GL_FRAMEBUFFER, FramebufferID);

	glViewport(0, 0, WindowSize.x, WindowSize.y);
	// To make sure that the glfw window and gl viewport always match, we must
	// set a callback function to be run every time glfw resizes the window!
	glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
}