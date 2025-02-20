/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : VertexArrayObject.cpp
Description : VertexArrayObject class is a representation of a VAO to be used by the graphics engine class
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/


#include "VertexArrayObject.h" // Include the header file for VertexArrayObject class
#include <glew.h> // Include GLEW for OpenGL function loading

// Constructor for VertexArrayObject, initializes a vertex array object using a vertex buffer descriptor
VertexArrayObject::VertexArrayObject(const VertexBufferDesc& _data)
{
	// Check for null or invalid vertex buffer attributes and log errors if found
	if (!_data.listSize) Debug::LogError("VertexArrayObject | vertexBuffer.listSize is NULL");
	if (!_data.vertexSize) Debug::LogError("VertexArrayObject | vertexBuffer.vertexSize is NULL");
	if (!_data.verticesList) Debug::LogError("VertexArrayObject | vertexBuffer.verticesList is NULL");

	// Generate and bind a new vertex array object
	glGenVertexArrays(1, &m_vertexArrayObjectID);
	glBindVertexArray(m_vertexArrayObjectID);

	// Initialize the vertex buffer
	glGenBuffers(1, &m_vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferID);
	// Allocate memory and upload vertex data to the buffer
	glBufferData(GL_ARRAY_BUFFER, _data.vertexSize * _data.listSize, _data.verticesList, GL_STATIC_DRAW);

	// Set up vertex attributes based on the attributes list in the vertex buffer descriptor
	size_t offset = 0; // Initialize offset for attribute pointers
	for (uint i = 0; i < _data.attributesListSize; i++)
	{
		offset += ((i == 0) ? 0 : _data.attributesList[i - 1].numElements * sizeof(float));

		// Specify the layout of the vertex data
		glVertexAttribPointer(
			i,
			_data.attributesList[i].numElements,
			GL_FLOAT,
			GL_FALSE,
			_data.vertexSize,
			(void*)offset // Offset in the vertex data
		);
		glEnableVertexAttribArray(i); // Enable the vertex attribute array
	}

	glBindVertexArray(0); // Unbind the vertex array object

	// Store vertex buffer data for later use
	m_vertexBufferData = _data;
}

// Constructor for VertexArrayObject that also initializes an index buffer
VertexArrayObject::VertexArrayObject(const VertexBufferDesc& _vbDesc, const IndexBufferDesc& _ibDesc)
	: VertexArrayObject(_vbDesc) // Call the other constructor for vertex buffer initialization
{
	// Check for null or invalid index buffer attributes and log errors if found
	if (!_ibDesc.listSize) Debug::LogError("VertexArrayObject | indexBuffer.listSize is NULL");
	if (!_ibDesc.indicesList) Debug::LogError("VertexArrayObject | indexBuffer.indicesList is NULL");

	glBindVertexArray(m_vertexArrayObjectID); // Bind the vertex array object

	// Initialize the index buffer
	glGenBuffers(1, &m_elementBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferId);
	// Allocate memory and upload index data to the buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _ibDesc.listSize * sizeof(uint), _ibDesc.indicesList, GL_STATIC_DRAW);
	glBindVertexArray(0); // Unbind the vertex array object

	// Store index buffer descriptor for later use
	m_indexBufferDesc = _ibDesc;
}

// Destructor for VertexArrayObject, cleans up OpenGL resources
VertexArrayObject::~VertexArrayObject()
{
	glDeleteBuffers(1, &m_elementBufferId); // Delete the index buffer
	glDeleteBuffers(1, &m_vertexBufferID); // Delete the vertex buffer
	glDeleteVertexArrays(1, &m_vertexArrayObjectID); // Delete the vertex array object
}

// Initializes an instance buffer for instanced rendering
void VertexArrayObject::initInstanceBuffer(Mat4* instanceData, size_t instanceCount)
{
	glBindVertexArray(m_vertexArrayObjectID); // Bind the vertex array object

	// Initialize the instance buffer
	glGenBuffers(1, &m_instanceBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_instanceBufferID);
	// Allocate memory and upload instance data to the buffer
	glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(Mat4), instanceData, GL_DYNAMIC_DRAW);

	// Set up attribute pointers for the instance data bound to attribute locations 3, 4, 5, and 6
	for (int i = 0; i < 4; i++) {
		glEnableVertexAttribArray(3 + i); // Enable instance attribute array
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Mat4), (void*)(sizeof(glm::vec4) * i)); // Set the attribute pointer
		glVertexAttribDivisor(3 + i, 1); // Set the divisor to 1 for instancing
	}

	glBindVertexArray(0); // Unbind the vertex array object
}

// Returns the ID of the vertex array object
uint VertexArrayObject::getId()
{
	return m_vertexArrayObjectID;
}

// Returns the size of the vertex buffer
uint VertexArrayObject::getVertexBufferSize()
{
	return sizeof(m_vertexBufferData.vertexSize); // Return the size of the vertex buffer
}

// Returns the number of indices in the index buffer
uint VertexArrayObject::getNumIndices()
{
	return m_indexBufferDesc.listSize; // Return the number of indices
}