#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include <math.h>

#define PI 3.14159265f
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

    // Initialize coordinate systems.
    m_model_origin = vec3(0.0f, 0.0f, 0.0f);
    m_model_x = vec3(1.0f, 0.0f, 0.0f);
    m_model_y = vec3(0.0f, 1.0f, 0.0f);
    m_model_z = vec3(0.0f, 0.0f, 1.0f);

    m_view_origin = vec3(0.0f, 0.0f, 8.0f);
    m_view_x = vec3(-1.0f, 0.0f, 0.0f);
    m_view_y = vec3(0.0f, 1.0f, 0.0f);
    m_view_z = vec3(0.0f, 0.0f, -1.0f);

    // Initialize field of view parameters.
    m_theta = 90.0f;
    m_near = 1.0f;
    m_far = 20.0f;
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}
/*
void A1::identity(glm::mat4& m) {
    for (int i = 0; i < 4; ++i) {
        m[i] = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        m[i][i] = 1.0f;
    }
}

void A1::orient(glm::mat4& m) {
    identity(m);
    m = glm::scale(m, vec3(scale, scale, scale));
    m = glm::rotate(m, (float)rotation/200.0f, vec3(0, 1, 0));
	m = glm::translate(m, vec3(-float(DIM)/2.0f, 0, -float(DIM)/2.0f));
}
*/
float A2::toRad(float deg)
{
    return deg*PI/180.0f;
}

glm::mat4 A2::translation(const glm::vec3 &v)
{
    return glm::mat4(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f),
                     vec4(0.0f, 0.0f, 1.0f, 0.0f), vec4(v, 1.0f));
}

glm::vec4 A2::point(const glm::vec3 &v)
{
    return vec4(v, 1.0f);
}

glm::vec4 A2::modelToWorld(const glm::vec4 &v)
{
    glm::mat4 r = mat4(vec4(m_model_x, 0.0f), vec4(m_model_y, 0.0f), vec4(m_model_z, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::mat4 t = glm::translate(mat4(1.0f), m_model_origin);
    return r*t*v;
}

glm::vec4 A2::worldToView(const glm::vec4 &v)
{
    glm::mat4 r = glm::inverse(mat4(vec4(m_view_x, 0.0f), vec4(m_view_y, 0.0f), vec4(m_view_z, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f)));
    glm::mat4 t = glm::translate(mat4(1.0f), -m_view_origin);
    return r*t*v;
}

glm::vec2 A2::viewToDevice(const glm::vec4 &v)
{
    return vec2(v)/(tan(toRad(m_theta/2.0f))*v[2]);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here:
    // Transform model -> view.
    // Stemming from (-1,-1,-1).
    vector<vec3> cube_in_model;
    cube_in_model.push_back(vec3(-1.0f, -1.0f, -1.0f));
    cube_in_model.push_back(vec3(-1.0f, -1.0f, 1.0f));
//    cube_in_model.push_back(vec3(-1.0f, -1.0f, -1.0f));
//    cube_in_model.push_back(vec3(-1.0f, 1.0f, -1.0f));
//    cube_in_model.push_back(vec3(-1.0f, -1.0f, -1.0f));
//    cube_in_model.push_back(vec3(1.0f, -1.0f, -1.0f));
    // Stemming from (1,1,1).
    cube_in_model.push_back(vec3(1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(1.0f, 1.0f, -1.0f));
    cube_in_model.push_back(vec3(1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(1.0f, -1.0f, 1.0f));
    cube_in_model.push_back(vec3(1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(-1.0f, 1.0f, 1.0f));
    // Stemming from (1, 1, -1).
//    cube_in_model.push_back(vec3(1.0f, 1.0f, -1.0f));
//    cube_in_model.push_back(vec3(1.0f, -1.0f, -1.0f));
//    cube_in_model.push_back(vec3(1.0f, 1.0f, -1.0f));
//    cube_in_model.push_back(vec3(-1.0f, 1.0f, -1.0f));
    // Stemming from (1, -1, 1).
    cube_in_model.push_back(vec3(1.0f, -1.0f, 1.0f));
    cube_in_model.push_back(vec3(1.0f, -1.0f, -1.0f));
    cube_in_model.push_back(vec3(1.0f, -1.0f, 1.0f));
    cube_in_model.push_back(vec3(-1.0f, -1.0f, 1.0f));
    // Stemming from (-1, 1, 1).
    cube_in_model.push_back(vec3(-1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(-1.0f, 1.0f, -1.0f));
    cube_in_model.push_back(vec3(-1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(-1.0f, -1.0f, 1.0f));

    vector<vec4> cube_in_view;
    for (int i = 0; i < cube_in_model.size(); ++i) {
        cube_in_view.push_back(modelToWorld(worldToView(point(cube_in_model[i]))));
    }

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	setLineColour(vec3(1.0f, 0.7f, 0.8f));
    for (int i = 0; i < cube_in_view.size(); i += 2) {
        drawLine(viewToDevice(cube_in_view[i]), viewToDevice(cube_in_view[i+1]));
    }

	setLineColour(vec3(1.0f, 0.0f, 0.0f));
    drawLine(viewToDevice(worldToView(modelToWorld(vec4(0.0f, 0.0f, 0.0f, 1.0f)))), viewToDevice(worldToView(modelToWorld(point(m_model_x)))));
	setLineColour(vec3(0.0f, 1.0f, 0.0f));
    drawLine(viewToDevice(worldToView(modelToWorld(vec4(0.0f, 0.0f, 0.0f, 1.0f)))), viewToDevice(worldToView(modelToWorld(point(m_model_y)))));
	setLineColour(vec3(0.0f, 0.0f, 1.0f));
    drawLine(viewToDevice(worldToView(modelToWorld(vec4(0.0f, 0.0f, 0.0f, 1.0f)))), viewToDevice(worldToView(modelToWorld(point(m_model_z)))));

    cout << "Start ------------_" << endl;
    cout << m_model_y << endl;
    cout << "World coord" << endl;
    cout << modelToWorld(point(m_model_y)) << endl;
    cout << "View coord" << endl;
    cout << worldToView(modelToWorld(point(m_model_y))) << endl;
    cout << "Device coord" << endl;
    cout << viewToDevice(worldToView(modelToWorld(point(m_model_y)))) << endl;
/*
	// Draw outer square:
	setLineColour(vec3(1.0f, 0.7f, 0.8f));
	drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
	drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
	drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
	drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));


	// Draw inner square:
	setLineColour(vec3(0.2f, 1.0f, 1.0f));
	drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
	drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
	drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
	drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));
*/
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);
	if( action == GLFW_PRESS ) {
        eventHandled = true;
        if (key == GLFW_KEY_RIGHT) {
            glm::mat4 r = glm::rotate(mat4(1.0f), 0.2f, m_view_y);
            m_view_x = vec3(r*vec4(m_view_x, 0.0f));
            m_view_y = vec3(r*vec4(m_view_y, 0.0f));
            m_view_z = vec3(r*vec4(m_view_z, 0.0f));
            cout << "HHH" << endl;
            cout << m_view_x << endl;
        }
        if (key == GLFW_KEY_LEFT) {
            glm::mat4 r = glm::rotate(mat4(1.0f), -0.2f, m_view_y);
            m_view_x = vec3(r*vec4(m_view_x, 0.0f));
            m_view_y = vec3(r*vec4(m_view_y, 0.0f));
            m_view_z = vec3(r*vec4(m_view_z, 0.0f));
            cout << "HHH" << endl;
            cout << m_view_x << endl;
        }
    }
	// Fill in with event handling code...

	return eventHandled;
}
