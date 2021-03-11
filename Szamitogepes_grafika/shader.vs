#version 430

/** Uniform változók. */
layout (std140, binding = 0) uniform ModelData
{
	mat4 mModelView;
	mat4 mView;
	mat4 mNormal;
	mat4 mMVP;
	mat4 mRotate;
} sModelData;

/** Bemeneti attribútumok. */
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

/** Kimeneti attribútumok. */
out vec3 vPosVS;
out vec3 vNormalVS;
out vec3 vNormalVS2;

void main()
{
	
	vPosVS = vec3(sModelData.mModelView * vec4(vPos, 1));
	vNormalVS =vec3(sModelData.mRotate*vec4(vNormal, 1.0));
	vNormalVS2 =vNormal;
	/** Kiírjuk a pozíciót. */
	gl_Position = sModelData.mMVP * vec4(vPos, 1);
}