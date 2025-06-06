/**
 CShaderManager
 @brief This class manages all the shaders used in this project.
 By: Toh Da Jun
 Date: Mar 2020
*/
#pragma once

// Include SingletonTemplate
#include "..\DesignPatterns\SingletonTemplate.h"

#include <map>
#include <string>
#include "Shader.h"

class CShaderManager : public CSingletonTemplate<CShaderManager>
{
	friend CSingletonTemplate<CShaderManager>;
public:
	// Destroy the instance
	void Destroy(void);

	// User Interface
	// Add a shader to this map
	void Add(const std::string& _name, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	// Remove a shader from this map
	void Remove(const std::string& _name);
	// Use a shader in this map
	void Use(const std::string& _name);
	// Check if a shader exists in this map
	bool Check(const std::string& _name);

	// The current active shader
	CShader* pActiveShader;

protected:
	// Constructor
	CShaderManager(void);

	// Destructor
	virtual ~CShaderManager(void);

	// The map containing all the shaders loaded
	std::map<std::string, CShader*> shaderMap;
};
