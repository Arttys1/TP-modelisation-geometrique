/**
* Fragment shader to draw mandelbrot and julia set on torus.
*/
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform float i;
uniform float j;

vec3 color_by_divergence(int i) {
    switch(i) {
        case 0:  return vec3(1.f, 1.f, 1.f);
        case 1:  return vec3(0.7f, 0.3f, 0.5f);
        case 2:  return vec3(0.1f, 0.3f, 0.3f);
        case 3:  return vec3(0.1f, 0.9f, 0.7f);
        case 4:  return vec3(0.2f, 0.6f, 0.5f);
        case 5:  return vec3(0.9f, 0.2f, 0.6f);
        case 6:  return vec3(0.4f, 0.4f, 0.8f);
        case 7:  return vec3(0.4f, 0.3f, 0.3f);
        case 8:  return vec3(0.8f, 0.8f, 0.1f);
        case 9:  return vec3(0.8f, 0.7f, 0.4f);
        case 10: return vec3(0.3f, 0.6f, 0.1f);
        case 11: return vec3(0.7f, 0.3f, 0.5f);
        case 12: return vec3(0.1f, 0.3f, 0.3f);
        case 13: return vec3(0.1f, 0.9f, 0.7f);
        case 14: return vec3(0.2f, 0.6f, 0.5f);
        case 15: return vec3(0.9f, 0.2f, 0.6f);
        case 16: return vec3(0.4f, 0.4f, 0.8f);
        case 17: return vec3(0.4f, 0.3f, 0.3f);
        case 18: return vec3(0.8f, 0.8f, 0.1f);
        case 19: return vec3(0.8f, 0.7f, 0.4f);
        default: return vec3(0.2f, 0.2f, 0.2f);
    }
}

vec3 iterate(vec2 z, vec2 c) {
    for(int i = 0; i < 20; i++) {
        z = vec2(z.x * z.x - z.y * z.y , 2 * z.x * z.y) + c;
        if(z.x * z.x + z.y * z.y > 4) {
            return color_by_divergence(i);
        }
    }
    return vec3(0.f, 0.f, 0.f);
}

vec3 mandelbrot(vec2 pos) {
    vec2 c = pos;
    vec2 z = vec2(0.f, 0.f);
    return iterate(z, c);
}

vec3 julia_set(vec2 pos) {
    vec2 c = vec2(i, j);
    vec2 z = pos;
    return iterate(z, c);
}

void main() {
    vec2 pos  = (TexCoord * 2) - vec2(2,1);
    vec3 color = mandelbrot(pos);
    //vec3 color = julia_set(pos);
    FragColor = vec4(color, 1.0f);
}
