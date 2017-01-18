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
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;

    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            if (i % 5 == 0 && j % 5 == 0) {
                grid[i][j] = 1;
            } else {
                grid[i][j] = 0;
            }
        }
    }

    x = DIM-1;
    y = DIM-1;
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

void A1::initCube(int x, int y, int z)
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
    // Translate cube vertices by x,y,z.
    for(int i = 0; i < sz; i+=3) {
        verts[i] += x;
        verts[i+1] += y;
        verts[i+2] += z;
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

void A1::initSquare()
{
    glGenVertexArrays(1, &m_vao_square);
    glBindVertexArray(m_vao_square);

    // Enable the attribute index location for "position" when rendering.
    GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
    glEnableVertexAttribArray(positionAttribLocation);

    vec3 squareVertices[] = {
        vec3(-0.5f, -0.5f, 0.0f),  // Vertex 0
        vec3(0.5f, 0.5f, 0.0f),    // Vertex 1
        vec3(-0.5f, 0.5f, 0.0f),   // Vertex 2
        vec3(0.5f, -0.5f, 0.0f),   // Vertex 3
    };

    // Generate a vertex buffer object to hold the squares's vertex data.
    glGenBuffers(1, &m_vbo_square);

    //-- Copy square's vertex data to the vertex buffer:
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_square);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices,
            GL_STATIC_DRAW);
    

    // NOTE: The data type used here for indices is "GLushort", which must
    // match the index data type given to glDrawElements(...), which below
    // in our draw( ) method is GL_UNSIGNED_SHORT.
    // You can change the type, but they must match in both places.
    GLushort triangleIndices[] = {
            0,1,2,      // Triangle 0
            0,3,1       // Triangle 1
    };


    // The VAO keeps track of the last bound GL_ELEMENT_ARRAY_BUFFER, which is where
    // the indices will be stored for rendering.  Bind the VAO, and upload indices to
    // the GL_ELEMENT_ARRAY_BUFFER target.
        glGenBuffers(1, &m_ibo_square);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_square);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices,
                GL_STATIC_DRAW);


    // Tell GL how to map data from the vertex buffer "m_vbo_square" into the
    // "position" vertex attribute index of our shader program.
    // This mapping information is stored in the Vertex Array Object "m_vao_square".
    // That is why we must bind "m_vao_square" first in the line above, so
    // that "m_vao_square" captures the data mapping issued by the call to
    // glVertexAttribPointer(...).
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_square);
    //GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
    //glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    //-- Unbind target, and restore default values:
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
			// Select this colour.
		}
		ImGui::PopID();

		ImGui::PushID( 1 );
		ImGui::ColorEdit3( "##Colour", colour );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 1 ) ) {
			// Select this colour.
		}
		ImGui::PopID();

		ImGui::PushID( 2 );
		ImGui::ColorEdit3( "##Colour", colour );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 2 ) ) {
			// Select this colour.
		}
		ImGui::PopID();

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


void A1::drawSquare(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4)
{
    initTriangle(v1, v2, v4);
    glBindVertexArray(m_triangle_vao);
        glUniform3f(col_uni, 1, 0.5, 0.5);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray( 0 );

    initTriangle(v2, v3, v4);
    glBindVertexArray(m_triangle_vao);
        glUniform3f(col_uni, 1, 0.5, 0.5);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		    glUniform3f( col_uni, 1, 1, 1 );
		    glDrawArrays( GL_LINES, 0, (3+DIM)*4 );
	    glBindVertexArray( 0 );

//        initSquare();
//        glBindVertexArray(m_vao_square);
//        const GLsizei numIndices = 6;
//        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, nullptr);

        // Draw the square.
        //initTriangle(glm::vec3(0,0,0), glm::vec3(0,0,1), glm::vec3(1,0,0));
        //glBindVertexArray(m_triangle_vao);
		//    glUniform3f( col_uni, 1, 1, 1 );
        //    glDrawArrays(GL_TRIANGLES, 0, 3);
	    //glBindVertexArray( 0 );

        for (int i = 0; i < DIM; ++i) {
            for (int j = 0; j < DIM; ++j) {
                for (int k = 0; k < grid[i][j]; ++k) {
                    initCube(i, k, j);
                    glBindVertexArray(m_cube_vao);
                    glUniform3f(col_uni, 1, 1, 1);
                    glDrawArrays(GL_LINES, 0, 2*12);
                }
            }
        }           

        drawSquare(glm::vec3(x,0,y), glm::vec3(x,0,y+1), glm::vec3(x+1,0,y+1), glm::vec3(x+1,0,y));

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
        } else if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
        } else if (key == GLFW_KEY_R) {
            resetGrid();
        }
		// Respond to some key events.
	}

	return eventHandled;
}
