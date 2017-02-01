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

    // Initialize interaction mode.
    m_mode = rotateView;
    m_mode_int = 0;
    m_mode_names.push_back("Rotate View");
    m_mode_names.push_back("Translate View");
    m_mode_names.push_back("Perspective");
    m_mode_names.push_back("Rotate Model");
    m_mode_names.push_back("Translate Model");
    m_mode_names.push_back("Scale Model");
    m_mode_names.push_back("Viewport");

    // Mouse button control.
    m_mouse_left = false;
    m_mouse_middle = false;
    m_mouse_right = false;
    m_valid = false;

    // Mouse scaling.
    m_scale = 330.0f;

    // Useful points.
    O = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    e1 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    e2 = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    e3 = vec4(0.0f, 0.0f, 1.0f, 1.0f);
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

glm::vec4 A2::toPoint(const glm::vec3 &v)
{
    return vec4(v, 1.0f);
}

glm::vec4 A2::toVector(const glm::vec3 &v)
{
    return vec4(v, 0.0f);
}

glm::vec4 A2::modelToWorld(const glm::vec4 &v)
{
    glm::mat4 worldPoints = mat4(toPoint(m_model_x+m_model_origin), toPoint(m_model_y+m_model_origin),
                                 toPoint(m_model_z+m_model_origin), toPoint(m_model_origin));
    glm::mat4 modelPoints = mat4(e1, e2, e3, O);
    return worldPoints * glm::inverse(modelPoints) * v;
/*
    glm::mat4 r = mat4(vec4(m_model_x, 0.0f), vec4(m_model_y, 0.0f), vec4(m_model_z, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::mat4 t = glm::translate(mat4(1.0f), m_model_origin);
    //return r*t*v;
    return t*r*v;
*/
}

glm::vec4 A2::worldToView(const glm::vec4 &v)
{
    glm::mat4 worldPoints = mat4(toPoint(m_view_x+m_view_origin), toPoint(m_view_y+m_view_origin),
                                 toPoint(m_view_z+m_view_origin), toPoint(m_view_origin));
    glm::mat4 viewPoints = mat4(e1, e2, e3, O);
    return viewPoints * glm::inverse(worldPoints) * v;
/*
    glm::mat4 r = glm::inverse(mat4(vec4(m_view_x, 0.0f), vec4(m_view_y, 0.0f), vec4(m_view_z, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f)));
    glm::mat4 t = glm::translate(mat4(1.0f), -m_view_origin);
    //return r*t*v;
    return t*r*v;
*/
}

glm::vec4 A2::modelToView(const glm::vec4 &v)
{
    return worldToView(modelToWorld(v));
}

glm::vec2 A2::viewToDevice(const glm::vec4 &v)
{
    return vec2(v)/(tan(toRad(m_theta/2.0f))*v[2]);
}

void A2::rotateViewByAxis(double dx, Axis a) {
    glm::mat4 r;
    switch (a) {
        case(Axis::x):
            r = glm::rotate(mat4(1.0f), (float)dx/m_scale, m_view_x);
            break;
        case(Axis::y):
            r = glm::rotate(mat4(1.0f), (float)dx/m_scale, m_view_y);
            break;
        case(Axis::z):
            r = glm::rotate(mat4(1.0f), (float)dx/m_scale, m_view_z);
            break;
    }
    m_view_x = vec3(r*toVector(m_view_x));
    m_view_y = vec3(r*toVector(m_view_y));
    m_view_z = vec3(r*toVector(m_view_z));
}

void A2::translateViewByAxis(double dx, Axis a) {
    switch (a) {
        case(Axis::x):
            m_view_origin += m_view_x*(float)dx/m_scale;
            break;
        case(Axis::y):
            m_view_origin += m_view_y*(float)dx/m_scale;
            break;
        case(Axis::z):
            m_view_origin += m_view_z*(float)dx/m_scale;
            break;
    }
}

