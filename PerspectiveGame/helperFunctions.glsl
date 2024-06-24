
vec3 hexToRgb(float hexValue) {
    vec3 rgbColor;
    rgbColor.r = mod(hexValue / (256.0 * 256.0), 256.0) / 255.0;
    rgbColor.g = mod(hexValue / 256.0, 256.0) / 255.0;
    rgbColor.b = mod(hexValue, 256.0) / 255.0;
    return rgbColor;
}