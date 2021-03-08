#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <imgui.h>
#include <string>
#include <string_view>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "conf.h"
#include "image.h"
#include "util.h"

namespace fs = std::filesystem;

auto constexpr inline filename_def = "mandelbrot.pgm";

auto static glfw_error_callback(int const error, char const* const desc) -> void {
  throw std::runtime_error{fmt::format("GLFW Error: [{}] {}", error, desc)};
}

auto load_shader(fs::path const fpath) -> GLuint {
  namespace fs = std::filesystem;

  auto const shader = glCreateShader(GL_FRAGMENT_SHADER);

  auto const sz = fs::file_size(fpath);
  auto fp = std::ifstream{fpath.c_str(), std::ios::in | std::ios::binary};
  auto st = std::string(sz, '\0');

  fp.read(st.data(), static_cast<long>(sz));

  auto const cst = st.c_str();
  glShaderSource(shader, 1, &cst, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    auto err = std::string(512, '\0');
    glGetShaderInfoLog(shader, 512, nullptr, err.data());
    throw std::runtime_error{fmt::format("Failed to compile shader {}: {}", fpath, err.data())};
  }

  return shader;
}

auto draw(GLuint const prog, f32 const x, f32 const y, i32 const w, i32 const h) -> void {
  glUseProgram(prog);

  glUniform1i(glGetUniformLocation(prog, "width"), w);
  glUniform1i(glGetUniformLocation(prog, "height"), h);
  glUniform1i(glGetUniformLocation(prog, "iters"), 4096);
  glUniform2f(glGetUniformLocation(prog, "x_bounds"), -2.0, 1.0);
  glUniform2f(glGetUniformLocation(prog, "y_bounds"), -1.5, 1.5);

  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(w, y);
  glVertex2f(w, h);
  glVertex2f(x, h);
  glEnd();

  glBegin(GL_QUADS);
  glColor4f(0.8f, 0.1f, 0.1f, 0.6f);
  glVertex2f(x, y);
  glVertex2f(w, y);
  glVertex2f(w, h);
  glVertex2f(x, h);
  glEnd();
}

auto main(i32 const argc, char const* const* const argv) -> int {
  try {

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
      throw std::runtime_error{"Failed to initialize GLFW\n"};

    auto const glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    auto window = glfwCreateWindow(1280, 720, "Example", nullptr, nullptr);
    if (!window)
      throw std::runtime_error{"Failed to create GLFW window\n"};

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    if (glewInit() != GLEW_OK)
      throw std::runtime_error{"Failed to initialize GLEW\n"};

    auto const prog = glCreateProgram();
    auto const shader = load_shader("shader.frag");

    glAttachShader(prog, shader);
    glLinkProgram(prog);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    auto io = ImGui::GetIO();
    auto show_demo_window = true;
    auto const clear_color = ImVec4{0.45f, 0.55f, 0.60f, 1.00f};
    */

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      /*
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();

      ImGui::NewFrame();

      if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

      ImGui::Render();
      */

      i32 disp_w, disp_h;
      glfwGetFramebufferSize(window, &disp_w, &disp_h);
      /*
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glViewport(0, 0, disp_w, disp_h);
      glOrtho(0, disp_w, disp_h, 0, -1, 1);
      */

      glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
      glClear(GL_COLOR_BUFFER_BIT);

      // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      draw(prog, 0.0F, 0.0F, disp_w, disp_h);

      glfwSwapBuffers(window);
    }

    /*
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    */

    glfwDestroyWindow(window);
    glfwTerminate();

  } catch (std::exception const& e) {
    fmt::print("Exception: {}\n", e.what());
    return -1;
  }
}
