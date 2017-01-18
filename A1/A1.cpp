#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
static const int D = 16;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 )
{
    // Red
    colours[0][0] = 1.0f;
    colours[0][1] = 0.0f;
    colours[0][2] = 0.0f;
    // Orange
    colours[1][0] = 1.0f;
    colours[1][1] = 0.5f;
    colours[1][2] = 0.0f;
    // Yellow
    colours[2][0] = 1.0f;
    colours[2][1] = 1.0f;
    colours[2][2] = 0.0f;
    // Green
    colours[3][0] = 0.2f;
    colours[3][1] = 0.4f;
    colours[3][2] = 0.2f;
    // Blue
    colours[4][0] = 0.0f;
    colours[4][1] = 0.4f;
    colours[4][2] = 1.0f;
    // Indego
    colours[5][0] = 0.0f;
    colours[5][1] = 1.0f;
    colours[5][2] = 1.0f;
    // Violet
    colours[6][0] = 0.4f;
    colours[6][1] = 0.0f;
    colours[6][2] = 0.4f;
    // Grey-Green
    colours[7][0] = 0.4f;
    colours[7][1] = 0.6f;
    colours[7][2] = 0.6f;
    // Grey-Green
    colours[8][0] = 1.0f;
    colours[8][1] = 1.0f;
    colours[8][2] = 1.0f;

    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            grid[i][j] = 0;
            gridColours[i][j] = 0;
        }
    }

    x = 0;
    y = 0;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
    initCube();
    initSquare();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
    // The number of coordinates for representing the lines of the grid.
    // (DIM+3)*2 lines, each with 2 points, each with 3 vertices.
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initCube()
{
    // 12 edges, each with 2 points, each with 3 coordinates.
	size_t sz = 12 * 2 * 3;

	float *verts = new float[ sz ];
	size_t ct = 0;
    // Cube vertices.
    for(int i = 0; i < 2; ++i) {
        for(int j = 0; j < 2; ++j) {
            // Extending out of X-Y plane
            verts[ct] = i;
            verts[ct+1] = j;
            verts[ct+2] = 0;
            verts[ct+3] = i;
            verts[ct+4] = j;
            verts[ct+5] = 1;
            ct += 6;

            // Extending out of X-Z plane
            verts[ct] = i;
            verts[ct+1] = 0;
            verts[ct+2] = j;
            verts[ct+3] = i;
            verts[ct+4] = 1;
            verts[ct+5] = j;
            ct += 6;

            // Extending out of Y-Z plane
            verts[ct] = 0;
            verts[ct+1] = i;
            verts[ct+2] = j;
            verts[ct+3] = 1;
            verts[ct+4] = i;
            verts[ct+5] = j;
            ct += 6;
        }
    }

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STREAM_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

// Initializes a triangle parallel to the x-z plane at position x,y,z.
void A1::initTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
{
    // 3 points, each with 3 coordinates.
	size_t sz = 3 * 3;
	float *verts = new float[ sz ];
	size_t ct = 0;
    // Triangle Vertices
    for(int i = 0; i < 3; ++i) {
        glm::vec3 v = i == 0 ? v1 : (i == 1 ? v2 : v3);
        verts[ct] = v[0];
        verts[ct+1] = v[1];
        verts[ct+2] = v[2];
        ct += 3;
    }

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_triangle_vao );
	glBindVertexArray( m_triangle_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_triangle_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_triangle_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STREAM_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;
	CHECK_GL_ERRORS;
}

void A1::apply(float* a, const glm::vec3& v, int i) {
    for (int j = 0; j < 3; ++j) {
        a[i+j] = v[j];
    }
}

void A1::initSquare()
{
    // 6 points, each with 3 coordinates.
	size_t sz = 6 * 3;
	float *verts = new float[ sz ];
	size_t ct = 0;
    // Triangle Vertices
    glm::vec3 v1(0, 0, 0);
    glm::vec3 v2(0, 0, 1);
    glm::vec3 v3(1, 0, 1);
    glm::vec3 v4(1, 0, 0);
    apply(verts, v1, 0);
    apply(verts, v2, 3);
    apply(verts, v4, 6);
    apply(verts, v2, 9);
    apply(verts, v3, 12);
    apply(verts, v4, 15);

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_square_vao );
	glBindVertexArray( m_square_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_square_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_square_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STREAM_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

void A1::resetGrid()
{
    for(int i = 0; i < DIM; ++i) {
        for(int j = 0; j < DIM; ++j) {
            grid[i][j] = 0;
        }
    }
    x = 0;
    y = 0;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( ImGui::Button( "Reset Grid" ) ) {
			resetGrid();
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.
        for (int i = 0; i < 9; ++i) {
            ImGui::PushID(i);
            ImGui::ColorEdit3("##Colour", colours[i]);
            ImGui::SameLine();
            if(ImGui::RadioButton("##Col", &current_col, i)) {
                // Select this colour.
            }
		    ImGui::PopID();
        }
/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}


//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::identity(glm::mat4& m) {
    for (int i = 0; i < 4; ++i) {
        m[i] = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        m[i][i] = 1.0f;
    }
}

void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );

		// Just draw the grid for now.
        identity(W);
	    W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		glBindVertexArray( m_grid_vao );
		    glUniform3f( col_uni, 1, 1, 1 );
		    glDrawArrays( GL_LINES, 0, (3+DIM)*4 );
	    glBindVertexArray( 0 );

        // Draw the cubes.
        for (int i = 0; i < DIM; ++i) {
            for (int j = 0; j < DIM; ++j) {
                for (int k = 0; k < grid[i][j]; ++k) {
	                identity(W);
	                W = glm::translate(W, vec3( -float(DIM)/2.0f + i, k, -float(DIM)/2.0f + j ) );
		            glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

                    glBindVertexArray(m_cube_vao);
                    glUniform3f(col_uni, colours[gridColours[i][j]][0], colours[gridColours[i][j]][1], colours[gridColours[i][j]][2]);
                    glDrawArrays(GL_LINES, 0, 2*12);
                }
            }
        }           

        // Draw the square.
        identity(W);
        W = glm::translate(W, vec3( -float(DIM)/2.0f + x, grid[x][y], -float(DIM)/2.0f + y) );
        glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		glBindVertexArray( m_square_vao );
		    glUniform3f( col_uni, 1, 0.5, 0.5 );
		    glDrawArrays( GL_TRIANGLES, 0, 6);
	    glBindVertexArray( 0 );
        //drawSquare(glm::vec3(x,0,y), glm::vec3(x,0,y+1), glm::vec3(x+1,0,y+1), glm::vec3(x+1,0,y));

	m_shader.disable();

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
        if (key == GLFW_KEY_RIGHT) {
            x = std::min(D-1, x+1);
        } else if (key == GLFW_KEY_LEFT) {
            x = std::max(0, x-1);
        } else if (key == GLFW_KEY_UP) {
            y = std::max(0, y-1);
        } else if (key == GLFW_KEY_DOWN) {
            y = std::min(D-1, y+1);
        } else if (key == GLFW_KEY_SPACE) {
            ++grid[x][y];
            gridColours[x][y] = current_col; 
        } else if (key == GLFW_KEY_BACKSPACE) {
            grid[x][y] = std::max(0, grid[x][y]-1);
        } else if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
        } else if (key == GLFW_KEY_R) {
            resetGrid();
        }
		// Respond to some key events.
	}

	return eventHandled;
}
