//
// Created by Swung 0x48 on 2024/10/7.
//

#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "includes.h"
#include "gl/gl.h"
#include "egl/egl.h"
#include "egl/loader.h"
#include "gles/loader.h"
#include "gl/envvars.h"
#include "gl/log.h"
#include "config/settings.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define DEBUG 0

__attribute__((used)) const char* license = "GNU LGPL-2.1 License";

extern char* (*MesaConvertShader)(const char *src, unsigned int type, unsigned int glsl, unsigned int essl);

// ฟังก์ชันโหลดไลบรารีเชดเดอร์คอนเวอร์ชัน
void init_libshaderconv() {
    const char *shaderconv_lib = "libshaderconv";
    const char *func_name = "MesaConvertShader";
    const char *glslconv_name[] = {shaderconv_lib, NULL};
    void* glslconv = open_lib(glslconv_name, shaderconv_lib);
    if (glslconv == NULL) {
        LOG_D("%s not found\n", shaderconv_lib);
    }
    else {
        MesaConvertShader = (char * (*)(const char *,unsigned int,unsigned int,unsigned int))dlsym(glslconv, func_name);
        if (MesaConvertShader) {
            LOG_D("%s loaded\n", shaderconv_lib);
        } else {
            LOG_D("failed to load %s\n", shaderconv_lib);
        }
    }
}

// ฟังก์ชันสำหรับโหลดฟอนต์และสร้างเท็กซ์เจอร์
GLuint loadFontTexture(const char* fontPath) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        LOG_D("ERROR::FREETYPE: Could not init FreeType Library");
        return 0;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face)) {
        LOG_D("ERROR::FREETYPE: Failed to load font");
        return 0;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (FT_Load_Char(face, 'A', FT_LOAD_RENDER)) {
        LOG_D("ERROR::FREETYPE: Failed to load Glyph");
        return 0;
    }

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

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return texture;
}

// ฟังก์ชันสำหรับเรนเดอร์ข้อความ
void RenderText(GLuint fontTexture, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    // เรนเดอร์แต่ละตัวอักษร
    for (auto c : text) {
        // คำนวณตำแหน่งและขนาดของตัวอักษรที่จะแสดงผล

        // วาดตัวอักษรผ่าน OpenGL ES
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

// ฟังก์ชันเริ่มต้นระบบฟอนต์
void init_fonts() {
    GLuint fontTexture = loadFontTexture("path/to/your/font.ttf");

    if (fontTexture != 0) { // ตรวจสอบว่าฟอนต์ถูกโหลดสำเร็จ
        // เรนเดอร์ข้อความในลูปหลัก
        RenderText(fontTexture, "Hello, MobileGlues!", 25.0f, 25.0f, 1.0f, glm::vec3(1.0, 1.0, 1.0));
    } else {
        LOG_D("Custom font loading failed.");
    }
}

// ฟังก์ชันเริ่มต้นการตั้งค่า
void init_config() {
    if (check_path())
        config_refresh();
}

// ฟังก์ชันแสดง License
void show_license() {
    LOG_V("The Open Source License of MobileGlues: ");
    LOG_V("  %s", license);
}

#if PROFILING
PERFETTO_TRACK_EVENT_STATIC_STORAGE();
void init_perfetto() {
    perfetto::TracingInitArgs args;
    args.backends |= perfetto::kSystemBackend;
    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();
}
#endif

// ฟังก์ชันหลักที่ใช้เริ่มต้นการทำงาน
void proc_init() {
    init_config();

    clear_log();
    start_log();

    LOG_V("Initializing %s ...", RENDERERNAME);
    show_license();

    init_settings();

    load_libs();
    init_target_egl();
    init_target_gles();

    init_libshaderconv();
    
    // เริ่มต้นการใช้งานฟอนต์แบบกำหนดเอง
    init_fonts();

#if PROFILING
    init_perfetto();
#endif

    // Cleanup
    destroy_temp_egl_ctx();
    g_initialized = 1;
}

int main() {
    proc_init();
    return 0;
}
    
