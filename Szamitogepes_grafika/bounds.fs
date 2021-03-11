#version 430

/** Kimeneti puffer. */
out vec4 outColor;

layout(location = 1) uniform vec3 vBoundsColor;

void main()
{
    /** Kiírjuk a határoló színét. */
    outColor = vec4(vBoundsColor, 0.5);
}