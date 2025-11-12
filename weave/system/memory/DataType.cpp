#include "DataType.h"
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <sstream>

using namespace weave::types;

/*
DataType weave::datatype::DataTypeFromName(std::string str) {
	static std::unordered_map<std::string, DataType> typemap = {
		//Manual equivalences created as shortcuts
		{"void", DataType::Void},
		{"float", DataType::Float},
		{"vector2", DataType::Vector2},
		{"vector3", DataType::Vector3},
		{"vector4", DataType::Vector4},
		{"vec2", DataType::Vector2},
		{"vec3", DataType::Vector3},
		{"vec4", DataType::Vector4},
		{"double", DataType::Double},
		{"dvector2", DataType::DVector2},
		{"dvector3", DataType::DVector3},
		{"dvector4", DataType::DVector4},
		{"dvec2", DataType::DVector2},
		{"dvec3", DataType::DVector3},
		{"dvec4", DataType::DVector4},
		{"int", DataType::Int},
		{"ivector2", DataType::IVector2},
		{"ivector3", DataType::IVector3},
		{"ivector4", DataType::IVector4},
		{"ivec2", DataType::IVector2},
		{"ivec3", DataType::IVector3},
		{"ivec4", DataType::IVector4},
		{"uint", DataType::UInt},
		{"uvector2", DataType::UVector2},
		{"uvector3", DataType::UVector3},
		{"uvector4", DataType::UVector4},
		{"uvec2", DataType::UVector2},
		{"uvec3", DataType::UVector3},
		{"uvec4", DataType::UVector4},
		{"int8", DataType::Int8},
		{"uint8", DataType::UInt8},
		{"int8", DataType::Int16},
		{"uint8", DataType::UInt16},
		{"int64", DataType::Int64},
		{"uint64", DataType::UInt64},
		{"matrix2x2", DataType::Matrix2x2},
		{"matrix3x3", DataType::Matrix3x3},
		{"matrix4x4", DataType::Matrix4x4},
		{"mat2", DataType::Matrix2x2},
		{"mat3", DataType::Matrix3x3},
		{"mat4", DataType::Matrix4x4},
		{"dmatrix2x2", DataType::DMatrix2x2},
		{"dmatrix3x3", DataType::DMatrix3x3},
		{"dmatrix4x4", DataType::DMatrix4x4},
		{"dmat2", DataType::DMatrix2x2},
		{"dmat3", DataType::DMatrix3x3},
		{"dmat4", DataType::DMatrix4x4},
		{"quaternion", DataType::Quaternion},
		{"quat", DataType::Quaternion},
		{"dquaternion", DataType::DQuaternion},
		{"dquat", DataType::DQuaternion},
		{"string", DataType::String},
		{"custom", DataType::UserDefined},
	};

	//Transform to uppercase
	std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)::tolower((int)c); });
	//Return the correct value for the associated string
	return typemap[str];
}

std::string weave::datatype::NameFromDataType(DataType type) {
	static std::unordered_map<DataType, std::string> typemap = {
		//Manual equivalences created as shortcuts
		{DataType::Void, "void"},
		{DataType::Float,"float"},
		{DataType::Vector2,"vector2"},
		{DataType::Vector3,"vector3"},
		{DataType::Vector4,"vector4"},
		{DataType::Vector2,"vec2"},
		{DataType::Vector3,"vec3"},
		{DataType::Vector4,"vec4"},
		{DataType::Double,"double"},
		{DataType::DVector2,"dvector2"},
		{DataType::DVector3,"dvector3"},
		{DataType::DVector4,"dvector4"},
		{DataType::DVector2,"dvec2"},
		{DataType::DVector3,"dvec3"},
		{DataType::DVector4,"dvec4"},
		{DataType::Int,"int"},
		{DataType::IVector2,"ivector2"},
		{DataType::IVector3,"ivector3"},
		{DataType::IVector4,"ivector4"},
		{DataType::IVector2,"ivec2"},
		{DataType::IVector3,"ivec3"},
		{DataType::IVector4,"ivec4"},
		{DataType::UInt,"uint"},
		{DataType::UVector2,"uvector2"},
		{DataType::UVector3,"uvector3"},
		{DataType::UVector4,"uvector4"},
		{DataType::UVector2,"uvec2"},
		{DataType::UVector3,"uvec3"},
		{DataType::UVector4,"uvec4"},
		{DataType::Int8,"int8"},
		{DataType::UInt8,"uint8"},
		{DataType::Int8,"int16"},
		{DataType::UInt8,"uint16"},
		{DataType::Int64,"int64"},
		{DataType::UInt64,"uint64"},
		{DataType::Matrix2x2,"matrix2x2"},
		{DataType::Matrix3x3,"matrix3x3"},
		{DataType::Matrix4x4,"matrix4x4"},
		{DataType::Matrix2x2,"mat2"},
		{DataType::Matrix3x3,"mat3"},
		{DataType::Matrix4x4,"mat4"},
		{DataType::DMatrix2x2,"dmatrix2x2"},
		{DataType::DMatrix3x3,"dmatrix3x3"},
		{DataType::DMatrix4x4,"dmatrix4x4"},
		{DataType::DMatrix2x2,"dmat2"},
		{DataType::DMatrix3x3,"dmat3"},
		{DataType::DMatrix4x4,"dmat4"},
		{DataType::Quaternion,"quaternion"},
		{DataType::Quaternion,"quat"},
		{DataType::DQuaternion,"dquaternion"},
		{DataType::DQuaternion,"dquat"},
		{DataType::String,"string"},
		{DataType::UserDefined,"custom"},
	};

	//Return the correct string for the associated type
	return typemap[type];
}

*/