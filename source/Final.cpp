#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <map>
#include <camera.h>

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp includes
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// font
# include <ft2build.h>
#include FT_FREETYPE_H

#include <random>

FT_Library ft;
FT_Face face;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


// Loading photos
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Project includes
#include "maths_funcs.h"

//Teapot
const char *textureName = "./textures/lightbrown.tga";
int noOfMasterHairs = 0;
// Hair/fur variables
int noofHairSegments = 12; // IF YOU CHANGE THIS, DO NOT FORGET TO CHANGE IN furSimulation.compute AND furShader.gs TOO!!
float hairSegmentLength = 0.00005f;
const int nrOfDataVariablesPerMasterHair = 1; // position


GLuint hairDataTextureID_rest ;
GLuint hairDataTextureID_last ;
GLuint hairDataTextureID_current ;
GLuint hairDataTextureID_simulated ;

// To be able to add randomness to each hair strand
GLuint randomDataTextureID ;


/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
#define SQUARE_MESH "./models/teapot.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

// Camera
//glm::vec3 cameraPosition = glm::vec3(0.0f, 6.0f, 50.0f);
Camera camera(glm::vec3(12.0f, 0.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f), 180, 0);
glm::vec3 lightPos(0.5f, 3.0f, 5.3f);

unsigned int planeVAO = 0;

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
    //add mtangents and mbitangents here
    //add mtangents and mbitangents here
    std::vector<vec3> mTangents;
    std::vector<vec3> mBitangents;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint hairShaderProgramID, skyboxShaderProgramID, textShaderProgramID;

ModelData mesh_data;
int width = 1920;
int height = 1080;

float delta;
GLuint loc1, loc2, loc3, loc4, loc5;
GLfloat rotate_y = 0.0f;

const GLuint i = 15;
GLuint VAO[i], VBO[i * 2], VTO[i];
std::vector < ModelData > meshData;
std::vector < const char* > dataArray;


bool mappingOn = false;


#pragma region SKYBOX
// ------------ SKYBOX ------------
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;
unsigned int furTexture;
vector<std::string> faces
        {
                "./skybox/1x+.png",
                "./skybox/1x-.png",
                "./skybox/1y+.png",
                "./skybox/1y-.png",
                "./skybox/1z+.png",
                "./skybox/1z-.png"
        };
float skyboxVertices[] = {
	-200.0f,  200.0f, -200.0f,
	-200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f, -200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,

	-200.0f, -200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f,
	-200.0f, -200.0f,  200.0f,

	-200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f, -200.0f,
	 200.0f,  200.0f,  200.0f,
	 200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f,  200.0f,
	-200.0f,  200.0f, -200.0f,

	-200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f, -200.0f,
	 200.0f, -200.0f, -200.0f,
	-200.0f, -200.0f,  200.0f,
	 200.0f, -200.0f,  200.0f
};
#pragma endregion SKYBOX
#pragma region MESH LOADING

// ------------ FREETYPE/TEXT/FONT ------------
// This code has been taken from https://learnopengl.com/In-Practice/Text-Rendering.
#pragma region TEXT
unsigned int textVAO, textVBO;

// This struct is used to store the individual character information.
struct Character {
	unsigned int TextureID;
	glm::ivec2   Size;
	glm::ivec2   Bearing;
	long Advance;
};

std::map<char, Character> Characters;

