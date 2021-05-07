
// just something i wanted to get done fast
// so i can code shader effects without going to shadertoy website.

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


// https://github.com/ocornut/imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_stdlib.h"

// https://github.com/BalazsJako/ImGuiColorTextEdit
#include "TextEditor.h" 



#define GLSL_VERSION   "#version 330"
#define FONT_FILE      "Topaz-8.ttf"
#define FONT_SCALE     17.0



void create_vertex_shader(int* id) {
	*id = glCreateShader(GL_VERTEX_SHADER);
	if(*id > 0) {
		const char* src = GLSL_VERSION "\n"
			"layout(location = 0) in vec3 pos;"
			"void main() {"
				"gl_Position = vec4(pos, 1.0);"
			"}";

		glShaderSource(*id, 1, &src, NULL);
		glCompileShader(*id);
	}
	else {
		fprintf(stderr, "Failed to create vertex shader!\n");
	}
}

void create_fragment_shader(int* id, const char* src) {
	*id = glCreateShader(GL_FRAGMENT_SHADER);
	if(*id > 0) {
		glShaderSource(*id, 1, &src, NULL);
		glCompileShader(*id);
	}
	else {
		fprintf(stderr, "Failed to create vertex shader!\n");
	}
}

void link_shaders(int* program, int vs, int fs) {
	*program = glCreateProgram();
	glAttachShader(*program, vs);
	glAttachShader(*program, fs);
	glLinkProgram(*program);

	glDeleteShader(vs);
	glDeleteShader(fs);
}

void check_shader_compile_status(int shader, int* shader_ok, char** error_msg, int* error_msg_len) {
	glGetShaderiv(shader, GL_COMPILE_STATUS, shader_ok);
	if(*error_msg != NULL) {
		free(*error_msg);
		*error_msg = NULL;
	}

	if(!*shader_ok) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, error_msg_len);
		if(*error_msg_len > 0) {

			*error_msg = (char*)malloc(*error_msg_len);
			if(*error_msg != NULL) {
				glGetShaderInfoLog(shader, *error_msg_len, NULL, *error_msg);
			}
			else {
				fprintf(stderr, "Failed to allocate memory for info log!\nerrno: %i\n", errno);
			}
		}
	}
	else {
		*error_msg_len = 0;
		*shader_ok = 1;
	}

}

static int window_width = 0;
static int window_height = 0;

void glfw_window_size_callback(GLFWwindow* window, int w, int h) {
	glViewport(0, 0, w, h);
	window_width = w;
	window_height = h;
}	

