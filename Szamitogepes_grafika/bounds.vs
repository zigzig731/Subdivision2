#version 430

/** Uniform változók. */
layout(location = 0) uniform mat4 mMVP;

/** Bemeneti attribútumok. */
layout(location = 0) in vec3 vPos;

void main()
{
	/** Kiírjuk a pozíciót. */
	gl_Position = mMVP * vec4(vPos, 1);
}