int createFont() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "Error loading FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "./fonts/arial.ttf", 0, &face))
    {
        std::cout << "Error loading Font" << std::endl;
        return -1;
    }
    else {
        FT_Set_Pixel_Sizes(face, 0, 25);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "Error loading Glyph" << std::endl;
                continue;
            }
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character character = {
                    texture,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    face->glyph->advance.x
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void RenderText(GLuint shaderProgramID, std::string text, float x, float y, float scale, glm::vec3 color)
{
    glUseProgram(shaderProgramID);
    glUniform3f(glGetUniformLocation(shaderProgramID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
    ModelData modelData;

    /* Use assimp to read the model file, forcing it to be read as    */
    /* triangles. The second flag (aiProcess_PreTransformVertices) is */
    /* relevant if there are multiple meshes in the model file that   */
    /* are offset from the origin. This is pre-transform them so      */
    /* they're in the right position.                                 */
    const aiScene* scene = aiImportFile(
            file_name,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PreTransformVertices | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace
    );

    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return modelData;
    }

    printf("  %i materials\n", scene->mNumMaterials);
    printf("  %i meshes\n", scene->mNumMeshes);
    printf("  %i textures\n", scene->mNumTextures);

    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        printf("    %i vertices in mesh\n", mesh->mNumVertices);
        modelData.mPointCount += mesh->mNumVertices;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
            }
            if (mesh->HasTangentsAndBitangents()) {
                const aiVector3D* vta = &(mesh->mTangents[v_i]);
                modelData.mTangents.push_back(vec3(vta->x, vta->y, vta->z));

                const aiVector3D* vbt = &(mesh->mBitangents[v_i]);
                modelData.mBitangents.push_back(vec3(vbt->x, vbt->y, vbt->z));
            }
        }
    }

    aiReleaseImport(scene);
    return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertexShader, const char* fragmentShader, const char* tessellation_control_shader_filename = nullptr, const char* tessellation_eval_shader_filename = nullptr, const char* geometry_shader_filename = nullptr)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vertexShader, GL_VERTEX_SHADER);

    //The Tessellation Control Shader and Tessellation Evaluation Shader work together to add extra detail
    // to a model at runtime by subdividing the model's surfaces into smaller patches, allowing for smoother
    // and more complex curves. The Tessellation Control Shader defines the size and shape of the patches,
    // while the Tessellation Evaluation Shader defines how the vertices of the patches are created.
    if (tessellation_control_shader_filename != nullptr) {
        AddShader(shaderProgramID, tessellation_control_shader_filename, GL_TESS_CONTROL_SHADER);
    }
    if (tessellation_eval_shader_filename != nullptr) {
        AddShader(shaderProgramID, tessellation_eval_shader_filename, GL_TESS_EVALUATION_SHADER);
    }
    //The Geometry Shader allows for the manipulation of primitive shapes such as points, lines, and triangles before they are rasterized to the screen.
    // This shader can be used to create effects such as particle systems, wireframes, and billboards, among others.
    // For hair rendering, it's to create lines to represent the strands.
    if (geometry_shader_filename != nullptr) {
        AddShader(shaderProgramID, geometry_shader_filename, GL_GEOMETRY_SHADER);
    }
    AddShader(shaderProgramID, fragmentShader, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

unsigned int loadTexture(const char* texture, int i) {
    glGenTextures(1, &VTO[i]);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(texture, &width, &height, &nrComponents, 0);

    GLenum format;
    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, VTO[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return VTO[i];
}


GLuint loadCubemap(vector<std::string> faces)
{
	GLuint skyboxTextureID;
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return skyboxTextureID;
}
// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(std::vector < const char* > dataArray) {
    int counter = 0;

    loc1 = glGetAttribLocation(hairShaderProgramID, "position");
    loc2 = glGetAttribLocation(hairShaderProgramID, "normal");
    loc3 = glGetAttribLocation(hairShaderProgramID, "TexCoord");

    for (int i = 0; i < dataArray.size(); i++) {
        mesh_data = load_mesh(dataArray[i]);
        meshData.push_back(mesh_data);

        glGenBuffers(1, &VBO[counter]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[counter]);
        glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &VBO[counter + 1]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 1]);
        glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

        glGenBuffers(1, &VBO[counter + 2]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 2]);
        glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);

        glGenVertexArrays(1, &VAO[i]);
        glBindVertexArray(VAO[i]);

        glEnableVertexAttribArray(loc1);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[counter]);
        glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glEnableVertexAttribArray(loc2);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 1]);
        glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        glEnableVertexAttribArray(loc3);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[counter + 2]);
        glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        counter += 3;
    }
}

void generateFontBufferObjects() {
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void generateSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}
#pragma endregion VBO_FUNCTIONS