void A2::rotateModelByAxis(double dx, Axis a) {
    glm::mat4 r;
    switch (a) {
        case(Axis::x):
            r = glm::rotate(mat4(1.0f), (float)dx/m_scale, m_model_x);
            break;
        case(Axis::y):
            r = glm::rotate(mat4(1.0f), (float)dx/m_scale, m_model_y);
            break;
        case(Axis::z):
            r = glm::rotate(mat4(1.0f), (float)dx/m_scale, m_model_z);
            break;
    }
    m_model_x = vec3(r*toVector(m_model_x));
    m_model_y = vec3(r*toVector(m_model_y));
    m_model_z = vec3(r*toVector(m_model_z));
}

void A2::translateModelByAxis(double dx, Axis a) {
    switch (a) {
        case(Axis::x):
            m_model_origin += m_model_x*(float)dx/m_scale;
            break;
        case(Axis::y):
            m_model_origin += m_model_y*(float)dx/m_scale;
            break;
        case(Axis::z):
            m_model_origin += m_model_z*(float)dx/m_scale;
            break;
    }
}

float A2::clamp(float value, float low, float high)
{
    value = std::min(value, high);
    value = std::max(value, low);
    return value;
}

void A2::clipProjectDrawFromView(vec4 v1, vec4 v2)
{
    // v1 and v2 are in view coordinates.
    // First clip to near plane.
    if (v1.z < m_near && v2.z < m_near) {
        // Trivial elimination by near plane.
        return;
    } else {
        if (v1.z >= m_near && v2.z >= m_near) {
            // Trivial inclusion by near plane.
        } else {
            // Clip to near plane before projection.
            float t = (m_near - v2.z) / (v1.z - v2.z);
            if (v1.z < m_near && v2.z >= m_near)
                v1 = t*(v1-v2)+v2;
            if (v2.z < m_near && v1.z >= m_near)
                v2 = t*(v1-v2)+v2;
        }
        // Project vertices to near plane.
        vec2 w1 = viewToDevice(v1);
        vec2 w2 = viewToDevice(v2);
        // Clip 2D coordinates to [-1, 1] X [-1, 1].
        float m = -1.0f;
        float M = 1.0f;

        // X = -1 plane:
        if (w1.x < m && w2.x < m) {
            // Trivial exclusion.
            return;
        } else if (w1.x >= m && w2.x >= m) {
            // Trivial inclusion
        } else {
            float t = (m - w2.x) / (w1.x - w2.x);
            if (w1.x < m && w2.x >= m)
                w1 = t*(w1-w2) + w2;
            if (w2.x < m && w1.x >= m)
                w2 = t*(w1-w2) + w2;
        }

        // X = 1 plane:
        if (w1.x > M && w2.x > M) {
            // Trivial exclusion.
            return;
        } else if (w1.x <= M && w2.x <= M) {
            // Trivial inclusion
        } else {
            float t = (M - w2.x) / (w1.x - w2.x);
            if (w1.x > M && w2.x <= M)
                w1 = t*(w1-w2) + w2;
            if (w2.x > M && w1.x <= M)
                w2 = t*(w1-w2) + w2;
        }

        // Y = -1 plane:
        if (w1.y < m && w2.y < m) {
            // Trivial exclusion.
            return;
        } else if (w1.y >= m && w2.y >= m) {
            // Trivial inclusion
        } else {
            float t = (m - w2.y) / (w1.y - w2.y);
            if (w1.y < m && w2.y >= m)
                w1 = t*(w1-w2) + w2;
            if (w2.y < m && w1.y >= m)
                w2 = t*(w1-w2) + w2;
        }

        // Y = 1 plane:
        if (w1.y > M && w2.y > M) {
            // Trivial exclusion.
            return;
        } else if (w1.y <= M && w2.y <= M) {
            // Trivial inclusion
        } else {
            float t = (M - w2.y) / (w1.y - w2.y);
            if (w1.y > M && w2.y <= M)
                w1 = t*(w1-w2) + w2;
            if (w2.y > M && w1.y <= M)
                w2 = t*(w1-w2) + w2;
        }
        drawLine(w1, w2);
    }
}
//drawLine(viewToDevice(modelToView(O)), viewToDevice(modelToView(e1)));

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
        cube_in_view.push_back(worldToView(modelToWorld(toPoint(cube_in_model[i]))));
    }

    // Clip to near plane.
	initLineData();
	setLineColour(vec3(1.0f, 0.7f, 0.8f));

    vector<vec4> clip_cube_in_view;
    for (int i = 0; i < cube_in_view.size(); i += 2) {
        vec4 v1 = cube_in_view[i];
        vec4 v2 = cube_in_view[i+1];
        clipProjectDrawFromView(v1, v2);
        
/*
        if (v1.z < m_near && v2.z < m_near) {
            // Trivial elimination.
        } else if (v1.z >= m_near && v2.z >= m_near) {
            // Trivial inclusion.
            clip_cube_in_view.push_back(v1);
            clip_cube_in_view.push_back(v2);
        } else {
            float t = (m_near - v2.z) / (v1.z - v2.z);
            clip_cube_in_view.push_back(t*(v1-v2)+v2);
            if (v1.z < m_near && v2.z >= m_near)
                clip_cube_in_view.push_back(v2);
            if (v2.z < m_near && v1.z >= m_near)
                clip_cube_in_view.push_back(v1);
        }
*/
    }

	// Call at the beginning of frame, before drawing lines:
    //for (int i = 0; i < clip_cube_in_view.size(); i += 2) {
    //    drawLine(viewToDevice(clip_cube_in_view[i]), viewToDevice(clip_cube_in_view[i+1]));
    //}

    // Model gnomon.
	setLineColour(vec3(1.0f, 0.0f, 0.0f));
    //drawLine(viewToDevice(modelToView(O)), viewToDevice(modelToView(e1)));
    clipProjectDrawFromView(modelToView(O), modelToView(e1));
	setLineColour(vec3(0.0f, 1.0f, 0.0f));
    //drawLine(viewToDevice(modelToView(O)), viewToDevice(modelToView(e2)));
    clipProjectDrawFromView(modelToView(O), modelToView(e2));
	setLineColour(vec3(0.0f, 0.0f, 1.0f));
    //drawLine(viewToDevice(modelToView(O)), viewToDevice(modelToView(e3)));
    clipProjectDrawFromView(modelToView(O), modelToView(e3));

    // World gnomon.
	setLineColour(vec3(1.0f, 0.0f, 0.0f));
    //drawLine(viewToDevice(worldToView(O)), viewToDevice(worldToView(e1)));
    clipProjectDrawFromView(worldToView(O), worldToView(e1));
	setLineColour(vec3(0.0f, 1.0f, 0.0f));
    //drawLine(viewToDevice(worldToView(O)), viewToDevice(worldToView(e2)));
    clipProjectDrawFromView(worldToView(O), worldToView(e2));
	setLineColour(vec3(0.0f, 0.0f, 1.0f));
    //drawLine(viewToDevice(worldToView(O)), viewToDevice(worldToView(e3)));
    clipProjectDrawFromView(worldToView(O), worldToView(e3));
