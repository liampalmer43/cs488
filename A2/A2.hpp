#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};

enum Mode {
    rotateView,
    translateView,
    
    perspective,

    rotateModel,
    translateModel,
    scaleModel,

    viewport,

    last
};

enum Axis {
    x, y, z
};

class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

    glm::mat4 translation(const glm::vec3 &v);
    glm::vec4 toPoint(const glm::vec3 &v);
    glm::vec4 toVector(const glm::vec3 &v);
    float toRad(float deg);
    glm::vec4 modelToWorld(const glm::vec4 &v);
    glm::vec4 worldToView(const glm::vec4 &v);
    glm::vec4 modelToView(const glm::vec4 &v);
    glm::vec2 viewToDevice(const glm::vec4 &v);
    void rotateViewByAxis(double dx, Axis a);
    void translateViewByAxis(double dx, Axis a);
    void rotateModelByAxis(double dx, Axis a);
    void translateModelByAxis(double dx, Axis a);
    float clamp(float value, float low, float high);
    void clipProjectDrawFromView(glm::vec4 v1, glm::vec4 v2);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

    // Model coordinate frame.
    glm::vec3 m_model_origin;
    glm::vec3 m_model_x;
    glm::vec3 m_model_y;
    glm::vec3 m_model_z;
    float m_model_scale_x;
    float m_model_scale_y;
    float m_model_scale_z;

    // View coordinate frame.
    glm::vec3 m_view_origin;
    glm::vec3 m_view_x;
    glm::vec3 m_view_y;
    glm::vec3 m_view_z;

    // Field of view variables.
    float m_theta;
    float m_near;
    float m_far;

    // Current interaction mode.
    Mode m_mode;
    int m_mode_int;
    std::vector<const char*> m_mode_names;
    
    // Mouse event control.
    bool m_mouse_left;
    bool m_mouse_middle;
    bool m_mouse_right;
    double m_prev_x;
    float m_mouse_pos_x;
    float m_mouse_pos_y;

    // Mouse scaling.
    float m_rotation_scale;
    float m_translation_scale;

    // Useful points.
    glm::vec4 O;
    glm::vec4 e1;
    glm::vec4 e2;
    glm::vec4 e3;

    // Window parameters.
    int m_width;
    int m_height;

    // Viewport parameters.
    float m_viewport_x1;
    float m_viewport_y1;
    float m_viewport_x2;
    float m_viewport_y2;
};