int main(int argc, char** argv) {
	
	int fd = -1;
	if(argc > 1) {
		if((fd = open(argv[1], O_RDWR)) < 0) {
			perror("open");
			fprintf(stderr, "Failed to open file \"%s\"!\nerrno: %i\n", argv[1], errno);
			return -1;
		}
	}

	if(!glfwInit()) {
		fprintf(stderr, "Failed to initialize glfw!\n");
		return -1;
	}
	
	glfwWindowHint(GLFW_MAXIMIZED, 1);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, 1);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 800, "ShaderSanbox", NULL, NULL);
	if(window == NULL) {
		fprintf(stderr, "Failed to create window\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, glfw_window_size_callback);

	if(glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize glew!\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	//glfwGetFramebufferSize(window, &width, &height);
	//glViewport(0, 0, width, height);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, 1);
	ImGui_ImplOpenGL3_Init(GLSL_VERSION);

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& s = ImGui::GetStyle();
	io.Fonts->AddFontFromFileTTF(FONT_FILE, FONT_SCALE);

	s.Colors[ImGuiCol_WindowBg]         = ImVec4(0.0, 0.0, 0.0, 0.0);
	s.Colors[ImGuiCol_TextSelectedBg]   = ImVec4(0.4, 0.0, 0.0, 1.0);
	s.Colors[ImGuiCol_Text]             = ImVec4(1.0, 0.2, 0.2, 1.0);
	s.Colors[ImGuiCol_FrameBg]          = ImVec4(0.1, 0.05, 0.05, 1.0);

	s.WindowPadding = ImVec2(0.0, 0.0);
	s.WindowRounding = 0.0;
	
	std::string shader_code = GLSL_VERSION "\n"
			"uniform float  gTime;\n"
			"uniform vec2   gRes;\n"
			"\n"
			"\n"
			"\n"
			"void main() {\n\n"
			"	vec2 uv = (2.0 * gl_FragCoord.xy - gRes) / gRes.y;\n"
			"	vec3 rainbow = 0.5 + 0.5 * cos(gTime + acos(cos(uv.y)) + vec3(0,2,4));\n"
			"	gl_FragColor = vec4(rainbow, 1.0);\n\n"
			"}\n";

	if(fd >= 0) {
		struct stat sb;
		if(fstat(fd, &sb) >= 0) {
			char* buf = (char*)malloc(sb.st_size+1);
			if(read(fd, buf, sb.st_size) >= 0) {
				buf[sb.st_size] = '\0';
				shader_code = std::string(buf);
			}
			else {
				fprintf(stderr, "Failed to read file!\n");
			}
			free(buf);
		}
		else {
			fprintf(stderr, "Failed to get file status!\n");
		}
	}

	int fs = 0;
	int vs = 0;
	int shader_program = 0;
	int show_editor = 1;
	int shader_ok = 0;
	int error_msg_len = 0;
	char* error_msg = NULL;

	create_vertex_shader(&vs);
	create_fragment_shader(&fs, shader_code.c_str());
	link_shaders(&shader_program, vs, fs);
	check_shader_compile_status(fs, &shader_ok, &error_msg, &error_msg_len);
	
	TextEditor texteditor;
	auto lang = TextEditor::LanguageDefinition::GLSL();
	texteditor.SetLanguageDefinition(lang);

	TextEditor::Palette palette = texteditor.GetDarkPalette();
	palette[(int)TextEditor::PaletteIndex::Background]     = 0xAA111111;
	palette[(int)TextEditor::PaletteIndex::LineBackground] = 0xAA111111;
	palette[(int)TextEditor::PaletteIndex::Default]		   = 0xFFFFFFFF;
	palette[(int)TextEditor::PaletteIndex::Number]		   = 0xFF1188EE;
	palette[(int)TextEditor::PaletteIndex::LineNumber]	   = 0xFFDDDDDD;
	palette[(int)TextEditor::PaletteIndex::Selection]      = 0xFF664433;
	palette[(int)TextEditor::PaletteIndex::Comment]          = 0xFF666666;
	palette[(int)TextEditor::PaletteIndex::MultiLineComment] = 0xFF666666;
	palette[(int)TextEditor::PaletteIndex::Punctuation]      = 0xFF55CC55;
	palette[(int)TextEditor::PaletteIndex::CurrentLineFill]  = 0xDD222222;
	palette[(int)TextEditor::PaletteIndex::CursorEdge]       = 0xFFFFFF88;
	palette[(int)TextEditor::PaletteIndex::Cursor]           = 0x33FFFF88;

	texteditor.SetPalette(palette);
	texteditor.SetText(shader_code);


	while(!glfwWindowShouldClose(window)) {
	
		const int ctrl = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
		
		if(ctrl && ImGui::IsKeyPressed(GLFW_KEY_D)) {
			show_editor = !show_editor;
		}

		if(ctrl && ImGui::IsKeyPressed(GLFW_KEY_S)) {
			create_fragment_shader(&fs, texteditor.GetText().c_str());
			link_shaders(&shader_program, vs, fs);
			check_shader_compile_status(fs, &shader_ok, &error_msg, &error_msg_len);

			if(fd >= 0) {
				const std::string& code = texteditor.GetText();
				write(fd, code.c_str(), code.size());
			}
		}


		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		if(shader_ok) {
			glUseProgram(shader_program);
			glUniform1f(glGetUniformLocation(shader_program, "gTime"), glfwGetTime());
			glUniform2f(glGetUniformLocation(shader_program, "gRes"), window_width, window_height);
			
			glBegin(GL_QUADS);
			glVertex2f(-1.0, -1.0);
			glVertex2f(-1.0,  1.0);
			glVertex2f( 1.0,  1.0);
			glVertex2f( 1.0, -1.0);
			glEnd();
		}

		ImGui::SetNextWindowPos(ImVec2(0.0, 0.0));
		ImGui::SetNextWindowSize(ImVec2(window_width, window_height));
		ImGui::Begin("##shader_editor", (bool*)NULL, ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

		if(show_editor) {
			texteditor.Render("##shader_code", ImVec2(window_width, 
						shader_ok ? window_height : (double)window_height/1.5));

			if(!shader_ok) {
				ImGui::InputTextMultiline("Compiler errors", error_msg, error_msg_len, 
						ImVec2(window_width, (double)window_height/3.0), ImGuiInputTextFlags_ReadOnly);
			}

		}

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	if(fd >= 0) {
		close(fd);
	}

	if(error_msg != NULL) {
		free(error_msg);
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