/*
    cout << "Start ------------_" << endl;
    cout << m_model_y << endl;
    cout << "World coord" << endl;
    cout << modelToWorld(point(m_model_y)) << endl;
    cout << "View coord" << endl;
    cout << worldToView(modelToWorld(point(m_model_y))) << endl;
    cout << "Device coord" << endl;
    cout << viewToDevice(worldToView(modelToWorld(point(m_model_y)))) << endl;
*/
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
        for (int i = Mode::rotateView; i < Mode::last; ++i) {
            ImGui::PushID(i);
            if (ImGui::RadioButton("##Mode", &m_mode_int, i)) {
                m_mode = static_cast<Mode>(i);
            }
		    ImGui::PopID();
            ImGui::SameLine();
		    ImGui::Text("%s", m_mode_names[i]);
        }


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
	if (!ImGui::IsMouseHoveringAnyWindow()) {
        double dx = xPos - m_prev_x;
        switch (m_mode) {
            case Mode::rotateView:
                if (m_mouse_left)
                    rotateViewByAxis(dx, Axis::x);
                if (m_mouse_middle)
                    rotateViewByAxis(dx, Axis::y);
                if (m_mouse_right)
                    rotateViewByAxis(dx, Axis::z);
                break;
            case Mode::translateView:
                if (m_mouse_left)
                    translateViewByAxis(dx, Axis::x);
                if (m_mouse_middle)
                    translateViewByAxis(dx, Axis::y);
                if (m_mouse_right)
                    translateViewByAxis(dx, Axis::z);
                break;
            case Mode::perspective:
                if (m_mouse_left)
                    m_theta = clamp(m_theta+(float)dx, 5.0f, 160.0f);
                if (m_mouse_middle)
                    m_near = clamp(m_near+(float)dx, 0.1f, m_far);
                if (m_mouse_right)
                    m_far = std::max(m_far+(float)dx, m_near);
                break;
            case Mode::rotateModel:
cout << "Rotating Model" << endl;
cout << m_mouse_left << m_mouse_middle << m_mouse_right << endl;
cout << m_model_origin << endl;
cout << m_model_x << endl;
cout << m_model_y << endl;
cout << m_model_z << endl;
cout << "modelToWorld(e3)" << endl;
cout << modelToWorld(e3) << endl;
cout << worldToView(modelToWorld(e3)) << endl;
//drawLine(viewToDevice(worldToView(modelToWorld(O))), viewToDevice(worldToView(modelToWorld(e2))));
                if (m_mouse_left)
                    rotateModelByAxis(dx, Axis::x);
                if (m_mouse_middle)
                    rotateModelByAxis(dx, Axis::y);
                if (m_mouse_right)
                    rotateModelByAxis(dx, Axis::z);
                break;
            case Mode::translateModel:
cout << "Translating Model" << endl;
cout << m_mouse_left << m_mouse_middle << m_mouse_right << endl;
                if (m_mouse_left)
                    translateModelByAxis(dx, Axis::x);
                if (m_mouse_middle)
                    translateModelByAxis(dx, Axis::y);
                if (m_mouse_right)
                    translateModelByAxis(dx, Axis::z);
                break;
            case Mode::scaleModel:
                break;
            case Mode::viewport:
                break;
            default:
                break;
        }
        m_prev_x = xPos;
        eventHandled = true;
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
	}

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

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (actions == GLFW_PRESS) {
                m_mouse_left = true;
            } else if (actions == GLFW_RELEASE) {
                m_mouse_left = false;
            }
            eventHandled = true;
        } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (actions == GLFW_PRESS) {
                m_mouse_middle = true;
            } else if (actions == GLFW_RELEASE) {
                m_mouse_middle = false;
            }
            eventHandled = true;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (actions == GLFW_PRESS) {
                m_mouse_right = true;
            } else if (actions == GLFW_RELEASE) {
                m_mouse_right = false;
            }
            eventHandled = true;
        }
	}

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
        }
        if (key == GLFW_KEY_LEFT) {
            glm::mat4 r = glm::rotate(mat4(1.0f), -0.2f, m_view_y);
            m_view_x = vec3(r*vec4(m_view_x, 0.0f));
            m_view_y = vec3(r*vec4(m_view_y, 0.0f));
            m_view_z = vec3(r*vec4(m_view_z, 0.0f));
        }
        if (key == GLFW_KEY_O) {
            m_mode = rotateView;
        } else if (key == GLFW_KEY_N) {
            m_mode = translateView;
        }
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT) {
        }
    }
	// Fill in with event handling code...

	return eventHandled;
}