GLfloat* createMasterHairs(){

    std::vector<vec3> vertexArray = meshData[0].mVertices;
    std::vector<vec3> vertexNormal = meshData[0].mNormals;
    noOfMasterHairs = meshData[0].mPointCount;
    glm::mat4 model;
    // Calculate the amount of data per hair strand (number of segments * 4 (for position and normal data) * number of data variables per strand)
    int amountOfDataPerMasterHair = noofHairSegments * 4 * nrOfDataVariablesPerMasterHair;
    // Allocate memory for the hair data
    GLfloat* hairData = new GLfloat[noOfMasterHairs * amountOfDataPerMasterHair];

    int masterHairIndex = 0;

    for(int i = 0; i < noOfMasterHairs; i ++){
        // Get the root position of the hair strand
        glm::vec4 rootPos = glm::vec4(vertexArray[i].v[0], vertexArray[i].v[1], vertexArray[i].v[2], 1.f);
        rootPos = model * rootPos;
        //// Get the root position of the hair strand
        glm::vec4 rootNormal = glm::vec4(vertexNormal[i].v[0], vertexNormal[i].v[1], vertexNormal[i].v[2], 0.f);
//        glm::vec4 rootNormal = glm::vec4(-vertexArray[i].v[1], -vertexArray[i].v[0], -vertexArray[i].v[2], 0.f);
//        glm::vec4 rootNormal = glm::vec4(0.0f, 0.0f, 0.0f, 0.f);
        // Store the root position data in the hair data array
        hairData[masterHairIndex++] = rootPos.x;
        hairData[masterHairIndex++] = rootPos.y;
        hairData[masterHairIndex++] = rootPos.z;
        hairData[masterHairIndex++] = rootPos.w;

        // Loop through all the segments of the hair strand (excluding the root)
        //// Calculate the new position of the hair segment (by offsetting from the root position)
        for(int hairSegment = 0; hairSegment < noofHairSegments - 1; hairSegment++){
            glm::vec4 newPos = rootPos + hairSegment*hairSegmentLength*rootNormal;
            hairData[masterHairIndex++] = newPos.x;
            hairData[masterHairIndex++] = newPos.y;
            hairData[masterHairIndex++] = newPos.z;
            hairData[masterHairIndex++] = newPos.w;
        }
    }
    return hairData;
}

