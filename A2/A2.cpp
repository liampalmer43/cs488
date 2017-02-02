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

    // Initialize various data.
    reset();

    // Mode names.
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
    m_mouse_pos_x = 1.0f;
    m_mouse_pos_y = 1.0f;

    // Mouse scaling.
    m_rotation_scale = 200.0f;
    m_translation_scale = 100.0f;

    // Useful points.
    O = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    e1 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    e2 = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    e3 = vec4(0.0f, 0.0f, 1.0f, 1.0f);

    // Window parameters.
    m_width = 650;
    m_height = 650;

}

void A2::reset()
{
    // Initialize coordinate systems.
    m_model_origin = vec3(0.0f, 0.0f, 0.0f);
    m_model_x = vec3(1.0f, 0.0f, 0.0f);
    m_model_y = vec3(0.0f, 1.0f, 0.0f);
    m_model_z = vec3(0.0f, 0.0f, 1.0f);
    m_model_scale_x = 1.0f;
    m_model_scale_y = 1.0f;
    m_model_scale_z = 1.0f;

    m_view_origin = vec3(0.0f, 0.0f, 15.0f);
    m_view_x = vec3(-1.0f, 0.0f, 0.0f);
    m_view_y = vec3(0.0f, 1.0f, 0.0f);
    m_view_z = vec3(0.0f, 0.0f, -1.0f);

    // Initialize field of view parameters.
    m_theta = 30.0f;
    m_near = 1.0f;
    m_far = 20.0f;

    // Initialize interaction mode.
    m_mode = rotateModel;
    m_mode_int = static_cast<int>(m_mode);

    // Viewport parameters.
    m_viewport_x1 = -0.9f;
    m_viewport_y1 = 0.9f;
    m_viewport_x2 = 0.9f;
    m_viewport_y2 = -0.9f;
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

float A2::toRad(float deg)
{
    return deg*PI/180.0f;
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

glm::mat4 A2::getRotationMatrix(float a, const glm::vec3 &u)
{
    mat4 r = mat4(1.0f);
    float d1 = cos(a)+u.x*u.x*(1-cos(a));
    float v1 = u.x*u.y*(1-cos(a))-u.z*sin(a);
    float v2 = u.x*u.z*(1-cos(a))+u.y*sin(a);
    float d2 = cos(a)+u.y*u.y*(1-cos(a));
    float v3 = u.y*u.z*(1-cos(a))-u.x*sin(a);
    float d3 = cos(a)+u.z*u.z*(1-cos(a));
    r[0][0] = d1;
    r[1][1] = d2;
    r[2][2] = d3;

    r[1][0] = v1;
    r[0][1] = v1;

    r[2][0] = v2;
    r[0][2] = v2;

    r[2][1] = v3;
    r[1][2] = v3;
cout << r << endl;

    return r;
}

glm::mat4 A2::getTranslationMatrix(const glm::vec3 &v)
{
    return glm::mat4(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f),
                     vec4(0.0f, 0.0f, 1.0f, 0.0f), vec4(v, 1.0f));
}

void A2::rotateViewByAxis(double dx, Axis a) {
    glm::mat4 r;
    switch (a) {
        case(Axis::x):
            r = glm::rotate(mat4(1.0f), (float)dx/m_rotation_scale, m_view_x);
            break;
        case(Axis::y):
            r = glm::rotate(mat4(1.0f), (float)dx/m_rotation_scale, m_view_y);
            break;
        case(Axis::z):
            r = glm::rotate(mat4(1.0f), (float)dx/m_rotation_scale, m_view_z);
            break;
    }
    m_view_x = vec3(r*toVector(m_view_x));
    m_view_y = vec3(r*toVector(m_view_y));
    m_view_z = vec3(r*toVector(m_view_z));
}

void A2::translateViewByAxis(double dx, Axis a) {
    switch (a) {
        case(Axis::x):
            m_view_origin += m_view_x*(float)dx/m_translation_scale;
            break;
        case(Axis::y):
            m_view_origin += m_view_y*(float)dx/m_translation_scale;
            break;
        case(Axis::z):
            m_view_origin += m_view_z*(float)dx/m_translation_scale;
            break;
    }
}

void A2::rotateModelByAxis(double dx, Axis a) {
    glm::mat4 r;
    switch (a) {
        case(Axis::x):
            //r = getRotationMatrix((float)dx/m_rotation_scale, m_model_x);
            r = glm::rotate(mat4(1.0f), (float)dx/m_rotation_scale, m_model_x);
            break;
        case(Axis::y):
            r = glm::rotate(mat4(1.0f), (float)dx/m_rotation_scale, m_model_y);
            break;
        case(Axis::z):
            r = glm::rotate(mat4(1.0f), (float)dx/m_rotation_scale, m_model_z);
            break;
    }
    m_model_x = vec3(r*toVector(m_model_x));
    m_model_y = vec3(r*toVector(m_model_y));
    m_model_z = vec3(r*toVector(m_model_z));
}

void A2::translateModelByAxis(double dx, Axis a) {
    switch (a) {
        case(Axis::x):
            m_model_origin += m_model_x*(float)dx/m_translation_scale;
            break;
        case(Axis::y):
            m_model_origin += m_model_y*(float)dx/m_translation_scale;
            break;
        case(Axis::z):
            m_model_origin += m_model_z*(float)dx/m_translation_scale;
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

    // Trivial elimination by near plane.
    if (v1.z < m_near && v2.z < m_near)
        return;
    // Trivial elimination by far plane.
    if (v1.z > m_far && v2.z > m_far)
        return;

    // Clip to near plane.
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

    // Clip to far plane.
    if (v1.z <= m_far && v2.z <= m_far) {
        // Trivial inclusion by far plane.
    } else {
        // Clip to far plane before projection.
        float t = (m_far - v2.z) / (v1.z - v2.z);
        if (v1.z > m_far && v2.z <= m_far)
            v1 = t*(v1-v2)+v2;
        if (v2.z > m_far && v1.z <= m_far)
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

    // Translate w1 and w2 to the viewport.
    vec2 top_left = vec2(std::min(m_viewport_x1, m_viewport_x2), std::max(m_viewport_y1, m_viewport_y2));
    vec2 bottom_right = vec2(std::max(m_viewport_x1, m_viewport_x2), std::min(m_viewport_y1, m_viewport_y2));
    float a = top_left.x;
    float b = top_left.y;
    float c = bottom_right.x;
    float d = bottom_right.y;
    float A = (c-a)/2.0f;
    float B = 0.0f;
    float C = (c+a)/2.0f;
    float D = 0.0f;
    float E = (b-d)/2.0f;
    float F = (b+d)/2.0f;
    vec2 w1p = vec2(A*w1.x + B*w1.y + C, D*w1.x + E*w1.y + F);
    vec2 w2p = vec2(A*w2.x + B*w2.y + C, D*w2.x + E*w2.y + F);

    drawLine(w1p, w2p);
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
    cube_in_model.push_back(vec3(-1.0f, -1.0f, -1.0f));
    cube_in_model.push_back(vec3(-1.0f, 1.0f, -1.0f));
    cube_in_model.push_back(vec3(-1.0f, -1.0f, -1.0f));
    cube_in_model.push_back(vec3(1.0f, -1.0f, -1.0f));
    // Stemming from (1,1,1).
    cube_in_model.push_back(vec3(1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(1.0f, 1.0f, -1.0f));
    cube_in_model.push_back(vec3(1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(1.0f, -1.0f, 1.0f));
    cube_in_model.push_back(vec3(1.0f, 1.0f, 1.0f));
    cube_in_model.push_back(vec3(-1.0f, 1.0f, 1.0f));
    // Stemming from (1, 1, -1).
    cube_in_model.push_back(vec3(1.0f, 1.0f, -1.0f));
    cube_in_model.push_back(vec3(1.0f, -1.0f, -1.0f));
    cube_in_model.push_back(vec3(1.0f, 1.0f, -1.0f));
    cube_in_model.push_back(vec3(-1.0f, 1.0f, -1.0f));
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

    // Scale cube appropriately.
    for (int i = 0; i < cube_in_model.size(); ++i) {
        cube_in_model[i].x *= m_model_scale_x;
        cube_in_model[i].y *= m_model_scale_y;
        cube_in_model[i].z *= m_model_scale_z;
    }

    // Move cube to view space.
    vector<vec4> cube_in_view;
    for (int i = 0; i < cube_in_model.size(); ++i) {
        cube_in_view.push_back(worldToView(modelToWorld(toPoint(cube_in_model[i]))));
    }

    // Draw cube.
	initLineData();
	setLineColour(vec3(1.0f, 0.7f, 0.8f));
    for (int i = 0; i < cube_in_view.size(); i += 2) {
        clipProjectDrawFromView(cube_in_view[i], cube_in_view[i+1]);
    }
        
    // Model gnomon.
	setLineColour(vec3(1.0f, 0.0f, 0.0f));
    clipProjectDrawFromView(modelToView(O), modelToView(e1));
	setLineColour(vec3(0.0f, 1.0f, 0.0f));
    clipProjectDrawFromView(modelToView(O), modelToView(e2));
	setLineColour(vec3(0.0f, 0.0f, 1.0f));
    clipProjectDrawFromView(modelToView(O), modelToView(e3));

    // World gnomon.
	setLineColour(vec3(1.0f, 0.0f, 0.0f));
    clipProjectDrawFromView(worldToView(O), worldToView(e1));
	setLineColour(vec3(0.0f, 1.0f, 0.0f));
    clipProjectDrawFromView(worldToView(O), worldToView(e2));
	setLineColour(vec3(0.0f, 0.0f, 1.0f));
    clipProjectDrawFromView(worldToView(O), worldToView(e3));

	setLineColour(vec3(1.0f, 1.0f, 1.0f));
    drawLine(vec2(m_viewport_x1, m_viewport_y1), vec2(m_viewport_x2, m_viewport_y1));
    drawLine(vec2(m_viewport_x1, m_viewport_y2), vec2(m_viewport_x2, m_viewport_y2));
    drawLine(vec2(m_viewport_x1, m_viewport_y1), vec2(m_viewport_x1, m_viewport_y2));
    drawLine(vec2(m_viewport_x2, m_viewport_y1), vec2(m_viewport_x2, m_viewport_y2));
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
        cout << ImGui::GetWindowSize().x << " , h: " << ImGui::GetWindowSize().y << endl;
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( ImGui::Button( "Reset Application" ) ) {
			reset();
		}

		ImGui::Text("CS 488: A2 Pipeline");
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
		ImGui::Text( "Near Plane: %.1f", m_near);
		ImGui::Text( "Far Plane: %.1f", m_far);
		ImGui::Text( "Field of View: %.1f Deg", m_theta);

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
        // Update mouse variables.
        m_mouse_pos_x = xPos;
        m_mouse_pos_y = yPos;

        // Update rotations/translations/scaling...
        double dx = xPos - m_prev_x;
        float d;
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
                d = (float)dx/16.0f;
                if (m_mouse_left)
                    m_theta = clamp(m_theta+(float)dx/2.0f, 5.0f, 160.0f);
                if (m_mouse_middle)
                    m_near = clamp(m_near+d, 0.1f, m_far);
                if (m_mouse_right)
                    m_far = std::max(m_far+d, m_near);
                break;
            case Mode::rotateModel:
                if (m_mouse_left)
                    rotateModelByAxis(dx, Axis::x);
                if (m_mouse_middle)
                    rotateModelByAxis(dx, Axis::y);
                if (m_mouse_right)
                    rotateModelByAxis(dx, Axis::z);
                break;
            case Mode::translateModel:
                if (m_mouse_left)
                    translateModelByAxis(dx, Axis::x);
                if (m_mouse_middle)
                    translateModelByAxis(dx, Axis::y);
                if (m_mouse_right)
                    translateModelByAxis(dx, Axis::z);
                break;
            case Mode::scaleModel:
                d = (float)dx/30.0f;
                if (m_mouse_left)
                    m_model_scale_x = clamp(m_model_scale_x+d, 0.1f, 100.0f);
                if (m_mouse_middle)
                    m_model_scale_y = clamp(m_model_scale_y+d, 0.1f, 100.0f);
                if (m_mouse_right)
                    m_model_scale_z = clamp(m_model_scale_z+d, 0.1f, 100.0f);
                break;
            case Mode::viewport:
                // Update viewport corner.
                if (m_mouse_left) {
                    float x = clamp(2.0f*(m_mouse_pos_x/m_width)-1.0f, -1.0f, 1.0f);
                    float y = clamp(-2.0f*(m_mouse_pos_y/m_height)+1.0f, -1.0f, 1.0f);
                    m_viewport_x2 = x;
                    m_viewport_y2 = y;
                }
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
                if (m_mode == Mode::viewport) {
                    float x = clamp(2.0f*(m_mouse_pos_x/m_width)-1.0f, -1.0f, 1.0f);
                    float y = clamp(-2.0f*(m_mouse_pos_y/m_height)+1.0f, -1.0f, 1.0f);
                    m_viewport_x1 = x;
                    m_viewport_x2 = x;
                    m_viewport_y1 = y;
                    m_viewport_y2 = y;
                }

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

    m_width = width;
    m_height = height;
    eventHandled = true;

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
	if (action == GLFW_PRESS) {
        eventHandled = true;
        if (key == GLFW_KEY_O) {
            m_mode = Mode::rotateView;
        } else if (key == GLFW_KEY_N) {
            m_mode = Mode::translateView;
        } else if (key == GLFW_KEY_P) {
            m_mode = Mode::perspective;
        } else if (key == GLFW_KEY_R) {
            m_mode = Mode::rotateModel;
        } else if (key == GLFW_KEY_T) {
            m_mode = Mode::translateModel;
        } else if (key == GLFW_KEY_S) {
            m_mode = Mode::scaleModel;
        } else if (key == GLFW_KEY_V) {
            m_mode = Mode::viewport;
        } else if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
        } else if (key == GLFW_KEY_A) {
            reset();
        } else {
            eventHandled = false;
        }
        m_mode_int = static_cast<int>(m_mode);
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_SHIFT) {
        }
    }
	// Fill in with event handling code...

	return eventHandled;
}