GLuint generateTextureFromHairData(GLfloat* hairData){
    GLuint hairDataTextureID;
    glGenTextures(1, &hairDataTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hairDataTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, noofHairSegments * nrOfDataVariablesPerMasterHair, noOfMasterHairs, 0, GL_RGBA, GL_FLOAT, hairData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    return hairDataTextureID;
}

GLuint createRandomness(){
    GLfloat* randomData = new GLfloat[2048*2048*3];

    std::random_device rd;
    std::mt19937* gen = new std::mt19937(rd());
    std::uniform_real_distribution<float>* dis = new std::uniform_real_distribution<float>(-1, 1);

    for(int i = 0; i < 2048*2048*3; i++){
        randomData[i] =  (*dis)(*gen) * 0.3f;
    }

    delete gen;
    delete dis;

    GLuint randomDataTextureID;
    glGenTextures(1, &randomDataTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, randomDataTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, randomData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return randomDataTextureID;
}



void display() {

    float currentFrame = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = currentFrame - lastFrame;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
	// View and Projection
	glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, -5.0f, -50.0f));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
//
//	// ------------------------------------------------- SKYBOX -------------------------------------------------
	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxShaderProgramID);

	glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShaderProgramID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);


    // hair shader
    glUseProgram(hairShaderProgramID);
    GLint modelLocFur = glGetUniformLocation(hairShaderProgramID, "model");
    GLint viewLocFur = glGetUniformLocation(hairShaderProgramID, "view");
    GLint projLocFur = glGetUniformLocation(hairShaderProgramID, "projection");

    GLint mainTextureLoc = glGetUniformLocation(hairShaderProgramID, "mainTexture");
    GLint hairDataTextureLoc = glGetUniformLocation(hairShaderProgramID, "hairDataTexture");
    GLint randomDataTextureLoc = glGetUniformLocation(hairShaderProgramID, "randomDataTexture");
    glUniform1i(mainTextureLoc,0);
    glUniform1i(hairDataTextureLoc, 1);
    glUniform1i(randomDataTextureLoc, 2);

    GLint noOfHairSegmentLoc = glGetUniformLocation(hairShaderProgramID, "noOfHairSegments");
    GLint noOfVerticesLoc = glGetUniformLocation(hairShaderProgramID, "noOfVertices");
    GLint nrOfDataVariablesPerMasterHairLoc = glGetUniformLocation(hairShaderProgramID, "nrOfDataVariablesPerMasterHair");

    GLint cameraPosLocFur = glGetUniformLocation(hairShaderProgramID, "cameraPosition");

    GLint lightPosLocFur = glGetUniformLocation(hairShaderProgramID, "lightPos");
    GLint lightColorLocFur = glGetUniformLocation(hairShaderProgramID, "lightColor");

    // OpenGL settings
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Create camera transformation
    glm::mat4 viewforhair = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width/(float)height, 0.1f, 100.0f);

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLocFur, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocFur, 1, GL_FALSE, glm::value_ptr(viewforhair));
    glUniformMatrix4fv(projLocFur, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3f(lightPosLocFur, lightPos.x, lightPos.y, lightPos.z);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glUniform3f(lightColorLocFur, lightColor.x, lightColor.y, lightColor.z);
    glUniform3f(cameraPosLocFur, camera.Position.x, camera.Position.y, camera.Position.z);

    glUniform1f(noOfHairSegmentLoc, (float)noofHairSegments);
    glUniform1f(noOfVerticesLoc, (float)noOfMasterHairs);
    glUniform1f(nrOfDataVariablesPerMasterHairLoc, (float)nrOfDataVariablesPerMasterHair);

//    glBindVertexArray(VAO[0]);
    // Main texture (for colour)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, VTO[0]);
//    glUniform1i(glGetUniformLocation(mainTextureLoc, "mainTexture"), 0);?

    // Hair data saved in texture
    glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
    glBindTexture(GL_TEXTURE_2D, hairDataTextureID_simulated);
//    glUniform1i(glGetUniformLocation(hairDataTextureLoc, "hairDataTexture"), 1);

    // Random data saved in texture
    glActiveTexture(GL_TEXTURE0 + 2); // Texture unit 2
    glBindTexture(GL_TEXTURE_2D, randomDataTextureID);
//    glUniform1i(glGetUniformLocation(randomDataTextureLoc, "randomDataTexture"), 2);

    /****************************************************/
    /****************************************************/

    glCopyImageSubData(hairDataTextureID_current, GL_TEXTURE_2D, 0, 0, 0, 0,
                       hairDataTextureID_last, GL_TEXTURE_2D, 0, 0, 0, 0,
                       noofHairSegments, noOfMasterHairs, 1);

    glCopyImageSubData(hairDataTextureID_simulated, GL_TEXTURE_2D, 0, 0, 0, 0,
                       hairDataTextureID_current, GL_TEXTURE_2D, 0, 0, 0, 0,
                       noofHairSegments, noOfMasterHairs, 1);

    glDrawArrays(GL_PATCHES, 0, meshData[0].mPointCount);

//    glEnable(GL_BLEND);
    glutSwapBuffers();

}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 25.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{

	// Skybox
	generateSkybox();
	cubemapTexture = loadCubemap(faces);
    //fonts
    createFont();
    generateFontBufferObjects();
    // Textures
    loadTexture(textureName, 0);
	// load mesh into a vertex buffer array
	dataArray.push_back(SQUARE_MESH);
    generateObjectBufferMesh(dataArray);

    // Set up the shaders
    const char* furVertexFilename = "./hair_shaders/furShader.vert";
    const char* furTessControlFilename = "./hair_shaders/furShader.tc";
    const char* furTessEvaluateFilename = "./hair_shaders/furShader.te";
    const char* furGeometryFilename = "./hair_shaders/furShader.gs";
    const char* furFragmentFilename = "./hair_shaders/furShader.frag";
//    hairShaderProgramID = CompileShaders("./shaders/hairVertexShader_1.txt", "./shaders/hairFragmentShader_1.txt");
    hairShaderProgramID = CompileShaders(furVertexFilename, furFragmentFilename, furTessControlFilename, furTessEvaluateFilename, furGeometryFilename);
    skyboxShaderProgramID = CompileShaders("./shaders/skyboxVertexShader.txt", "./shaders/skyboxFragmentShader.txt");
    textShaderProgramID = CompileShaders("./shaders/textVertexShader.txt", "./shaders/textFragmentShader.txt");

    GLfloat* hairData = createMasterHairs();

    // Generation of textures for hair data
    hairDataTextureID_rest = generateTextureFromHairData(hairData);
    hairDataTextureID_last = generateTextureFromHairData(hairData);
    hairDataTextureID_current = generateTextureFromHairData(hairData);
    hairDataTextureID_simulated = generateTextureFromHairData(NULL);

    // To be able to add randomness to each hair strand
     randomDataTextureID = createRandomness();

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
            camera.ProcessKeyboard(FORWARD, 0.1);
            break;
        case 's':
            camera.ProcessKeyboard(BACKWARD, 0.1);
            break;
        case 'a':
            camera.ProcessKeyboard(LEFT, 0.1);
            break;
        case 'd':
            camera.ProcessKeyboard(RIGHT, 0.1);
            break;
    }
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Final Project : Hair Rendering");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
